#define MINIMP3_IMPLEMENTATION
#define MINIMP3_ONLY_MP3
#include "minimp3.h"

#include "SpeechManager.h"
#include <freertos/semphr.h>

extern SemaphoreHandle_t g_audioMutex;

bool SpeechManager::playMp3(const uint8_t* mp3Data, size_t mp3Len) {
    if (!mp3Data || mp3Len == 0) return false;

    // ── Decodificador en heap (no en stack — ~7KB) ────────────────────────────
    mp3dec_t* dec = (mp3dec_t*)malloc(sizeof(mp3dec_t));
    if (!dec) { Serial.println("[TTS] Sin heap para decoder"); return false; }
    mp3dec_init(dec);

    // ── Buffer PCM en PSRAM ───────────────────────────────────────────────────
    size_t pcmCapacity = mp3Len * 12; // ratio real: ~11× para 32kbps → 22050Hz PCM
    int16_t* pcmBuf = (int16_t*)ps_malloc(pcmCapacity);
    if (!pcmBuf) {
        Serial.println("[TTS] Sin PSRAM para PCM");
        free(dec);
        return false;
    }

    // ── Frame buffer en heap (no en stack — ~4.5KB) ───────────────────────────
    int16_t* framePcm = (int16_t*)malloc(MINIMP3_MAX_SAMPLES_PER_FRAME * 2 * sizeof(int16_t));
    if (!framePcm) {
        Serial.println("[TTS] Sin heap para framePcm");
        free(pcmBuf);
        free(dec);
        return false;
    }

    // ── Decode ────────────────────────────────────────────────────────────────
    mp3dec_frame_info_t info = {};
    size_t   totalSamples = 0;
    int      offset       = 0;
    uint32_t sampleRate   = 22050;
    bool     stereo       = false;

    while (offset < (int)mp3Len) {
        int samples = mp3dec_decode_frame(dec,
                          mp3Data + offset, mp3Len - offset,
                          framePcm, &info);
        if (info.frame_bytes <= 0) break;
        offset += info.frame_bytes;

        if (samples > 0) {
            sampleRate = info.hz;
            stereo     = (info.channels == 2);
            size_t frameSz = (size_t)samples * info.channels;
            if ((totalSamples + frameSz) * sizeof(int16_t) < pcmCapacity) {
                memcpy(pcmBuf + totalSamples, framePcm, frameSz * sizeof(int16_t));
                totalSamples += frameSz;
            }
        }
    }

    free(framePcm);
    free(dec);

    Serial.printf("[TTS] %u muestras @ %u Hz\n", (unsigned)totalSamples, sampleRate);

    if (totalSamples > 0) {
        if (g_audioMutex) xSemaphoreTake(g_audioMutex, portMAX_DELAY);
        M5.Speaker.playRaw(pcmBuf, totalSamples, sampleRate, stereo, 1, 0, false);
        uint32_t playDeadline = millis() + 12000;
        while (M5.Speaker.isPlaying()) {
            if (millis() > playDeadline) {
                Serial.println("[TTS] WARN: isPlaying() timeout — reiniciando speaker");
                M5.Speaker.end();
                vTaskDelay(pdMS_TO_TICKS(100));
                M5.Speaker.begin();
                M5.Speaker.setVolume(100);
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        // Esperar dentro del mutex: evita que MicTask grabe el eco físico de la sala
        vTaskDelay(pdMS_TO_TICKS(600));
        if (g_audioMutex) xSemaphoreGive(g_audioMutex);
    }

    free(pcmBuf);
    return totalSamples > 0;
}
