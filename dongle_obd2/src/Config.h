#pragma once

// --- Modo de funcionamiento ---
// 1 = valores simulados (sin transceptor CAN; permite probar BLE ya)
// 0 = lectura real del bus CAN del coche
#define DONGLE_SIMULATION_MODE 1

// --- CAN / TWAI ---
#define CAN_TX_PIN   5
#define CAN_RX_PIN   4
// 500 kbps es lo habitual en OBD2 (ISO 15765-4). Probar 250 si no responde.
#define CAN_SPEED_KBPS 500

// ID de petición funcional OBD2 y rango de respuestas de las ECUs
#define OBD_REQUEST_ID   0x7DF
#define OBD_RESPONSE_MIN 0x7E8
#define OBD_RESPONSE_MAX 0x7EF

// Timeout de espera de respuesta a una petición de PID (ms)
#define OBD_RESPONSE_TIMEOUT_MS 150

// --- Sondeo ---
// Cada cuánto se piden los PIDs al coche y se notifica por BLE (ms).
// No bajar demasiado: saturar el bus puede provocar avisos en el coche.
#define POLL_INTERVAL_MS 1000

// --- BLE ---
#define BLE_DEVICE_NAME  "VirtualFriend_OBD"
#define BLE_SERVICE_UUID "4fde9d23-1271-4d15-93c0-59b7b8844d08"
#define BLE_DATA_UUID    "0af349d3-94a8-425a-8e46-53ba4149eac3"
