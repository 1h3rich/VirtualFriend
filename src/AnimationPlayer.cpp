#include "AnimationPlayer.h"
#include "FaceRenderer.h"
#include "ExpressionManager.h"
#include <string.h>

extern FaceRenderer      face;
extern ExpressionManager expr;
extern volatile bool     g_micActive;
extern volatile bool     g_aiThinking;

// ---------------------------------------------------------------
// Double-buffer con M5Canvas (LGFX_Sprite) — DMA gestionado por M5GFX
// ---------------------------------------------------------------
M5Canvas  s_sprite(&M5.Display);   // canvas compartido con FaceRenderer
static uint16_t  s_lineBuf[320];          // buffer de línea para conversión

static QueueHandle_t s_eventQueue = nullptr;

// ---------------------------------------------------------------
// Push sprite al display con overlay de grabación si activo
// ---------------------------------------------------------------
static void pushWithOverlay() {
    if (g_micActive) {
        s_sprite.fillRect(0, 212, 320, 28, TFT_RED);
        s_sprite.setTextColor(TFT_WHITE, TFT_RED);
        s_sprite.drawCenterString("HABLA AHORA", 160, 214, 4);
    } else if (g_aiThinking) {
        s_sprite.fillRect(0, 212, 320, 28, TFT_BLUE);
        s_sprite.setTextColor(TFT_WHITE, TFT_BLUE);
        s_sprite.drawCenterString("PENSANDO...", 160, 214, 4);
    }
    s_sprite.pushSprite(0, 0);
}

// ---------------------------------------------------------------
// Callbacks de archivo SD para AnimatedGIF
// ---------------------------------------------------------------
static File s_gifFile;

static void* gifOpen(const char* fname, int32_t* pSize) {
    s_gifFile = SD.open(fname);
    if (!s_gifFile) return nullptr;
    *pSize = s_gifFile.size();
    return &s_gifFile;
}
static void gifClose(void* pHandle) {
    ((File*)pHandle)->close();
}
static int32_t gifRead(GIFFILE* pFile, uint8_t* pBuf, int32_t iLen) {
    File* f = (File*)pFile->fHandle;
    int32_t avail = pFile->iSize - pFile->iPos;
    if (iLen > avail) iLen = avail;
    if (iLen <= 0) return 0;
    iLen = (int32_t)f->read(pBuf, (size_t)iLen);
    pFile->iPos = (int32_t)f->position();
    return iLen;
}
static int32_t gifSeek(GIFFILE* pFile, int32_t iPos) {
    File* f = (File*)pFile->fHandle;
    f->seek((uint32_t)iPos);
    pFile->iPos = (int32_t)f->position();
    return pFile->iPos;
}

// ---------------------------------------------------------------
// Callback de dibujado — acumula en sprite, empuja en último scanline
// ---------------------------------------------------------------
static void gifDraw(GIFDRAW* pDraw) {
    uint8_t*  s   = pDraw->pPixels;
    uint16_t* pal = pDraw->pPalette;
    int       w   = pDraw->iWidth;
    int       x0  = pDraw->iX;
    int       y   = pDraw->iY + pDraw->y;

    if (w > 320) w = 320;          // proteger s_lineBuf[320]
    if (y < 0 || y >= 240) {       // proteger el sprite 320x240
        if (pDraw->y == pDraw->iHeight - 1) pushWithOverlay();
        return;
    }

    for (int x = 0; x < w; x++) {
        s_lineBuf[x] = (pDraw->ucHasTransparency && s[x] == pDraw->ucTransparent)
                       ? 0x0000 : pal[s[x]];
    }
    s_sprite.pushImage(x0, y, w, 1, s_lineBuf);

    if (pDraw->y == pDraw->iHeight - 1)
        pushWithOverlay();
}

// ---------------------------------------------------------------
// Constructor / begin
// ---------------------------------------------------------------
AnimationPlayer::AnimationPlayer() : _sdOk(false), _eventQueue(nullptr) {}

