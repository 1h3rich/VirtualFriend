#pragma once
#include <M5Unified.h>

class SpeechManager {
public:
    // Decodifica un buffer MP3 y lo reproduce por el altavoz.
    // mp3Data: buffer en PSRAM o heap. len: bytes.
    // Devuelve true si se reprodujo correctamente.
    bool playMp3(const uint8_t* mp3Data, size_t len);
};
