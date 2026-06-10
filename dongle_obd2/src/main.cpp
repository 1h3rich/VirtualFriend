// VirtualFriend — dongle OBD2 (máquina de diagnosis)
//
// Lee datos básicos del coche por CAN (ISO 15765-4) y los envía limpios
// por BLE al M5Stack CoreS3. Solo lectura: nunca escribe en ECUs.
//
// Con DONGLE_SIMULATION_MODE 1 (Config.h) genera valores plausibles sin
// hardware CAN — permite probar la vinculación BLE antes de montar el dongle.

#include <Arduino.h>
#include "Config.h"
#include "VehicleData.h"
#include "CanManager.h"
#include "ObdReader.h"
#include "BleServer.h"

static CanManager  can;
static ObdReader   obd;
static BleServer   ble;
static VehicleData vehicle;

#if DONGLE_SIMULATION_MODE
// Simulación con deriva suave: más realista que valores aleatorios sueltos
static void simulate(VehicleData* d) {
    static float rpm = 900, speed = 0, temp = 70;

    rpm   = constrain(rpm   + random(-300, 320), 800.0f, 4500.0f);
    speed = constrain(speed + random(-8, 9),     0.0f,   130.0f);
    temp  = constrain(temp  + random(-1, 2),     70.0f,  105.0f);

    d->rpm            = (int)rpm;
    d->speed          = (int)speed;
    d->coolantTemp    = (int)temp;
    d->batteryVoltage = 13.8f + random(-3, 4) / 10.0f;
    d->milOn          = random(0, 100) >= 98;  // 2%: en sim los errores son raros
    d->valid          = true;
}
#endif

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n[Dongle] VirtualFriend OBD2 — arrancando");

#if DONGLE_SIMULATION_MODE
    Serial.println("[Dongle] MODO SIMULACION (sin CAN)");
#else
    if (!can.begin()) {
        Serial.println("[Dongle] CAN no disponible — se reintentara en loop");
    }
    obd.begin(&can);
#endif

    ble.begin();
}

void loop() {
    static uint32_t lastPollMs = 0;
    uint32_t now = millis();
    if (now - lastPollMs < POLL_INTERVAL_MS) {
        delay(10);
        return;
    }
    lastPollMs = now;

#if DONGLE_SIMULATION_MODE
    simulate(&vehicle);
#else
    if (!can.isReady()) {
        // Reintento de init: el coche puede dar contacto después de arrancar el dongle
        static uint32_t lastRetryMs = 0;
        if (now - lastRetryMs >= 5000) {
            lastRetryMs = now;
            if (can.begin()) obd.begin(&can);
        }
        vehicle.valid = false;
    } else {
        obd.readAll(&vehicle);
    }
#endif

    ble.notify(vehicle);

    char dbg[64];
    vehicle.toPayload(dbg, sizeof(dbg));
    Serial.printf("[Dongle] %s%s%s\n", dbg,
                  vehicle.valid ? "" : "  (sin datos)",
                  ble.hasClient() ? "  [BLE->M5]" : "");
}
