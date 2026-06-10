#pragma once

// --- Offsets IMU (posición definitiva, medidos 2026-06-03) ---
// Gravedad cae en AY (+1G) → Y es el eje vertical.
// AX y AZ son los ejes horizontales (aceleración/frenada del coche).
#define IMU_OFFSET_AX  -0.0139f
#define IMU_OFFSET_AY   1.0031f
#define IMU_OFFSET_AZ  -0.0141f

// Umbral de aceleración/frenada brusca en eje horizontal dominante (en G)
#define ACCEL_THRESHOLD     0.45f
#define BRAKE_THRESHOLD     0.40f
// Umbral de bache: delta en eje vertical (AY corregido) entre frames a 40ms
#define BUMP_THRESHOLD      0.40f

// Tiempo mínimo entre eventos del mismo tipo (ms)
#define EVENT_COOLDOWN_MS   3000

// Tiempo que se muestra una frase en pantalla (ms)
#define PHRASE_DISPLAY_MS   4000

// Frecuencia de muestreo del IMU (ms entre lecturas)
#define IMU_SAMPLE_MS       40

// Colores de texto de frase
#define COLOR_TEXT          0xFFFFFF
#define COLOR_TEXT_ALERT    0xFF4444

// --- OBD / CAN ---
// 1 = valores simulados internamente (sin dongle)
// 0 = datos reales del dongle por BLE (el dongle puede a su vez estar en
//     simulación — DONGLE_SIMULATION_MODE en dongle_obd2/src/Config.h —
//     lo que permite probar la vinculación BLE sin hardware CAN)
#define OBD_SIMULATION_MODE 0

// Tiempo mínimo entre reacciones a datos OBD (ms)
#define OBD_REACTION_COOLDOWN_MS 8000

// Umbrales de reacción a datos del coche
#define OBD_TEMP_ALERT_C     95    // temperatura refrigerante alta
#define OBD_RPM_ALERT        4000  // motor revolucionado
#define OBD_SPEED_ALERT_KMH  120   // velocidad excesiva

// --- Dongle BLE (máquina de diagnosis) ---
// Deben coincidir con dongle_obd2/src/Config.h
#define OBD_BLE_SERVICE_UUID "4fde9d23-1271-4d15-93c0-59b7b8844d08"
#define OBD_BLE_DATA_UUID    "0af349d3-94a8-425a-8e46-53ba4149eac3"
// Si no llegan notificaciones en este tiempo, se considera sin conexión (ms)
#define OBD_BLE_DATA_TTL_MS  5000

// --- Credenciales (WiFi + token del worker) ---
// Definidas en Secrets.h (gitignorado). Plantilla: Secrets.example.h
#include "Secrets.h"

// --- Cloudflare Worker ---
#define WORKER_HOST  "virtualfriend-worker.rykhjob.workers.dev"
