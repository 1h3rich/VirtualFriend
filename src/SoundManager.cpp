#include "SoundManager.h"
#include <freertos/semphr.h>

extern SemaphoreHandle_t g_audioMutex;

void SoundManager::begin() {
    M5.Speaker.begin();
    M5.Speaker.setVolume(100);
}

void SoundManager::_tone(uint32_t freq, uint32_t durationMs, float volume) {
    M5.Speaker.tone(freq, durationMs, 0, volume);
    delay(durationMs + 10);
}

void SoundManager::play(SoundEffect fx) {
    // Si el I2S está en uso (grabando mic o reproduciendo IA), saltar el efecto.
    // trylock con timeout 0: no bloquea, no rompe el I2S del mic/speaker.
    if (g_audioMutex && xSemaphoreTake(g_audioMutex, 0) != pdTRUE) return;

    switch (fx) {
        case SoundEffect::STARTUP:
            _tone(523, 100);
            _tone(659, 100);
            _tone(784, 150);
            break;

        case SoundEffect::BUMP:
            _tone(220, 80, 0.4f);
            _tone(180, 80, 0.3f);
            break;

        case SoundEffect::ACCEL:
            _tone(440, 60);
            _tone(550, 60);
            break;

        case SoundEffect::BRAKE:
            _tone(330, 120, 0.35f);
            break;

        case SoundEffect::ALERT:
            _tone(880, 150, 0.5f);
            delay(80);
            _tone(880, 150, 0.5f);
            break;
    }

    if (g_audioMutex) xSemaphoreGive(g_audioMutex);
}
