#include "AiManager.h"
#include "AnimationPlayer.h"
#include "SoundManager.h"
#include "Config.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

extern AnimationPlayer anim;
extern SoundManager    sound;
extern volatile bool   g_aiThinking;

// ── WiFi ──────────────────────────────────────────────────────────────────────

void AiManager::begin() {
    _queue    = xQueueCreate(2, sizeof(SensorEvent));
    _micQueue = xQueueCreate(1, sizeof(MicAudioMsg));

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("[WiFi] Conectando");
    for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[WiFi] OK — IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n[WiFi] Sin conexion — modo sin voz activo");
    }

    xTaskCreatePinnedToCore(_taskEntry, "AiTask", 28672, this, 1, nullptr, 0);
}

bool AiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

// ── Cola de eventos ───────────────────────────────────────────────────────────

void AiManager::onEvent(SensorEvent ev) {
    if (!isConnected() || !_queue) return;
    if (uxQueueSpacesAvailable(_queue) == 0) {
        SensorEvent dummy;
        xQueueReceive(_queue, &dummy, 0);
    }
    xQueueSend(_queue, &ev, 0);
}

void AiManager::onMicAudio(uint8_t* wavData, size_t wavLen) {
    if (!isConnected() || !_micQueue) {
        free(wavData);
        sound.play(SoundEffect::ALERT);
        return;
    }
    MicAudioMsg msg = { wavData, wavLen };
    if (uxQueueSpacesAvailable(_micQueue) == 0) {
        MicAudioMsg old;
        if (xQueueReceive(_micQueue, &old, 0) == pdTRUE) free(old.data);
    }
    if (xQueueSend(_micQueue, &msg, 0) != pdTRUE) free(wavData);
}

// ── Tarea ─────────────────────────────────────────────────────────────────────

void AiManager::_taskEntry(void* param) {
    static_cast<AiManager*>(param)->_taskLoop();
    vTaskDelete(nullptr);
}

void AiManager::_taskLoop() {
    SensorEvent ev;
    MicAudioMsg mic;
    uint32_t    lastStackMs = 0;

    while (true) {
        uint32_t now = millis();
        if (now - lastStackMs >= 10000) {
            lastStackMs = now;
            Serial.printf("[STACK] Ai=%u\n",
                (unsigned)uxTaskGetStackHighWaterMark(nullptr));
        }

        if (xQueueReceive(_micQueue, &mic, 0) == pdTRUE) {
            _processListen(mic.data, mic.len);
            free(mic.data);
        } else if (xQueueReceive(_queue, &ev, pdMS_TO_TICKS(100)) == pdTRUE) {
            _process(ev);
        }
    }
}

// ── Helper: descarga MP3 del stream ──────────────────────────────────────────

