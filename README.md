# VirtualFriend

Copiloto virtual fisico para coche, basado en **M5Stack CoreS3** (ESP32-S3).
Cara animada en pantalla, voz sarcastica generada por IA, reacciona al
movimiento del vehiculo (IMU) y al toque del usuario.

## Hardware

- **M5Stack CoreS3** — pantalla tactil 320x240, altavoz AW88298, dos micros
  ES7210, microSD, IMU BMI270, bateria, WiFi, BLE.
- **Dongle OBD2** (`dongle_obd2/`) — ESP32-C3 + transceptor CAN SN65HVD230.
  Lee RPM, velocidad, temperatura, voltaje y testigo de averia del coche
  (solo lectura, PIDs estandar) y los envia por BLE al CoreS3.

## Arquitectura

```
[IMU/Touch] -> CtrlTask -> AnimTask  (cara animada en SD)
                       \-> AiTask   -> Cloudflare Worker
                                           |-> OpenAI gpt-4o-mini / Whisper
                                           \-> ElevenLabs TTS
[Touch]    -> MicTask   -> graba 3s WAV    -> AiTask -> Worker /listen

[Coche OBD2] -> dongle (ESP32-C3 + SN65HVD230, TWAI 500kbps)
                  \-> BLE notify "rpm=..;speed=..;coolant=..;volt=..;mil=.."
                        \-> BleTask (CoreS3) -> OBDManager -> reacciones
```

### Modos de simulacion

- `src/Config.h: OBD_SIMULATION_MODE` — 1: el CoreS3 inventa los datos
  (sin dongle). 0: los recibe del dongle por BLE.
- `dongle_obd2/src/Config.h: DONGLE_SIMULATION_MODE` — 1: el dongle envia
  datos plausibles sin CAN (permite probar la vinculacion BLE sin coche).
  0: lectura real del bus CAN.

### Tareas FreeRTOS

| Tarea     | Nucleo | Prio | Stack  | Funcion                                   |
|-----------|--------|------|--------|-------------------------------------------|
| AnimTask  | 0      | 2    | 20 KB  | reproduce GIF idle, eventos y expresiones |
| AiTask    | 0      | 1    | 28 KB  | HTTP POST al Worker, TTS                  |
| MicTask   | 1      | 1    | 12 KB  | grabacion I2S, genera WAV                 |
| CtrlTask  | 1      | 1    | 8 KB   | IMU, touch, dispatch de eventos           |

Sincronizacion: `g_audioMutex` serializa el acceso al I2S (mic/speaker
comparten bus). `g_micActive` y `g_aiThinking` activan los overlays en
pantalla.

## Estructura

```
src/
  main.cpp              entrypoint, setup y CtrlTask
  Config.h              umbrales IMU, credenciales WiFi/Worker
  AnimationPlayer.*     reproduccion de GIFs + fallback vectorial
  FaceRenderer.*        cara vectorial cuando no hay SD
  ExpressionManager.*   parametros de expresion (ojos, boca, color)
  SensorMonitor.*       IMU: detecta acelerada, frenada, bache
  MicManager.*          grabacion 3s WAV @ 16kHz
  SpeechManager.*       decode MP3 (minimp3) + reproduccion
  AiManager.*           cliente HTTP del Worker, gestiona expresiones IA
  SoundManager.*        beeps de eventos y alertas
  minimp3.h             header-only MP3 decoder
```

## Compilar y subir

```sh
pio run                       # compilar
pio run --target upload       # compilar + subir al M5Stack
pio device monitor            # ver logs por serie
```

El puerto USB suele estar en `/dev/ttyACM0` o `/dev/ttyACM1`.
Ajusta `upload_port` / `monitor_port` en `platformio.ini` si cambia.

## Endpoints del Worker

- `POST /` — evento de sensor (HARD_ACCEL, HARD_BRAKE, BUMP).
- `POST /listen` — audio WAV del micro -> Whisper -> respuesta IA.

Ambos devuelven `audio/mpeg` (MP3 22050Hz) con headers `X-VF-Expression`
y `X-VF-Sound`.
