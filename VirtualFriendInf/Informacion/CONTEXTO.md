# CONTEXTO DEL PROYECTO — VirtualFriend
> Leer este archivo al inicio de cada sesión para tener contexto completo.
> Actualizar al final de cada sesión con lo que se ha hecho.

---

## QUÉ ES ESTE PROYECTO

**VirtualFriend**: copiloto virtual físico para coche. Un pequeño dispositivo con pantalla que acompaña al conductor, muestra una cara animada, tiene personalidad humorística/sarcástica, reacciona al movimiento del vehículo y, en fases avanzadas, lee datos reales del coche por OBD2/CAN.

**Carpeta del proyecto**: `~/virtualfriend/`
**Lenguaje**: C++ / Arduino Framework
**Objetivo actual**: prototipo funcional y ampliable, no producto comercial.

---

## HARDWARE

### Dispositivo principal (cara/voz/IA)
- **M5Stack CoreS3 K128**
  - ESP32-S3, pantalla táctil, Wi-Fi, BLE, altavoz, 2 micrófonos, cámara, IMU, magnetómetro, RTC, microSD, batería

### Dongle OBD2 propio (lector del coche)
- **ESP32-C3** — microcontrolador, TWAI/CAN + BLE
- **SN65HVD230** — transceptor CAN (traduce lógica 3.3V ↔ bus CAN del coche)
- **Conversor DC-DC 12V→5V** — alimentación desde OBD2 pin 16
- **Fusible 1A** — protección en el cable +12V
- **Cable OBD2 macho a cables abiertos**
- **Caja plástica ~100×60×30 mm** (nunca metálica)

### Pines OBD2 usados
| Pin | Señal | Descripción |
|-----|-------|-------------|
| 4/5 | GND   | Masa |
| 6   | CAN-H | Bus CAN High |
| 14  | CAN-L | Bus CAN Low |
| 16  | +12V  | Alimentación directa batería |

---

## ARQUITECTURA DEL SISTEMA

```
Coche → Puerto OBD2
  ↓ cable físico
Dongle OBD2 (ESP32-C3 + SN65HVD230)
  ↓ BLE (datos limpios, no tramas crudas)
M5Stack CoreS3
  → cara, voz, frases, IA, reacciones
```

**Datos que lee el dongle (fase inicial)**:
- RPM: PID 0x0C → `((A*256)+B)/4`
- Velocidad: PID 0x0D → `A` km/h
- Temperatura refrigerante: PID 0x05 → `A-40` °C
- Voltaje: PID 0x42 → `((A*256)+B)/1000` V

**Velocidad CAN**: 500 kbps (probar 250 si no responde)
**ID petición OBD2**: 0x7DF | **ID respuesta**: 0x7E8–0x7EA

**Formato BLE enviado por dongle**:
```
rpm=1850;speed=72;coolant=91   (texto simple para prototipo)
```
o JSON en fases avanzadas.

---

## PERSONALIDAD DEL COPILOTO

- Tono: humorístico, sarcástico, carismático, útil
- Se pone serio ante avisos importantes
- Ejemplos:
  - Aceleración fuerte → "Tranquilo, Fernando Alonso, que esto no es Nürburgring."
  - Temperatura alta → "Ojo. El motor se está calentando más de la cuenta."
  - Bache → "Ese bache no era parte del trayecto, era contenido descargable."

---

## FASES DE DESARROLLO

| Fase | Estado | Objetivo |
|------|--------|----------|
| 1 | Pendiente | M5Stack: cara animada, expresiones, frases, sonido, IMU. Sin OBD2. |
| 2 | Pendiente | Dongle: ESP32-C3 + SN65HVD230, leer RPM por Serial Monitor. Sin BLE. |
| 3 | Pendiente | Dongle: leer RPM + velocidad + temperatura + voltaje |
| 4 | Pendiente | BLE: dongle envía datos, M5Stack recibe |
| 5 | Pendiente | Integración: datos del coche cambian expresiones y activan frases |
| 6 | Pendiente | Caja física, montaje final, pruebas reales en coche |

---

## ESTRUCTURA DE CÓDIGO PREVISTA

```
virtualfriend/
├── dongle_obd2/
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Config.h          ← pines CAN_TX/RX, velocidad CAN, nombre BLE
│   │   ├── VehicleData.h     ← struct {rpm, speed, coolantTemp, batteryVoltage, valid}
│   │   ├── CanManager.h/cpp  ← init TWAI, send/receive tramas
│   │   ├── ObdReader.h/cpp   ← pedir PIDs, convertir bytes a valores
│   │   └── BleServer.h/cpp   ← servidor BLE GATT, notificaciones
│   └── platformio.ini
└── m5stack_copilot/
    └── src/
        ├── main.cpp
        ├── FaceRenderer.h/cpp
        ├── ExpressionManager.h/cpp
        ├── PhraseEngine.h/cpp
        ├── SoundManager.h/cpp
        ├── SensorMonitor.h/cpp   ← IMU para baches/frenazos
        └── BleClient.h/cpp       ← recibir datos del dongle
```

