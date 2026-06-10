#include "BleObdClient.h"
#include "Config.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

// ── Estado compartido (escrito por el callback de NimBLE, leído por CtrlTask) ──

static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;

static volatile bool     s_connected   = false;
static volatile uint32_t s_lastDataMs  = 0;
static int   s_rpm     = 0;
static int   s_speed   = 0;
static int   s_coolant = 0;
static float s_volt    = 0.0f;
static bool  s_mil     = false;

// ── Parseo del payload del dongle ─────────────────────────────────────────────
// Formato: rpm=1850;speed=72;coolant=91;volt=14.2;mil=0

static void parsePayload(const char* p) {
    int rpm = 0, speed = 0, coolant = 0, mil = 0;
    float volt = 0.0f;

    int n = sscanf(p, "rpm=%d;speed=%d;coolant=%d;volt=%f;mil=%d",
                   &rpm, &speed, &coolant, &volt, &mil);
    if (n < 2) {
        Serial.printf("[BLE-OBD] payload no reconocido: %s\n", p);
        return;
    }

    portENTER_CRITICAL(&s_lock);
    s_rpm     = rpm;
    s_speed   = speed;
    s_coolant = coolant;
    s_volt    = volt;
    s_mil     = (mil != 0);
    portEXIT_CRITICAL(&s_lock);
    s_lastDataMs = millis();
}

static void notifyCB(NimBLERemoteCharacteristic* chr,
                     uint8_t* data, size_t len, bool isNotify) {
    char buf[80];
    size_t n = min(len, sizeof(buf) - 1);
    memcpy(buf, data, n);
    buf[n] = '\0';
    parsePayload(buf);
}

// ── Conexión ──────────────────────────────────────────────────────────────────

class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* c) override {
        Serial.println("[BLE-OBD] Conectado al dongle");
        s_connected = true;
    }
    void onDisconnect(NimBLEClient* c) override {
        Serial.println("[BLE-OBD] Dongle desconectado");
        s_connected = false;
    }
};

// Conecta y se suscribe. Devuelve false si algo falló (se reintenta en el loop).
static bool connectToDongle(NimBLEAdvertisedDevice* dev) {
    NimBLEClient* client = NimBLEDevice::createClient();
    client->setClientCallbacks(new ClientCallbacks(), true);
    client->setConnectTimeout(10);

    if (!client->connect(dev)) {
        NimBLEDevice::deleteClient(client);
        return false;
    }

    NimBLERemoteService* svc = client->getService(OBD_BLE_SERVICE_UUID);
    NimBLERemoteCharacteristic* chr =
        svc ? svc->getCharacteristic(OBD_BLE_DATA_UUID) : nullptr;

    if (!chr || !chr->canNotify() || !chr->subscribe(true, notifyCB)) {
        Serial.println("[BLE-OBD] Servicio/caracteristica no disponible");
        client->disconnect();
        NimBLEDevice::deleteClient(client);
        return false;
    }
    return true;
}

// ── Tarea: escanear → conectar → esperar desconexión → repetir ────────────────

void BleObdClient::_taskLoop() {
    NimBLEDevice::init("");
    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setActiveScan(true);

    while (true) {
        if (!s_connected) {
            NimBLEScanResults results = scan->start(5, false);
            NimBLEAdvertisedDevice found;
            bool haveDongle = false;

            for (int i = 0; i < results.getCount(); i++) {
                NimBLEAdvertisedDevice d = results.getDevice(i);
                if (d.isAdvertisingService(NimBLEUUID(OBD_BLE_SERVICE_UUID))) {
                    found = d;
                    haveDongle = true;
                    break;
                }
            }
            scan->clearResults();

            if (haveDongle) {
                Serial.println("[BLE-OBD] Dongle encontrado — conectando");
                connectToDongle(&found);
            }
            // Sin dongle a la vista: reintento tranquilo, no monopolizar la radio
            if (!s_connected) vTaskDelay(pdMS_TO_TICKS(10000));
        } else {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void bleObdTask(void* pvParams) {
    static_cast<BleObdClient*>(pvParams)->_taskLoop();
    vTaskDelete(nullptr);
}

// ── API pública ───────────────────────────────────────────────────────────────

void BleObdClient::begin() {
    xTaskCreatePinnedToCore(bleObdTask, "BleTask", 6144, this, 1, nullptr, 0);
}

bool BleObdClient::isLinked() const {
    return s_connected &&
           (millis() - s_lastDataMs) < OBD_BLE_DATA_TTL_MS;
}

int BleObdClient::getRPM() const {
    portENTER_CRITICAL(&s_lock); int v = s_rpm; portEXIT_CRITICAL(&s_lock);
    return v;
}
int BleObdClient::getSpeed() const {
    portENTER_CRITICAL(&s_lock); int v = s_speed; portEXIT_CRITICAL(&s_lock);
    return v;
}
int BleObdClient::getCoolantTemp() const {
    portENTER_CRITICAL(&s_lock); int v = s_coolant; portEXIT_CRITICAL(&s_lock);
    return v;
}
float BleObdClient::getBatteryVoltage() const {
    portENTER_CRITICAL(&s_lock); float v = s_volt; portEXIT_CRITICAL(&s_lock);
    return v;
}
bool BleObdClient::getMilOn() const {
    portENTER_CRITICAL(&s_lock); bool v = s_mil; portEXIT_CRITICAL(&s_lock);
    return v;
}
