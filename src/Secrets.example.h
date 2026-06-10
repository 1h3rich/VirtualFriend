#pragma once
// Plantilla de credenciales. Copiar como Secrets.h y rellenar con valores reales.
// Secrets.h está en .gitignore y nunca debe subirse al repositorio.

// --- WiFi ---
#define WIFI_SSID    "TU_SSID"
#define WIFI_PASS    "TU_PASSWORD"

// --- Cloudflare Worker ---
// Debe coincidir con el secret DEVICE_TOKEN del worker (wrangler secret put DEVICE_TOKEN)
#define WORKER_TOKEN "TU_TOKEN"
