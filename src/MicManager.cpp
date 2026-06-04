#include "MicManager.h"
#include <Arduino.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t g_audioMutex;
extern volatile bool     g_micActive;

void MicManager::begin() {
    _sem = xSemaphoreCreateBinary();
    xTaskCreatePinnedToCore(micTask, "MicTask", 12288, this, 1, nullptr, 1);
}

void MicManager::triggerRecord() {
    if (_recording || _pending) return;
    // Feedback visual inmediato: el indicador aparece en el siguiente frame del GIF
    // aunque la grabacion real tarde en arrancar (espera de mutex, init de I2S).
    g_micActive = true;
    xSemaphoreGive(_sem);
}

void MicManager::takePending(uint8_t** outBuf, size_t* outLen) {
    *outBuf  = _wavBuf;
    *outLen  = _wavLen;
    _wavBuf  = nullptr;
    _wavLen  = 0;
    _pending = false;
}

void MicManager::_buildWavHeader(uint8_t* b, uint32_t n) {
    uint32_t dataSize  = n * 2;
    uint32_t chunkSize = 36 + dataSize;
    uint32_t byteRate  = SAMPLE_RATE * 2;

    auto le32 = [](uint8_t* p, uint32_t v) {
        p[0]=v&0xFF; p[1]=(v>>8)&0xFF; p[2]=(v>>16)&0xFF; p[3]=(v>>24)&0xFF;
    };
    auto le16 = [](uint8_t* p, uint16_t v) {
        p[0]=v&0xFF; p[1]=(v>>8)&0xFF;
    };

    memcpy(b,    "RIFF", 4); le32(b+4,  chunkSize);
    memcpy(b+8,  "WAVE", 4);
    memcpy(b+12, "fmt ", 4); le32(b+16, 16);
    le16(b+20, 1);
    le16(b+22, 1);
    le32(b+24, SAMPLE_RATE);
    le32(b+28, byteRate);
    le16(b+32, 2);
    le16(b+34, 16);
    memcpy(b+36, "data", 4); le32(b+40, dataSize);
}

// Inicializa el micrófono limpiamente.
// Devuelve true si quedó listo para grabar.
bool MicManager::_initMic() {
    M5.Mic.end();
    vTaskDelay(pdMS_TO_TICKS(100));
    auto cfg = M5.Mic.config();
    cfg.magnification = 32;
    cfg.noise_filter_level = 0;
    M5.Mic.config(cfg);
    bool ok = M5.Mic.begin();
    if (!ok) Serial.println("[MIC] WARN: Mic.begin() fallo");
    vTaskDelay(pdMS_TO_TICKS(500));
    return ok;
}

void MicManager::_taskLoop() {
    while (true) {
        xSemaphoreTake(_sem, portMAX_DELAY);
        _recording = true;

        Serial.printf("[MIC] PSRAM libre: %u bytes\n",
            (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

        int16_t* pcm = (int16_t*)ps_malloc(SAMPLE_COUNT * sizeof(int16_t));
        if (!pcm) {
            Serial.println("[MIC] Sin PSRAM para PCM");
            g_micActive = false;
            _recording  = false;
            continue;
        }

        // Esperar mutex con timeout: si AiTask lleva demasiado tiempo, cancelar
        bool gotMutex = !g_audioMutex ||
            (xSemaphoreTake(g_audioMutex, pdMS_TO_TICKS(20000)) == pdTRUE);
        if (!gotMutex) {
            Serial.println("[MIC] Timeout mutex — cancelando grabacion");
            free(pcm);
            g_micActive = false;
            _recording  = false;
            continue;
        }

        M5.Speaker.end();
        vTaskDelay(pdMS_TO_TICKS(300));

        _initMic();

        // g_micActive ya esta a true desde triggerRecord(); refrescamos por si
        // hubo un reset interno en algun camino raro.
        g_micActive = true;
        Serial.println("[MIC] grabando...");
        uint32_t dispStart = millis();

        bool ok = M5.Mic.record(pcm, SAMPLE_COUNT, SAMPLE_RATE);

        if (!ok) {
            Serial.println("[MIC] record() fallo — reintentando con re-init");
            _initMic();
            ok = M5.Mic.record(pcm, SAMPLE_COUNT, SAMPLE_RATE);
        }

        if (!ok) {
            Serial.println("[MIC] Error definitivo al grabar");
            uint32_t elapsed = millis() - dispStart;
            if (elapsed < 2000) vTaskDelay(pdMS_TO_TICKS(2000 - elapsed));
            g_micActive = false;
            Serial.println("[MIC] g_micActive=false (error)");
            M5.Mic.end();
            M5.Speaker.begin();
            if (g_audioMutex) xSemaphoreGive(g_audioMutex);
            free(pcm);
            _recording = false;
            continue;
        }

        // Timeout de 6s para isRecording (RECORD_SECS=3, margen amplio)
        uint32_t recDeadline = millis() + 6000;
        while (M5.Mic.isRecording()) {
            if (millis() > recDeadline) {
                Serial.println("[MIC] WARN: isRecording() timeout");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        uint32_t recMs = millis() - dispStart;
        // Garantizar visibilidad mínima de 2000ms para que AnimTask renderice el overlay
        if (recMs < 2000) vTaskDelay(pdMS_TO_TICKS(2000 - recMs));
        g_micActive = false;
        Serial.printf("[MIC] g_micActive=false  dur=%ums  stack=%u\n",
            (unsigned)recMs,
            (unsigned)uxTaskGetStackHighWaterMark(nullptr));

        M5.Mic.end();

        int32_t maxAmp = 0;
        for (int i = 0; i < SAMPLE_COUNT; i++) {
            int32_t v = abs((int32_t)pcm[i]);
            if (v > maxAmp) maxAmp = v;
        }
        Serial.printf("[MIC] amp=%d  [0]=%d [100]=%d\n",
            maxAmp, (int)pcm[0], (int)pcm[100]);
        if (maxAmp < 300) Serial.println("[MIC] WARN: audio silencioso");

        M5.Speaker.begin();
        M5.Speaker.setVolume(100); // defensivo: end() puede resetear el volumen
        vTaskDelay(pdMS_TO_TICKS(100));
        if (g_audioMutex) xSemaphoreGive(g_audioMutex);

        size_t wavLen = 44 + SAMPLE_COUNT * 2;
        uint8_t* wav  = (uint8_t*)ps_malloc(wavLen);
        if (!wav) {
            Serial.println("[MIC] Sin PSRAM para WAV");
            free(pcm);
            _recording = false;
            continue;
        }

        _buildWavHeader(wav, SAMPLE_COUNT);
        memcpy(wav + 44, pcm, SAMPLE_COUNT * 2);
        free(pcm);

        _wavBuf   = wav;
        _wavLen   = wavLen;
        _pending  = true;
        _recording = false;
    }
}

void micTask(void* pvParams) {
    static_cast<MicManager*>(pvParams)->_taskLoop();
    vTaskDelete(nullptr);
}
