#include "ExpressionManager.h"

static const uint32_t C_CYAN   = 0x00F0E0;
static const uint32_t C_RED    = 0xFF2010;
static const uint32_t C_YELLOW = 0xFFE000;
static const uint32_t C_DIM    = 0x00A090;

static const uint32_t BG_DARK  = 0x04080E;
static const uint32_t BG_RED   = 0x0E0202;
static const uint32_t BG_NIGHT = 0x020408;

ExpressionManager::ExpressionManager() {
    _current = Expression::NEUTRAL;
    _build(Expression::NEUTRAL);
}

void ExpressionManager::setExpression(Expression expr) {
    if (expr == _current) return;
    _current = expr;
    _build(expr);
}

void ExpressionManager::_build(Expression expr) {
    _params = {
        EyeShape::SQUARE, EyeShape::SQUARE,
        0, 0,
        MouthShape::FLAT, 0,
        C_CYAN, BG_DARK, true
    };

    switch (expr) {
        case Expression::NEUTRAL:
            break;

        case Expression::HAPPY:
            _params.eyeL    = EyeShape::SQUINT;
            _params.eyeR    = EyeShape::SQUINT;
            _params.mouth   = MouthShape::GRIN;
            _params.eyeDY   = 4;
            break;

        case Expression::SARCASTIC:
            _params.eyeL    = EyeShape::SQUINT;
            _params.eyeR    = EyeShape::SMIRK;
            _params.mouth   = MouthShape::SMIRK;
            _params.eyeDX   = 6;
            _params.eyeColor = C_DIM;
            break;

        case Expression::SURPRISED:
            _params.eyeL    = EyeShape::WIDE;
            _params.eyeR    = EyeShape::WIDE;
            _params.mouth   = MouthShape::OPEN;
            _params.eyeDY   = -6;
            _params.eyeColor = 0xCCFFFF;
            break;

        case Expression::ALERT:
            _params.eyeL    = EyeShape::SLIT;
            _params.eyeR    = EyeShape::SLIT;
            _params.mouth   = MouthShape::THIN;
            _params.eyeColor = C_RED;
            _params.bgColor  = BG_RED;
            break;

        case Expression::SLEEPING:
            _params.eyeL    = EyeShape::CLOSED;
            _params.eyeR    = EyeShape::CLOSED;
            _params.mouth   = MouthShape::FLAT;
            _params.eyeColor = C_DIM;
            _params.bgColor  = BG_NIGHT;
            _params.glowEnabled = false;
            break;

        case Expression::ANGRY:
            _params.eyeL    = EyeShape::ANGRY_L;
            _params.eyeR    = EyeShape::ANGRY_R;
            _params.mouth   = MouthShape::FROWN;
            _params.eyeColor = C_RED;
            _params.bgColor  = BG_RED;
            _params.eyeDY   = 6;
            break;
    }
}