void AnimationPlayer::begin() {
    _gif.begin(GIF_PALETTE_RGB565_BE);
    _eventQueue = xQueueCreate(4, sizeof(AnimEvent));
    s_eventQueue = _eventQueue;

    Serial.printf("[ANIM] PSRAM size=%d free=%d  SRAM free=%d\n",
        (int)ESP.getPsramSize(), (int)ESP.getFreePsram(),
        (int)esp_get_free_heap_size());

    s_sprite.setPsram(true);
    bool ok = s_sprite.createSprite(320, 240);
    if (!ok) {
        s_sprite.setPsram(false);
        ok = s_sprite.createSprite(320, 240);
    }
    Serial.printf("[ANIM] sprite=%dx%d ok=%d  free_after=%d\n",
        s_sprite.width(), s_sprite.height(), ok,
        (int)esp_get_free_heap_size());
    s_sprite.fillScreen(0x0000);

    face.setCanvas(&s_sprite);

    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("[ANIM] SD no disponible — modo vectorial");
        return;
    }
    const char* paths[] = {
        "/faces/idle/idle.gif", "/faces/startup/startup.gif",
        "/faces/accel/accel.gif", "/faces/brake/brake.gif", "/faces/bump/bump.gif"
    };
    for (auto p : paths)
        Serial.printf("[ANIM] %s %s\n", p, SD.exists(p) ? "OK" : "MISSING");

    if (!SD.exists("/faces/idle/idle.gif")) {
        Serial.println("[ANIM] idle.gif no encontrado — modo vectorial");
        return;
    }
    _sdOk = true;
    Serial.println("[ANIM] SD OK — modo GIF activo");
}

// ---------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------
bool AnimationPlayer::_exists(const char* path) {
    return SD.exists(path);
}

bool AnimationPlayer::_pickPath(const char* base, char* out, size_t outLen) {
    char candidates[MAX_VARIANTS][64];
    int found = 0;

    snprintf(candidates[found], 64, "/faces/%s/%s.gif", base, base);
    if (_exists(candidates[found])) found++;

    for (int v = 2; v <= MAX_VARIANTS; v++) {
        snprintf(candidates[found], 64, "/faces/%s/%s%d.gif", base, base, v);
        if (_exists(candidates[found])) found++;
        else break;
    }

    if (found == 0) return false;
    int idx = (found == 1) ? 0 : (int)(esp_random() % (uint32_t)found);
    snprintf(out, outLen, "%s", candidates[idx]);
    return true;
}

// ---------------------------------------------------------------
// Reproduce un GIF una vez; devuelve true si completó sin interrupción
// ---------------------------------------------------------------
bool AnimationPlayer::_playGifOnce(const char* path) {
    if (!_exists(path)) return false;

    s_sprite.fillScreen(0x0000);
    pushWithOverlay();
    int rc = _gif.open(path, gifOpen, gifClose, gifRead, gifSeek, gifDraw);
    if (!rc) {
        vTaskDelay(pdMS_TO_TICKS(200));
        return false;
    }

    int frameDelay;
    while (_gif.playFrame(false, &frameDelay)) {
        vTaskDelay(pdMS_TO_TICKS(max(1, frameDelay)));
    }
    _gif.close();
    return true;
}

// ---------------------------------------------------------------
// Bucle infinito del GIF de idle, interrumpible por evento en la cola
// ---------------------------------------------------------------
bool AnimationPlayer::_playGifLoop(const char* path) {
    if (!_exists(path)) return false;

    static uint32_t lastStack = 0;

    while (true) {
        AnimEvent ev;
        if (xQueuePeek(_eventQueue, &ev, 0) == pdTRUE) return true;

        // Limpiar sprite en RAM sin hacer push — el GIF necesita fondo negro limpio
        // (frames con transparencia usan los píxeles anteriores como base).
        // La pantalla no se actualiza hasta que gifDraw renderice el primer frame.
        s_sprite.fillScreen(0x0000);

        int rc = _gif.open(path, gifOpen, gifClose, gifRead, gifSeek, gifDraw);
        if (!rc) {
            vTaskDelay(pdMS_TO_TICKS(200));
            return false;
        }

        int  frameDelay;
        bool interrupted = false;
        while (_gif.playFrame(false, &frameDelay)) {
            if (xQueuePeek(_eventQueue, &ev, 0) == pdTRUE) {
                interrupted = true;
                break;
            }
            uint32_t now = millis();
            if (now - lastStack >= 10000) {
                lastStack = now;
                Serial.printf("[STACK] Anim=%u\n",
                    (unsigned)uxTaskGetStackHighWaterMark(nullptr));
            }
            vTaskDelay(pdMS_TO_TICKS(max(1, frameDelay)));
        }
        _gif.close();
        if (interrupted) return true;
    }
}

