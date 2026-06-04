#pragma once
#include <M5Unified.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Gestiona la grabación de audio por micrófono.
// El usuario toca la pantalla → se graban RECORD_SECS segundos a 16kHz →
// se genera un WAV en PSRAM listo para enviar al Worker.
class MicManager {
public:
    void begin();

    // Llama desde CtrlTask al detectar toque. Ignora si ya graba o hay pending.
    void triggerRecord();

    bool isRecording()  const { return _recording; }
    bool hasPending()   const { return _pending; }

    // Devuelve el buffer WAV (ps_malloc'd) y su longitud.
    // Solo llamar si hasPending() == true. El llamador libera el buffer con free().
    void takePending(uint8_t** outBuf, size_t* outLen);

    void _taskLoop(); // interno

private:
    static const int SAMPLE_RATE  = 16000;
    static const int RECORD_SECS  = 3;
    static const int SAMPLE_COUNT = SAMPLE_RATE * RECORD_SECS;

    SemaphoreHandle_t _sem      = nullptr;
    volatile bool     _recording = false;
    volatile bool     _pending   = false;

    uint8_t* _wavBuf = nullptr;
    size_t   _wavLen = 0;

    void _buildWavHeader(uint8_t* buf, uint32_t numSamples);
    bool _initMic();
};

void micTask(void* pvParams);
