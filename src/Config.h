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
// 1 = valores simulados (mientras no llegue el comunicador CAN)
// 0 = lectura real (activar cuando esté disponible el hardware)
#define OBD_SIMULATION_MODE 1

// Tiempo mínimo entre reacciones a datos OBD (ms)
#define OBD_REACTION_COOLDOWN_MS 8000

// --- Credenciales (WiFi + token del worker) ---
// Definidas en Secrets.h (gitignorado). Plantilla: Secrets.example.h
#include "Secrets.h"

// --- Cloudflare Worker ---
#define WORKER_HOST  "virtualfriend-worker.rykhjob.workers.dev"