**Config.h del dongle (orientativo)**:
```cpp
#define CAN_TX_PIN 5
#define CAN_RX_PIN 4
#define CAN_SPEED  500000
#define BLE_DEVICE_NAME "VirtualFriend_OBD"
```

---

## LIBRERÍAS PREVISTAS

| Dispositivo | Librería |
|-------------|----------|
| ESP32-C3 dongle | ESP-IDF TWAI, Arduino BLE |
| Datos OBD2 complejos (VIN, DTCs largos) | TWAI_ISO-TP |
| M5Stack CoreS3 | M5Unified, BLE client |

---

## REGLAS DE SEGURIDAD (CRÍTICAS)

- Solo lectura de PIDs OBD2 estándar
- NUNCA escribir en ECUs, borrar DTCs, controlar ABS/airbag/DSG/inmovilizador
- NUNCA comandos propietarios del fabricante
- Siempre fusible 1A en +12V
- Medir con multímetro antes de conectar al coche (salida DC-DC debe ser 5.0V ±0.2V)
- Caja plástica, no metálica
- Si el coche muestra aviso → desconectar inmediatamente
- Si algo se calienta → desconectar inmediatamente

---

## LO QUE SE HA HECHO HASTA AHORA

### Sesión 1 (2026-06-02/03)
- [x] Leído y analizado `ConfeccionOBD2.txt` con el manual técnico del dongle
- [x] Generado PDF visual completo: `/home/rich/Escritorio/Proyecto/Manual_OBD2_Dongle.pdf`
  - Portada, índice, 12 capítulos, diagramas dibujados con reportlab:
    - Conector OBD2 con pines coloreados
    - Diagrama de bloques del sistema
    - Esquema de cableado completo estilo PCB
    - Estructura de trama CAN/OBD2
  - Tablas de pines, PIDs, conexiones, velocidades CAN
  - Pasos de montaje numerados, protocolo de pruebas, troubleshooting
- [x] Creado este archivo CONTEXTO.md

### Sesión 2 (2026-06-03)
- [x] Generado PDF de soldadura: `/home/rich/Escritorio/Proyecto/Manual_Soldadura_OBD2.pdf`
  - 12 imágenes reales generadas con IA (GPT-Image-2) de todos los componentes
  - Pinouts dibujados: ESP32-C3, SN65HVD230, DC-DC
  - Código de colores de cables detallado
  - Técnica de soldadura paso a paso con fotos
  - 9 pasos de montaje: fusible → DC-DC → ESP32 → SN65 → CAN → GND → verificación → caja
  - Tabla de verificaciones con multímetro
  - Resumen visual de todas las conexiones
- [x] Actualizado CONTEXTO.md

### Sesión 3 (2026-06-03)
- [x] Revisado `DiagramaDeCableado.png` original — errores detectados: CAN-H/CAN-L iban al DC-DC en vez del SN65, SN65 debajo del DC-DC con líneas confusas, diagrama recortado a la derecha (sin M5Stack), labels solapados
- [x] Generadas versiones corregidas del diagrama:
  - v2: `/home/rich/Escritorio/Proyecto/DiagramaDeCableado_v2.png` — layout con M5Stack, rutas CAN correctas (solapamientos parciales)
  - v3–v5: iteraciones con diseño de dos zonas (alimentación/CAN), calles Y exclusivas por señal, layout sin solapamientos
  - **Final**: `/home/rich/Escritorio/Proyecto/DiagramaDeCableado_v5.png` — aprobado, limpio
    - Canvas 1920×980, zona azul (alimentación) + zona verde (CAN/señales)
    - DC-DC y SN65HVD230 apilados (mismo X, diferente Y con hueco visual)
    - Cada señal en su propia calle Y: +12V=160, +5V=240, GND=340, 3V3=410, CAN-H=560, CAN-L=640, TX=720, RX=800, BLE=900
    - Leyenda completa + caja de puntos críticos en esquina derecha
- [x] Scripts generadores en `/tmp/gen_diagrama_v3.py` a `v5.py`
- [x] Revisado y rehecho `DiagramaTranceptorCan.png`:
  - Original: labels solapados, "RXDR" en vez de RXD, texto cortado, mal contraste
  - **Final**: `/home/rich/Escritorio/Proyecto/DiagramaTranceptorCan_v2.png`
    - Script en `/tmp/gen_diagrama_transceptor.py`
    - Foto real del módulo SN65HVD230 (AI-generada)
    - Bloque ESP32 (izq) + SN65 (centro, LÓGICA/BUS CAN) + OBD2 (der)
    - Cables con colores, flechas de dirección y pill labels
    - Nota sobre resistencia terminación 120Ω y cómo comprobarla

