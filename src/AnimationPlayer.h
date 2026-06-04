#pragma once
#include <M5Unified.h>
#include <SD.h>
#include <AnimatedGIF.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

enum class AnimEvent {
    NONE,
    STARTUP,
    HARD_ACCEL,
    HARD_BRAKE,
    BUMP,
    // Expresiones IA (resultado de la respuesta del Worker)
    AI_HAPPY,
    AI_SARCASTIC,
    AI_SERIOUS,
    AI_SURPRISED,
    AI_SLEEPY,
};

// Estructura de archivos en la SD:
//   /faces/idle/idle.gif           — bucle continuo en reposo
//   /faces/accel/accel.gif         — one-shot al acelerar
//   /faces/accel/accel2.gif        — variante aleatoria (opcional)
//   /faces/brake/brake.gif         — one-shot al frenar
//   /faces/brake/brake2.gif        — opcional
//   /faces/bump/bump.gif           — one-shot al bache
//   /faces/startup/startup.gif     — animación de arranque

class AnimationPlayer {
public:
    AnimationPlayer();
    void begin();
    bool sdAvailable() const { return _sdOk; }

    // Encola un evento — seguro desde cualquier tarea/núcleo
    void triggerEvent(AnimEvent ev);

    // Convierte una expresión IA (string del Worker) a AnimEvent y la encola.
    // Acepta: happy, sarcastic, serious, surprised, sleepy. Otros valores se ignoran.
    void triggerAiExpression(const char* exprName);

    // Loop interno de la tarea de animación (no llamar directamente)
    void _animTaskLoop();

private:
    bool          _sdOk;
    QueueHandle_t _eventQueue;
    AnimatedGIF   _gif;

    static const int MAX_VARIANTS = 5;

    bool _playGifOnce(const char* path);
    bool _playGifLoop(const char* path);  // loop interrumpible por evento
    bool _exists(const char* path);
    bool _pickPath(const char* base, char* out, size_t outLen);
    void _vectorFallback(AnimEvent ev);
};

void animPlayerTask(void* pvParams);
