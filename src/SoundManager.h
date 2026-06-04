#pragma once
#include <M5Unified.h>

enum class SoundEffect {
    STARTUP,
    BUMP,
    ACCEL,
    BRAKE,
    ALERT,
};

class SoundManager {
public:
    void begin();
    void play(SoundEffect fx);

private:
    void _tone(uint32_t freq, uint32_t durationMs, float volume = 0.3f);
};
