#pragma once
#include <stdint.h>

// Cliente BLE que se vincula con el dongle OBD2 (máquina de diagnosis).
// Escanea, conecta y se suscribe a las notificaciones de datos del coche.
// Reconecta solo si el dongle desaparece (p.ej. al quitar el contacto).
//
// Los getters son seguros desde cualquier tarea: los campos se actualizan
// bajo spinlock desde el callback de notificación de NimBLE.
class BleObdClient {
public:
    void begin();  // arranca la tarea de escaneo/conexión (BleTask, Core 0)

    // true si está conectado al dongle Y los datos son recientes (TTL en Config.h)
    bool isLinked() const;

    int   getRPM() const;
    int   getSpeed() const;
    int   getCoolantTemp() const;
    float getBatteryVoltage() const;
    bool  getMilOn() const;

    void _taskLoop();  // interno
};

void bleObdTask(void* pvParams);
