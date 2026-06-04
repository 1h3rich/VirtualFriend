#pragma once
#include <M5Unified.h>
#include "ExpressionManager.h"

class FaceRenderer {
public:
    FaceRenderer() = default;
    void begin() {}
    void setCanvas(M5Canvas* c) { _cv = c; }
    void draw(const FaceParams& params, const char* phrase = nullptr, bool phraseIsAlert = false);

private:
    M5Canvas* _cv = nullptr;

    static const int SCR_W   = 320;
    static const int SCR_H   = 240;

    // Ojos
    static const int EYE_CY  = 100;
    static const int EYE_LX  = 108;
    static const int EYE_RX  = 212;
    static const int EYE_W   = 60;
    static const int EYE_H   = 64;
    static const int EYE_R   = 12;

    // Boca
    static const int MOUTH_CX = SCR_W / 2;
    static const int MOUTH_CY = 170;
    static const int MOUTH_W  = 60;
    static const int MOUTH_CURV = 9;

    void _drawEye  (int cx, int cy, EyeShape   shape, uint32_t color, uint32_t bg, bool glow);
    void _drawMouth(int cx, int cy, MouthShape shape, uint32_t color, uint32_t bg, bool glow);
    void _drawGlow (int cx, int cy, int w, int h, uint32_t color);
    void _drawPhrase(const char* text, bool isAlert);
    void _drawCurve(int cx, int cy, int w, int curvDir, int thick, uint16_t col);

    uint16_t _c16(uint32_t rgb) {
        return (((rgb>>16)&0xFF)>>3)<<11 | (((rgb>>8)&0xFF)>>2)<<5 | ((rgb&0xFF)>>3);
    }
    uint16_t _dim(uint32_t rgb, float f) {
        uint8_t r = ((rgb>>16)&0xFF) * f;
        uint8_t g = ((rgb>>8) &0xFF) * f;
        uint8_t b = ( rgb     &0xFF) * f;
        return _c16((uint32_t)(r<<16)|(g<<8)|b);
    }
};
