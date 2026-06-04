#pragma once
#include <M5Unified.h>
#include "SensorMonitor.h"
#include "SpeechManager.h"

struct MicAudioMsg {
    uint8_t* data;
    size_t   len;
};

class AiManager {
public:
    void begin();
    void onEvent(SensorEvent ev);
    void onMicAudio(uint8_t* wavData, size_t wavLen); // AiTask libera el buffer

    bool isConnected() const;

private:
    SpeechManager _speech;
    QueueHandle_t _queue    = nullptr;
    QueueHandle_t _micQueue = nullptr;

    static void _taskEntry(void* param);
    void _taskLoop();
    void _process(SensorEvent ev);
    void _processListen(const uint8_t* wavData, size_t wavLen);

    static const char* _message (SensorEvent ev);
    static const char* _mood    (SensorEvent ev);
    static const char* _movement(SensorEvent ev);
};