### Sesión 4 (2026-06-03)
- [x] Cloudflare Worker desplegado y funcional
  - Worker en `~/Proyectos/virtualfriend-worker/`
  - URL pública: `https://virtualfriend-worker.rykhjob.workers.dev`
  - Secrets en Cloudflare: `GEMINI_API_KEY` y `DEVICE_TOKEN`
  - DEVICE_TOKEN: `(ver src/Secrets.h — no publicar)`
  - IA: Gemini 2.5 Flash con thinking desactivado (`thinkingBudget: 0`)
  - Tests pasando (6/6)
  - Documentación en `Escritorio/Proyecto/cloudflare/CLOUDFLARE_WORKER.md`
- [x] Problema detectado y resuelto: Gemini 2.5 Flash consumía todos los tokens pensando → se desactivó el modo thinking
- [x] Problema TLS en CachyOS: TLS 1.3 + ECH falla en este equipo con curl. Usar `--tlsv1.2 --tls-max 1.2` para pruebas locales. El M5Stack no tendrá este problema.

### Sesiones 5-7 (2026-06-03/04) — Implementación Fase 1 completa
- [x] Estructura `virtualfriend/m5stack_copilot/` con PlatformIO, env `m5stack-cores3`
- [x] **FaceRenderer**: cara vectorial (ojos+boca) con 8 formas de ojo y 7 de boca; modo de fallback cuando no hay SD
- [x] **ExpressionManager**: 7 expresiones (NEUTRAL/HAPPY/SARCASTIC/SURPRISED/ALERT/SLEEPING/ANGRY) con FaceParams
- [x] **SensorMonitor**: lectura IMU BMI270 a 25Hz; detección de HARD_ACCEL/HARD_BRAKE/BUMP con umbrales calibrados + cooldown 3s
- [x] **AnimationPlayer**: reproducción de GIFs desde SD con AnimatedGIF; sprite M5Canvas 320×240 en PSRAM; bucle interrumpible; pickPath para variantes aleatorias
- [x] **SoundManager**: 5 efectos (STARTUP/BUMP/ACCEL/BRAKE/ALERT) con mutex trylock para no romper I2S
- [x] **MicManager**: grabación 3s WAV @16kHz mono en PSRAM; mutex con timeout 20s; retry de record(); indicador "HABLA AHORA" inmediato al tocar; visibilidad mínima 2000ms
- [x] **SpeechManager**: decode MP3 con minimp3 (header-only); reproducción via M5.Speaker.playRaw; espera 600ms post-audio dentro del mutex (evita eco→Whisper)
- [x] **AiManager**: cliente HTTPS al Cloudflare Worker; dos endpoints (`/` para eventos, `/listen` para audio); flag `g_aiThinking` con overlay "PENSANDO..."; beep de alerta en errores; expresiones IA `X-VF-Expression` → AnimEvent → GIF o vector fallback
- [x] Compilación OK: 22.6% RAM, 17% Flash. Subido al M5Stack.

### Migración a proyecto VS Code (2026-06-04)
- [x] Movido todo el código desde `~/virtualfriend/m5stack_copilot/` a `~/VirtualFriend/` (proyecto Git original)
- [x] Reemplazado `platformio.ini` Wokwi/esp32dev por `m5stack-cores3`
- [x] Eliminados `wokwi.toml` y `diagram.json` (CoreS3 no soportado por Wokwi)
- [x] Eliminado `PhraseEngine.h/cpp` (la IA genera las frases)
- [x] README.md actualizado con arquitectura, tareas FreeRTOS y flujo
- [x] Commit `9570856` en main: "Fase 1: M5Stack CoreS3 firmware completo"

### Próxima sesión (Fase 2: dongle OBD2)
- [ ] Crear `dongle_obd2/` con PlatformIO env ESP32-C3
- [ ] CanManager: init TWAI a 500kbps, send/receive tramas
- [ ] ObdReader: PIDs 0x0C/0x0D/0x05/0x42, conversión bytes→valores
- [ ] Probar con coche real (lectura sin BLE primero)

---

## NOTAS TÉCNICAS IMPORTANTES

- El SN65HVD230 puede tener resistencia de terminación CAN de 120Ω fija → puede interferir con el bus del coche → revisar si hay problemas de comunicación
- Alimentar ESP32-C3 por USB y VIN/5V simultáneamente puede ser problemático según la placa → en pruebas, usar solo una fuente
- No confiar en colores del cable OBD2 → identificar pines por posición física
- Pin 1 del OBD2 = arriba izquierda, pin 16 = abajo derecha
- Frecuencia de consultas OBD2 no debe ser excesiva → puede saturar el bus o provocar avisos en el coche

---
*Actualizado: 2026-06-04 (Fase 1 completa, código en ~/VirtualFriend/)*
