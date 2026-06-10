#pragma once
#include <stdint.h>
#include <stdio.h>

// Datos limpios del coche. El dongle traduce tramas CAN a esta estructura
// y la envía por BLE como texto simple (ver toPayload).
struct VehicleData {
    int   rpm            = 0;     // PID 0x0C
    int   speed          = 0;     // PID 0x0D (km/h)
    int   coolantTemp    = 0;     // PID 0x05 (°C)
    float batteryVoltage = 0.0f;  // PID 0x42 (V)
    bool  milOn          = false; // PID 0x01, bit MIL (testigo de avería)
    bool  valid          = false; // true si la última ronda de lecturas respondió

    // Serializa al formato que espera el M5Stack:
    //   rpm=1850;speed=72;coolant=91;volt=14.2;mil=0
    // Devuelve los bytes escritos (sin contar el '\0').
    int toPayload(char* out, size_t outLen) const {
        return snprintf(out, outLen,
            "rpm=%d;speed=%d;coolant=%d;volt=%.1f;mil=%d",
            rpm, speed, coolantTemp, batteryVoltage, milOn ? 1 : 0);
    }
};
