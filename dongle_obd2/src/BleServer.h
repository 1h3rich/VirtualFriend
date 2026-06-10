#pragma once
#include "VehicleData.h"

// Servidor BLE GATT del dongle.
// Expone una característica de notificación con los datos del coche en texto:
//   rpm=1850;speed=72;coolant=91;volt=14.2;mil=0
// El M5Stack se suscribe y recibe una notificación por cada ronda de sondeo.
class BleServer {
public:
    void begin();

    // true si hay al menos un cliente (el M5Stack) conectado
    bool hasClient() const;

    // Serializa y notifica los datos a los clientes suscritos
    void notify(const VehicleData& data);
};