// ---------------------------------------------------------------
// Fallback vectorial
// ---------------------------------------------------------------
void AnimationPlayer::_vectorFallback(AnimEvent ev) {
    switch (ev) {
        case AnimEvent::STARTUP:       expr.setExpression(Expression::HAPPY);     break;
        case AnimEvent::HARD_ACCEL:    expr.setExpression(Expression::SARCASTIC); break;
        case AnimEvent::HARD_BRAKE:    expr.setExpression(Expression::SURPRISED); break;
        case AnimEvent::BUMP:          expr.setExpression(Expression::ANGRY);     break;
        case AnimEvent::AI_HAPPY:      expr.setExpression(Expression::HAPPY);     break;
        case AnimEvent::AI_SARCASTIC:  expr.setExpression(Expression::SARCASTIC); break;
        case AnimEvent::AI_SERIOUS:    expr.setExpression(Expression::ALERT);     break;
        case AnimEvent::AI_SURPRISED:  expr.setExpression(Expression::SURPRISED); break;
        case AnimEvent::AI_SLEEPY:     expr.setExpression(Expression::SLEEPING);  break;
        default:                       expr.setExpression(Expression::NEUTRAL);   break;
    }
    face.draw(expr.getParams());
    vTaskDelay(pdMS_TO_TICKS(2500));
    expr.setExpression(Expression::NEUTRAL);
    face.draw(expr.getParams());
}

// ---------------------------------------------------------------
// triggerEvent: seguro desde cualquier núcleo
// ---------------------------------------------------------------
void AnimationPlayer::triggerEvent(AnimEvent ev) {
    if (!_eventQueue) return;
    if (uxQueueSpacesAvailable(_eventQueue) == 0) {
        AnimEvent dummy;
        xQueueReceive(_eventQueue, &dummy, 0);
    }
    xQueueSend(_eventQueue, &ev, 0);
}

// ---------------------------------------------------------------
// triggerAiExpression: mapea string del Worker a AnimEvent
// ---------------------------------------------------------------
void AnimationPlayer::triggerAiExpression(const char* exprName) {
    if (!exprName || !*exprName) return;
    AnimEvent ev = AnimEvent::NONE;
    if      (!strcmp(exprName, "happy"))     ev = AnimEvent::AI_HAPPY;
    else if (!strcmp(exprName, "sarcastic")) ev = AnimEvent::AI_SARCASTIC;
    else if (!strcmp(exprName, "serious"))   ev = AnimEvent::AI_SERIOUS;
    else if (!strcmp(exprName, "surprised")) ev = AnimEvent::AI_SURPRISED;
    else if (!strcmp(exprName, "sleepy"))    ev = AnimEvent::AI_SLEEPY;
    else return;
    triggerEvent(ev);
}

// ---------------------------------------------------------------
// Loop principal de la tarea de animación
// ---------------------------------------------------------------
void AnimationPlayer::_animTaskLoop() {
    char path[64];

    if (_sdOk && _pickPath("startup", path, sizeof(path)))
        _playGifOnce(path);
    else
        _vectorFallback(AnimEvent::STARTUP);

    while (true) {
        if (_sdOk) {
            if (!_playGifLoop("/faces/idle/idle.gif")) {
                vTaskDelay(pdMS_TO_TICKS(500));
                continue;
            }
        } else {
            expr.setExpression(Expression::NEUTRAL);
            face.draw(expr.getParams());
            AnimEvent ev = AnimEvent::NONE; // el receive puede expirar sin escribir
            xQueueReceive(_eventQueue, &ev, pdMS_TO_TICKS(50));
            if (ev == AnimEvent::NONE) continue;
            xQueueSendToFront(_eventQueue, &ev, 0);
        }

        AnimEvent ev = AnimEvent::NONE;
        xQueueReceive(_eventQueue, &ev, 0);
        if (ev == AnimEvent::NONE) continue;

        const char* base = nullptr;
        switch (ev) {
            case AnimEvent::HARD_ACCEL:    base = "accel";     break;
            case AnimEvent::HARD_BRAKE:    base = "brake";     break;
            case AnimEvent::BUMP:          base = "bump";      break;
            case AnimEvent::STARTUP:       base = "startup";   break;
            case AnimEvent::AI_HAPPY:      base = "happy";     break;
            case AnimEvent::AI_SARCASTIC:  base = "sarcastic"; break;
            case AnimEvent::AI_SERIOUS:    base = "serious";   break;
            case AnimEvent::AI_SURPRISED:  base = "surprised"; break;
            case AnimEvent::AI_SLEEPY:     base = "sleepy";    break;
            default: break;
        }

        if (base) {
            if (_sdOk && _pickPath(base, path, sizeof(path)))
                _playGifOnce(path);
            else
                _vectorFallback(ev);
        }
    }
}

void animPlayerTask(void* pvParams) {
    static_cast<AnimationPlayer*>(pvParams)->_animTaskLoop();
    vTaskDelete(nullptr);
}
