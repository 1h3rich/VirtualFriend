#include <M5Unified.h>
#include "Config.h"
#include "FaceRenderer.h"
#include "ExpressionManager.h"
#include "SoundManager.h"
#include "SensorMonitor.h"
#include "AnimationPlayer.h"
#include "AiManager.h"
#include "MicManager.h"
#include "obd.h"

FaceRenderer      face;
ExpressionManager expr;

SemaphoreHandle_t g_audioMutex = nullptr;
volatile bool     g_micActive  = false;
volatile bool     g_aiThinking = false;

SoundManager           sound;  // global: AiManager también lo usa para alertas
static SensorMonitor   imu;
static OBDManager      obd;
AnimationPlayer        anim;   // global: AiManager le encola expresiones IA
static AiManager       ai;
static MicManager      mic;

// ── Tarea de control (Core 1) ─────────────────────────────────────────────────

void controlTask(void* pvParams) {
    uint32_t lastImuMs      = 0;
    uint32_t lastStackMs    = 0;
    uint32_t lastObdMs      = 0;

    while (true) {
        uint32_t now = millis();

        if (now - lastStackMs >= 10000) {
            lastStackMs = now;
            Serial.printf("[STACK] Ctrl=%u  heap=%u psram=%u\n",
                (unsigned)uxTaskGetStackHighWaterMark(nullptr),
                (unsigned)ESP.getFreeHeap(),
                (unsigned)ESP.getFreePsram());
        }

        if (now - lastImuMs >= IMU_SAMPLE_MS) {
            lastImuMs = now;
            M5.update();

            // ── Toque en pantalla → grabar audio ─────────────────────────────
            {
                static bool s_prevTouch = false;
                bool touched = M5.Touch.getCount() > 0;
                if (touched && !s_prevTouch && !mic.isRecording() && !mic.hasPending()) {
                    Serial.println("[TOUCH] Grabacion iniciada");
                    mic.triggerRecord();
                }
                s_prevTouch = touched;
            }

            // ── Audio grabado listo → enviar a IA ────────────────────────────
            if (mic.hasPending()) {
                uint8_t* buf; size_t len;
                mic.takePending(&buf, &len);
                ai.onMicAudio(buf, len); // AiTask libera el buffer
            }

            // ── IMU → eventos de conducción ───────────────────────────────────
            SensorEvent ev = imu.update();
            switch (ev) {
                case SensorEvent::HARD_ACCEL:
                    sound.play(SoundEffect::ACCEL);
                    anim.triggerEvent(AnimEvent::HARD_ACCEL);
                    ai.onEvent(ev);
                    break;

                case SensorEvent::HARD_BRAKE:
                    sound.play(SoundEffect::BRAKE);
                    anim.triggerEvent(AnimEvent::HARD_BRAKE);
                    ai.onEvent(ev);
                    break;

                case SensorEvent::BUMP:
                    sound.play(SoundEffect::BUMP);
                    anim.triggerEvent(AnimEvent::BUMP);
                    ai.onEvent(ev);
                    break;

                default: break;
            }
        }

        // ── OBD → reacciones al estado del coche ─────────────────────────────
        obd.update();
        if (obd.isConnected() && (now - lastObdMs >= OBD_REACTION_COOLDOWN_MS)) {
            if (obd.hasEngineError()) {
                anim.triggerEvent(AnimEvent::AI_SERIOUS);
                lastObdMs = now;
            } else if (obd.getEngineTemp() > 95) {
                anim.triggerEvent(AnimEvent::AI_SERIOUS);
                lastObdMs = now;
            } else if (obd.getRPM() > 4000) {
                anim.triggerEvent(AnimEvent::AI_SURPRISED);
                lastObdMs = now;
            } else if (obd.getSpeed() > 120) {
                anim.triggerEvent(AnimEvent::AI_SERIOUS);
                lastObdMs = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);

    g_audioMutex = xSemaphoreCreateMutex();

    face.begin();
    sound.begin();
    imu.begin();
    obd.begin();
    anim.begin();
    mic.begin();

    ai.begin(); // conecta WiFi y arranca AiTask

    randomSeed(millis());
    Serial.printf("[VirtualFriend] SD=%s  WiFi=%s\n",
        anim.sdAvailable() ? "OK" : "no",
        ai.isConnected()   ? "OK" : "sin conexion"
    );

    xTaskCreatePinnedToCore(animPlayerTask, "AnimTask", 20480, &anim, 2, nullptr, 0);
    xTaskCreatePinnedToCore(controlTask,   "CtrlTask",  8192,  nullptr, 1, nullptr, 1);
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
