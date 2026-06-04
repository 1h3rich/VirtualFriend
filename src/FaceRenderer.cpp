#include "FaceRenderer.h"
#include <math.h>

extern volatile bool g_micActive;
extern volatile bool g_aiThinking;

void FaceRenderer::_drawGlow(int cx, int cy, int w, int h, uint32_t color) {
    _cv->fillRoundRect(cx-w/2-10, cy-h/2-10, w+20, h+20, EYE_R+4, _dim(color, 0.12f));
    _cv->fillRoundRect(cx-w/2-5,  cy-h/2-5,  w+10, h+10, EYE_R+2, _dim(color, 0.28f));
    _cv->fillRoundRect(cx-w/2-2,  cy-h/2-2,  w+4,  h+4,  EYE_R+1, _dim(color, 0.55f));
}

void FaceRenderer::_drawCurve(int cx, int cy, int w, int curvDir, int thick, uint16_t col) {
    const int N = 10;
    for (int i = 0; i < N; i++) {
        float t0 = (float)i / N;
        float t1 = (float)(i+1) / N;
        int x0 = cx - w/2 + (int)(t0 * w);
        int x1 = cx - w/2 + (int)(t1 * w);
        int y0 = cy - curvDir * (int)(MOUTH_CURV * sinf(t0 * M_PI));
        int y1 = cy - curvDir * (int)(MOUTH_CURV * sinf(t1 * M_PI));
        for (int d = -(thick/2); d <= thick/2; d++) {
            _cv->drawLine(x0, y0+d, x1, y1+d, col);
        }
    }
}

void FaceRenderer::_drawEye(int cx, int cy, EyeShape shape, uint32_t color, uint32_t bg, bool glow) {
    uint16_t col  = _c16(color);
    uint16_t bg16 = _c16(bg);
    int w = EYE_W, h = EYE_H, r = EYE_R;

    switch (shape) {

        case EyeShape::CLOSED:
            _cv->fillRoundRect(cx-w/2, cy-3, w, 6, 3, col);
            return;

        case EyeShape::SLIT:
            if (glow) _drawGlow(cx, cy, w, h/4, color);
            _cv->fillRoundRect(cx-w/2, cy-h/8, w, h/4, 4, col);
            _cv->fillRoundRect(cx-w/2+6, cy-h/8+2, w/3, 3, 2, _dim(color, 1.4f > 1.f ? 1.f : 1.4f));
            return;

        case EyeShape::WIDE: {
            int ww = w+12, hh = h+12;
            if (glow) _drawGlow(cx, cy, ww, hh, color);
            _cv->fillRoundRect(cx-ww/2, cy-hh/2, ww, hh, EYE_R+8, col);
            _cv->fillCircle(cx-ww/4, cy-hh/4, 5, _dim(0xFFFFFF, 0.7f));
            return;
        }

        case EyeShape::SQUINT:
            if (glow) _drawGlow(cx, cy, w, h, color);
            _cv->fillRoundRect(cx-w/2, cy-h/2, w, h, r, col);
            _cv->fillRect(cx-w/2-1, cy-h/2-1, w+2, (h*45)/100, bg16);
            _cv->fillCircle(cx-w/4, cy+5, 4, _dim(0xFFFFFF, 0.5f));
            return;

        case EyeShape::ANGRY_L:
            if (glow) _drawGlow(cx, cy, w, h, color);
            _cv->fillRoundRect(cx-w/2, cy-h/2, w, h, r, col);
            for (int i = 0; i < h/2; i++) {
                int cut = (i * w/2) / (h/2);
                _cv->fillRect(cx, cy-h/2+i, cut, 1, bg16);
            }
            return;

        case EyeShape::ANGRY_R:
            if (glow) _drawGlow(cx, cy, w, h, color);
            _cv->fillRoundRect(cx-w/2, cy-h/2, w, h, r, col);
            for (int i = 0; i < h/2; i++) {
                int cut = (i * w/2) / (h/2);
                _cv->fillRect(cx-cut, cy-h/2+i, cut, 1, bg16);
            }
            return;

        case EyeShape::SMIRK:
            if (glow) _drawGlow(cx, cy, w, h/3, color);
            _cv->fillRoundRect(cx-w/2, cy-h/6, w, h/3, 6, col);
            _cv->fillRect(cx-w/2, cy-h/6, w/3, h/6, bg16);
            return;

        case EyeShape::SQUARE:
        default:
            if (glow) _drawGlow(cx, cy, w, h, color);
            _cv->fillRoundRect(cx-w/2, cy-h/2, w, h, r, col);
            _cv->fillCircle(cx-w/4, cy-h/4, 5, _dim(0xFFFFFF, 0.55f));
            _cv->fillCircle(cx-w/4+8, cy-h/4+6, 2, _dim(0xFFFFFF, 0.35f));
            return;
    }
}