static size_t downloadMp3(WiFiClient* stream, HTTPClient& http, uint8_t** outBuf) {
    int contentLen = http.getSize();
    size_t allocSz = (contentLen > 0 && contentLen <= 512 * 1024)
                     ? (size_t)contentLen : 96 * 1024;

    uint8_t* buf = (uint8_t*)ps_malloc(allocSz);
    if (!buf) return 0;

    size_t received = 0;
    uint32_t t0 = millis();

    while (received < allocSz) {
        if (millis() - t0 > 15000) break;
        int av = stream->available();
        if (av > 0) {
            size_t n = stream->readBytes(buf + received,
                                         min((size_t)av, allocSz - received));
            received += n;
            t0 = millis();
        } else if (!http.connected()) {
            break;
        } else {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }

    if (received == 0) { free(buf); return 0; }
    *outBuf = buf;
    return received;
}

// ── Petición combinada: Gemini + ElevenLabs en un solo round-trip ─────────────

void AiManager::_process(SensorEvent ev) {
    if (!isConnected()) {
        sound.play(SoundEffect::ALERT);
        return;
    }

    char body[256];
    snprintf(body, sizeof(body),
        "{\"message\":\"%s\",\"mood\":\"%s\",\"movement\":\"%s\"}",
        _message(ev), _mood(ev), _movement(ev)
    );

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    if (!http.begin(client, "https://" WORKER_HOST)) {
        sound.play(SoundEffect::ALERT);
        return;
    }

    const char* hdrs[] = {"X-VF-Expression", "X-VF-Sound"};
    http.collectHeaders(hdrs, 2);
    http.addHeader("Content-Type",   "application/json");
    http.addHeader("X-Device-Token", WORKER_TOKEN);
    http.setTimeout(18000);

    g_aiThinking = true;
    int code = http.POST((uint8_t*)body, strlen(body));
    if (code != 200) {
        Serial.printf("[AI] HTTP %d\n", code);
        g_aiThinking = false;
        http.end();
        sound.play(SoundEffect::ALERT);
        return;
    }

    String expression = http.header("X-VF-Expression");
    Serial.printf("[AI] expr=%s sound=%s\n",
        expression.c_str(),
        http.header("X-VF-Sound").c_str());

    uint8_t* mp3 = nullptr;
    size_t   len = downloadMp3(http.getStreamPtr(), http, &mp3);
    http.end();
    g_aiThinking = false;

    if (len > 0) {
        Serial.printf("[AI] MP3 %u bytes\n", (unsigned)len);
        anim.triggerAiExpression(expression.c_str());
        _speech.playMp3(mp3, len);
        free(mp3);
    } else {
        sound.play(SoundEffect::ALERT);
    }
}

// ── /listen: audio del micrófono → Whisper → OpenAI → ElevenLabs ────────────

void AiManager::_processListen(const uint8_t* wavData, size_t wavLen) {
    if (!isConnected()) {
        sound.play(SoundEffect::ALERT);
        return;
    }

    Serial.printf("[Listen] Enviando %u bytes WAV\n", (unsigned)wavLen);

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    if (!http.begin(client, "https://" WORKER_HOST "/listen")) {
        sound.play(SoundEffect::ALERT);
        return;
    }

    const char* hdrs[] = {"X-VF-Expression", "X-VF-Sound"};
    http.collectHeaders(hdrs, 2);
    http.addHeader("Content-Type",   "audio/wav");
    http.addHeader("X-Device-Token", WORKER_TOKEN);
    http.setTimeout(25000);

    g_aiThinking = true;
    int code = http.POST(const_cast<uint8_t*>(wavData), wavLen);
    if (code != 200) {
        Serial.printf("[Listen] HTTP %d\n", code);
        g_aiThinking = false;
        http.end();
        sound.play(SoundEffect::ALERT);
        return;
    }

    String expression = http.header("X-VF-Expression");
    Serial.printf("[Listen] expr=%s\n", expression.c_str());

    uint8_t* mp3 = nullptr;
    size_t   len = downloadMp3(http.getStreamPtr(), http, &mp3);
    http.end();
    g_aiThinking = false;

    if (len > 0) {
        Serial.printf("[Listen] MP3 %u bytes\n", (unsigned)len);
        anim.triggerAiExpression(expression.c_str());
        _speech.playMp3(mp3, len);
        free(mp3);
    } else {
        sound.play(SoundEffect::ALERT);
    }
}

// ── Mapeo evento → contexto ───────────────────────────────────────────────────

const char* AiManager::_message(SensorEvent ev) {
    switch (ev) {
        case SensorEvent::HARD_ACCEL: return "El conductor ha acelerado fuerte";
        case SensorEvent::HARD_BRAKE: return "El conductor ha frenado bruscamente";
        case SensorEvent::BUMP:       return "El coche ha pasado por un bache";
        default:                      return "Comenta algo interesante del viaje";
    }
}

const char* AiManager::_mood(SensorEvent ev) {
    switch (ev) {
        case SensorEvent::HARD_ACCEL: return "sarcastico";
        case SensorEvent::HARD_BRAKE: return "sorprendido";
        case SensorEvent::BUMP:       return "sarcastico";
        default:                      return "sarcastico";
    }
}

const char* AiManager::_movement(SensorEvent ev) {
    switch (ev) {
        case SensorEvent::HARD_ACCEL: return "hard_accel";
        case SensorEvent::HARD_BRAKE: return "hard_brake";
        case SensorEvent::BUMP:       return "bump";
        default:                      return "normal";
    }
}
