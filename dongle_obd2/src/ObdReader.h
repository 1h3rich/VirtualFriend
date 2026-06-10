#pragma once
#include "CanManager.h"
#include "VehicleData.h"

// Lector OBD2 sobre CAN (ISO 15765-4, modo 01).
// SOLO lectura de PIDs estándar. Nunca borra DTCs ni escribe en ECUs.
class ObdReader {
public:
    void begin(CanManager* can) { _can = can; }

    // Pide los PIDs prioritarios y rellena data.
    // data->valid = true si al menos RPM o velocidad respondieron.
    void readAll(VehicleData* data);

private:
    CanManager* _can = nullptr;

    // Pide un PID del modo 01 y devuelve los bytes A y B de la respuesta.
    // Devuelve el número de bytes de datos útiles (0 si no hubo respuesta).
    int _requestPid(uint8_t pid, uint8_t* a, uint8_t* b);
};
