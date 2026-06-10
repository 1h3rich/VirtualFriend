#include "BleServer.h"
#include "Config.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

static NimBLEServer*         s_server  = nullptr;
static NimBLECharacteristic* s_dataChr = nullptr;

// Reanudar advertising al desconectar para que el M5Stack pueda reconectar solo
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* srv) override {
        Serial.println("[BLE] Cliente conectado");
    }
    void onDisconnect(NimBLEServer* srv) override {
        Serial.println("[BLE] Cliente desconectado — anunciando de nuevo");
        NimBLEDevice::startAdvertising();
    }
};

void BleServer::begin() {
    NimBLEDevice::init(BLE_DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // máximo alcance dentro del coche

    s_server = NimBLEDevice::createServer();
    s_server->setCallbacks(new ServerCallbacks());

    NimBLEService* svc = s_server->createService(BLE_SERVICE_UUID);
    s_dataChr = svc->createCharacteristic(
        BLE_DATA_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    svc->start();

    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_SERVICE_UUID);
    adv->setScanResponse(true);
    adv->start();

    Serial.printf("[BLE] Anunciando como '%s'\n", BLE_DEVICE_NAME);
}

bool BleServer::hasClient() const {
    return s_server && s_server->getConnectedCount() > 0;
}

void BleServer::notify(const VehicleData& data) {
    if (!s_dataChr) return;

    char payload[64];
    int n = data.toPayload(payload, sizeof(payload));
    if (n <= 0) return;

    s_dataChr->setValue((uint8_t*)payload, n);
    if (hasClient()) s_dataChr->notify();
}
