#pragma once
#include <stdint.h>
#include <stddef.h>

// Trama CAN simplificada (clásica, 11 bits, hasta 8 bytes de datos)
struct CanFrame {
    uint32_t id   = 0;
    uint8_t  len  = 0;
    uint8_t  data[8] = {0};
};

// Envoltorio del driver TWAI del ESP32-C3.
// SOLO lectura del bus: este módulo nunca escribe en ECUs ni envía
// comandos propietarios; únicamente peticiones OBD2 estándar (modo 01).
class CanManager {
public:
    // Inicializa el driver TWAI. Devuelve true si el bus quedó operativo.
    bool begin();
    void end();

    bool isReady() const { return _ready; }

    // Envía una trama. Devuelve true si se encoló correctamente.
    bool send(const CanFrame& frame);

    // Espera una trama hasta timeoutMs. Devuelve true si llegó una.
    bool receive(CanFrame* out, uint32_t timeoutMs);

private:
    bool _ready = false;
};
