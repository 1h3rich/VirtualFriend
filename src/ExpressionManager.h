#pragma once
#include <stdint.h>

enum class Expression {
    NEUTRAL,
    HAPPY,
    SARCASTIC,
    SURPRISED,
    ALERT,
    SLEEPING,
    ANGRY
};

enum class EyeShape {
    SQUARE,     // cuadrado redondeado — neutro
    SQUINT,     // mitad superior cortada — feliz/cerrado parcial
    WIDE,       // más grande y circular — sorprendido
    SLIT,       // franja horizontal fina — alerta/serio
    CLOSED,     // línea horizontal — dormido/parpadeando
    ANGRY_L,    // esquina sup-interna cortada (ojo izquierdo enojado)
    ANGRY_R,    // esquina sup-interna cortada (ojo derecho enojado)
    SMIRK,      // slit con esquina inf redondeada — sarcástico
};

enum class MouthShape {
    FLAT,       // línea horizontal neutra  ——
    SMILE,      // arco hacia arriba        ⌣
    GRIN,       // sonrisa amplia rellena
    FROWN,      // arco hacia abajo         ⌢
    SMIRK,      // izquierda plana, derecha curvada
    OPEN,       // óvalo abierto (sorpresa)
    THIN,       // línea muy fina (tensión/alerta)
};

struct FaceParams {
    EyeShape  eyeL;
    EyeShape  eyeR;
    int       eyeDX;
    int       eyeDY;
    MouthShape mouth;
    int       mouthDY;      // offset vertical adicional de la boca
    uint32_t  eyeColor;
    uint32_t  bgColor;
    bool      glowEnabled;
};

class ExpressionManager {
public:
    ExpressionManager();
    void       setExpression(Expression expr);
    Expression getCurrent()  const { return _current; }
    FaceParams getParams()   const { return _params; }

private:
    Expression _current;
    FaceParams _params;
    void _build(Expression expr);
};
