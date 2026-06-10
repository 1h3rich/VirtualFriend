#include "obd.h"
#include "Config.h"
#include <M5Unified.h>

#if !OBD_SIMULATION_MODE
#include "BleObdClient.h"
static BleObdClient s_ble;
#endif

OBDManager::OBDManager()
    : _connected(false), _rpm(0), _speed(0),
      _engineTemp(0), _engineError(false), _lastSimMs(0) {}

void OBDManager::begin() {
#if OBD_SIMULATION_MODE
    _connected = true;
    Serial.println("[OBD] Modo simulacion activo");
#else
    _connected = false;
    s_ble.begin();
    Serial.println("[OBD] Buscando dongle por BLE...");
#endif
}

void OBDManager::update() {
#if OBD_SIMULATION_MODE
    uint32_t now = millis();
    if (now - _lastSimMs < 2000) return;
    _lastSimMs = now;

    _rpm        = random(800, 4500);
    _speed      = random(0, 130);
    _engineTemp = random(70, 105);
    _engineError = random(0, 100) >= 95;  // 5% de probabilidad de error
#else
    _connected = s_ble.isLinked();
    if (!_connected) {
        _rpm = 0; _speed = 0; _engineTemp = 0; _engineError = false;
        return;
    }
    _rpm         = s_ble.getRPM();
    _speed       = s_ble.getSpeed();
    _engineTemp  = s_ble.getCoolantTemp();
    _engineError = s_ble.getMilOn();  // testigo de avería del cuadro
#endif
}