void FaceRenderer::_drawMouth(int cx, int cy, MouthShape shape, uint32_t color, uint32_t bg, bool glow) {
    uint16_t col  = _c16(color);
    uint16_t bg16 = _c16(bg);

    switch (shape) {

        case MouthShape::FLAT:
            if (glow) {
                _cv->fillRoundRect(cx-MOUTH_W/2-3, cy-4, MOUTH_W+6, 8, 4, _dim(color, 0.18f));
            }
            _cv->fillRoundRect(cx-MOUTH_W/2, cy-2, MOUTH_W, 4, 2, col);
            break;

        case MouthShape::SMILE:
            if (glow) {
                _drawCurve(cx, cy, MOUTH_W+8, +1, 7, _dim(color, 0.25f));
            }
            _drawCurve(cx, cy, MOUTH_W, +1, 3, col);
            break;

        case MouthShape::GRIN: {
            if (glow) _drawCurve(cx, cy, MOUTH_W+8, +1, 7, _dim(color, 0.22f));
            for (int x = cx-MOUTH_W/2; x <= cx+MOUTH_W/2; x++) {
                float t = (float)(x - (cx-MOUTH_W/2)) / MOUTH_W;
                int ytop = cy - (int)(MOUTH_CURV * sinf(t * M_PI));
                int ybot = cy + 6;
                if (ytop < ybot) _cv->drawFastVLine(x, ytop, ybot - ytop, col);
            }
            _drawCurve(cx, cy, MOUTH_W, +1, 2, _dim(0xFFFFFF, 0.45f));
            break;
        }

        case MouthShape::FROWN:
            if (glow) _drawCurve(cx, cy, MOUTH_W+8, -1, 7, _dim(color, 0.25f));
            _drawCurve(cx, cy, MOUTH_W, -1, 3, col);
            break;

        case MouthShape::SMIRK:
            if (glow) {
                _cv->fillRoundRect(cx-MOUTH_W/2-2, cy-4, MOUTH_W/2+4, 8, 3, _dim(color, 0.18f));
            }
            _cv->fillRoundRect(cx-MOUTH_W/2, cy-2, MOUTH_W/2, 4, 2, col);
            {
                int hw = MOUTH_W/2;
                for (int i = 0; i < 5; i++) {
                    float t0 = (float)i / 5;
                    float t1 = (float)(i+1) / 5;
                    int x0 = cx + (int)(t0 * hw);
                    int x1 = cx + (int)(t1 * hw);
                    int y0 = cy - (int)(MOUTH_CURV * sinf(t0 * M_PI * 0.9f));
                    int y1 = cy - (int)(MOUTH_CURV * sinf(t1 * M_PI * 0.9f));
                    for (int d = -1; d <= 1; d++)
                        _cv->drawLine(x0, y0+d, x1, y1+d, col);
                }
            }
            break;

        case MouthShape::OPEN:
            if (glow) _cv->fillEllipse(cx, cy, 18, 13, _dim(color, 0.25f));
            _cv->fillEllipse(cx, cy, 14, 10, col);
            _cv->fillEllipse(cx, cy, 8, 5, bg16);
            break;

        case MouthShape::THIN:
            _cv->fillRoundRect(cx-MOUTH_W/3, cy-1, 2*MOUTH_W/3, 2, 1, col);
            break;
    }
}

void FaceRenderer::draw(const FaceParams& params, const char* phrase, bool phraseIsAlert) {
    if (!_cv) return;
    _cv->fillScreen(_c16(params.bgColor));

    int lx = EYE_LX + params.eyeDX;
    int rx = EYE_RX + params.eyeDX;
    int ey = EYE_CY + params.eyeDY;
    int my = MOUTH_CY + params.mouthDY;

    _drawEye(lx, ey, params.eyeL, params.eyeColor, params.bgColor, params.glowEnabled);
    _drawEye(rx, ey, params.eyeR, params.eyeColor, params.bgColor, params.glowEnabled);
    _drawMouth(MOUTH_CX, my, params.mouth, params.eyeColor, params.bgColor, params.glowEnabled);

    if (phrase && phrase[0] != '\0') _drawPhrase(phrase, phraseIsAlert);

    if (g_micActive) {
        _cv->fillRect(0, 212, 320, 28, TFT_RED);
        _cv->setTextColor(TFT_WHITE, TFT_RED);
        _cv->drawCenterString("HABLA AHORA", 160, 214, 4);
    } else if (g_aiThinking) {
        _cv->fillRect(0, 212, 320, 28, TFT_BLUE);
        _cv->setTextColor(TFT_WHITE, TFT_BLUE);
        _cv->drawCenterString("PENSANDO...", 160, 214, 4);
    }
    _cv->pushSprite(0, 0);
}

void FaceRenderer::_drawPhrase(const char* text, bool isAlert) {
    uint16_t color = isAlert ? _c16(0xFF3020) : _c16(0x80C8B8);
    _cv->setTextColor(color);
    _cv->setTextSize(2);
    _cv->setTextDatum(TC_DATUM);
    _cv->drawString(text, SCR_W/2, SCR_H - 28);
}
