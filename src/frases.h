#pragma once
#include "eventos.h"

// ============================================================
// FRASES PREDEFINIDAS DEL COMPAÑERO VIRTUAL
// ============================================================
//
// ¿Qué es un array?
// Un array es una lista de elementos del mismo tipo guardados
// en orden. Se accede a cada elemento con un índice numérico
// que empieza en 0.
//
//   const char* frasesNormales[] = { "Frase A", "Frase B" };
//   frasesNormales[0] → "Frase A"
//   frasesNormales[1] → "Frase B"
//
// ¿Qué es "const char*"?
// En C++, el texto (strings) se puede representar de varias
// formas. "const char*" es un puntero a una cadena de
// caracteres que no vamos a modificar. Es la forma clásica
// de manejar texto en C para Arduino, y funciona muy bien
// en placas con memoria limitada.
//
// "const" significa que el texto no cambiará durante
// la ejecución del programa.
//
// ¿Qué es "PROGMEM"?
// En ESP32/Arduino, se puede almacenar texto en memoria flash
// (más abundante) en lugar de RAM (más limitada). Para el
// Prototipo Nivel 1 no lo usamos para mantenerlo sencillo,
// pero es bueno saber que existe para versiones futuras.
// ============================================================

// Número de frases por categoría
// Definir estas constantes evita errores al calcular índices.
static const int NUM_FRASES_INICIO        = 3;
static const int NUM_FRASES_NORMALES      = 3;
static const int NUM_FRASES_FRENAZO_SUAVE = 3;
static const int NUM_FRASES_FRENAZO_FUERTE= 3;
static const int NUM_FRASES_VOLANTAZO     = 3;
static const int NUM_FRASES_IMPACTO       = 3;
static const int NUM_FRASES_REPOSO        = 3;
static const int NUM_FRASES_ERROR         = 3;

// ---- Frases de inicio ----
static const char* frasesInicio[NUM_FRASES_INICIO] = {
    "Sistema iniciado. Contra todo pronostico, estoy vivo.",
    "Despertando... otra vez en un coche. Genial.",
    "Listo para acompañarte. O al menos para juzgarte en silencio."
};

// ---- Frases normales ----
static const char* frasesNormales[NUM_FRASES_NORMALES] = {
    "Todo tranquilo. Sospechoso, pero tranquilo.",
    "Ruta en marcha. Yo vigilo, tu intenta no hacer nada raro.",
    "No detecto problemas. Lo cual, viniendo de un coche, ya es una victoria."
};

// ---- Frases para frenazo suave ----
static const char* frasesFrenazoSuave[NUM_FRASES_FRENAZO_SUAVE] = {
    "Eso ha sido un frenazo suave. Mi dignidad apenas ha sufrido danos.",
    "Tranquilo, solo he visto pasar mi vida digital por delante.",
    "Frenada registrada. Elegante no ha sido, pero registrada queda."
};

// ---- Frases para frenazo fuerte ----
static const char* frasesFrenazoFuerte[NUM_FRASES_FRENAZO_FUERTE] = {
    "Vale, eso ha sido serio. Esta todo bien?",
    "He detectado un frenazo fuerte. Revisad que todos esteis bien.",
    "Eso no ha sido normal. Voy a dejar el sarcasmo aparcado un momento."
};

// ---- Frases para volantazo ----
static const char* frasesVolantazo[NUM_FRASES_VOLANTAZO] = {
    "Eso ha sido un volantazo o has intentado esquivar un pensamiento intrusivo?",
    "Movimiento lateral detectado. Mi estabilidad emocional acaba de dimitir.",
    "Volantazo registrado. El coche y yo necesitamos explicaciones."
};

// ---- Frases para impacto ----
static const char* frasesImpacto[NUM_FRASES_IMPACTO] = {
    "Ahora sin bromas. He detectado un impacto fuerte. Revisad que todo este bien.",
    "Impacto detectado. Si hay algun problema, hay que pedir ayuda.",
    "Alerta seria. He detectado un golpe fuerte."
};

// ---- Frases de reposo ----
static const char* frasesReposo[NUM_FRASES_REPOSO] = {
    "Me quedo en reposo. No hagais nada estupido sin mi.",
    "Modo vigilancia activado. Dramaticamente necesario.",
    "Voy a fingir que duermo, pero sigo juzgando."
};

// ---- Frases de error ----
static const char* frasesError[NUM_FRASES_ERROR] = {
    "No se que ha pasado, pero no me gusta.",
    "Evento desconocido. Mi confusion es total.",
    "Algo raro ha ocurrido. Y esta vez no parece culpa mia."
};

// ============================================================
// FUNCIÓN: elegirFrase
// ============================================================
// Recibe un evento y devuelve una frase adecuada para ese evento.
//
// ¿Cómo funciona la selección aleatoria?
// random(n) en Arduino devuelve un número entre 0 y n-1.
// Con eso elegimos una frase al azar del array correspondiente.
//
// ¿Qué es "switch"?
// Es una forma ordenada de comparar una variable con varios
// valores posibles y ejecutar código diferente según el caso.
// Es equivalente a una cadena de if/else if, pero más legible
// cuando hay muchos casos.
// ============================================================
inline const char* elegirFrase(EventoSistema evento) {
    switch (evento) {
        case EventoSistema::NORMAL:
            return frasesNormales[random(NUM_FRASES_NORMALES)];

        case EventoSistema::FRENAZO_SUAVE:
            return frasesFrenazoSuave[random(NUM_FRASES_FRENAZO_SUAVE)];

        case EventoSistema::FRENAZO_FUERTE:
            return frasesFrenazoFuerte[random(NUM_FRASES_FRENAZO_FUERTE)];

        case EventoSistema::VOLANTAZO:
            return frasesVolantazo[random(NUM_FRASES_VOLANTAZO)];

        case EventoSistema::IMPACTO:
            return frasesImpacto[random(NUM_FRASES_IMPACTO)];

        case EventoSistema::REPOSO:
            return frasesReposo[random(NUM_FRASES_REPOSO)];

        case EventoSistema::DESCONOCIDO:
        default:
            return frasesError[random(NUM_FRASES_ERROR)];
    }
}
