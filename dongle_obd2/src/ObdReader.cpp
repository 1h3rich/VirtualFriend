#include "ObdReader.h"
#include "Config.h"
#include <Arduino.h>

// PIDs OBD2 modo 01 (ISO 15765-4)
static const uint8_t PID_MIL_STATUS  = 0x01; // A bit7 = testigo de avería
static const uint8_t PID_COOLANT     = 0x05; // A-40 °C
static const uint8_t PID_RPM         = 0x0C; // ((A*256)+B)/4
static const uint8_t PID_SPEED       = 0x0D; // A km/h
static const uint8_t PID_BATTERY     = 0x42; // ((A*256)+B)/1000 V

int ObdReader::_requestPid(uint8_t pid, uint8_t* a, uint8_t* b) {
    if (!_can || !_can->isReady()) return 0;

    // Trama single-frame: [longitud=2, modo=01, PID, relleno...]
    CanFrame req;
    req.id      = OBD_REQUEST_ID;
    req.len     = 8;
    req.data[0] = 0x02;
    req.data[1] = 0x01;
    req.data[2] = pid;
    // resto a 0x00 (algunos coches prefieren relleno 0xAA; cambiar si no responde)

    if (!_can->send(req)) return 0;

    // Esperar la respuesta de la ECU: [len, 0x41, PID, A, B, ...]
    uint32_t deadline = millis() + OBD_RESPONSE_TIMEOUT_MS;
    CanFrame resp;
    while ((int32_t)(deadline - millis()) > 0) {
        uint32_t remaining = deadline - millis();
        if (!_can->receive(&resp, remaining)) break;

        if (resp.id < OBD_RESPONSE_MIN || resp.id > OBD_RESPONSE_MAX) continue;
        if (resp.len < 4)        continue;
        if (resp.data[1] != 0x41) continue;  // respuesta a modo 01
        if (resp.data[2] != pid)  continue;  // respuesta a otro PID en vuelo

        *a = resp.data[3];
        *b = (resp.len >= 5) ? resp.data[4] : 0;
        return resp.data[0] - 2;  // bytes de datos útiles tras modo+PID
    }
    return 0;
}

void ObdReader::readAll(VehicleData* data) {
    uint8_t a = 0, b = 0;
    bool anyOk = false;

    if (_requestPid(PID_RPM, &a, &b) > 0) {
        data->rpm = ((int)a * 256 + b) / 4;
        anyOk = true;
    }
    if (_requestPid(PID_SPEED, &a, &b) > 0) {
        data->speed = a;
        anyOk = true;
    }
    if (_requestPid(PID_COOLANT, &a, &b) > 0) {
        data->coolantTemp = (int)a - 40;
    }
    if (_requestPid(PID_BATTERY, &a, &b) > 0) {
        data->batteryVoltage = ((int)a * 256 + b) / 1000.0f;
    }
    if (_requestPid(PID_MIL_STATUS, &a, &b) > 0) {
        data->milOn = (a & 0x80) != 0;
    }

    data->valid = anyOk;
}
