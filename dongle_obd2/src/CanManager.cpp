#include "CanManager.h"
#include "Config.h"
#include <Arduino.h>
#include <driver/twai.h>

bool CanManager::begin() {
    twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NORMAL);
    // Colas amplias: el bus del coche puede tener mucho tráfico ajeno
    g.rx_queue_len = 16;
    g.tx_queue_len = 4;

#if CAN_SPEED_KBPS == 250
    twai_timing_config_t t = TWAI_TIMING_CONFIG_250KBITS();
#else
    twai_timing_config_t t = TWAI_TIMING_CONFIG_500KBITS();
#endif

    // Filtro hardware: solo respuestas OBD2 (0x7E8–0x7EF).
    // Mascara sobre ID estándar de 11 bits desplazado 21 bits.
    twai_filter_config_t f;
    f.acceptance_code = (OBD_RESPONSE_MIN << 21);
    f.acceptance_mask = ~((0x7F8u << 21));   // ignora los 3 bits bajos del ID
    f.single_filter   = true;

    if (twai_driver_install(&g, &t, &f) != ESP_OK) {
        Serial.println("[CAN] Error instalando driver TWAI");
        return false;
    }
    if (twai_start() != ESP_OK) {
        Serial.println("[CAN] Error arrancando TWAI");
        twai_driver_uninstall();
        return false;
    }

    _ready = true;
    Serial.printf("[CAN] TWAI OK — %d kbps, TX=%d RX=%d\n",
                  CAN_SPEED_KBPS, CAN_TX_PIN, CAN_RX_PIN);
    return true;
}

void CanManager::end() {
    if (!_ready) return;
    twai_stop();
    twai_driver_uninstall();
    _ready = false;
}

bool CanManager::send(const CanFrame& frame) {
    if (!_ready) return false;

    twai_message_t msg = {};
    msg.identifier       = frame.id;
    msg.data_length_code = frame.len;
    memcpy(msg.data, frame.data, frame.len);

    return twai_transmit(&msg, pdMS_TO_TICKS(50)) == ESP_OK;
}

bool CanManager::receive(CanFrame* out, uint32_t timeoutMs) {
    if (!_ready) return false;

    twai_message_t msg;
    if (twai_receive(&msg, pdMS_TO_TICKS(timeoutMs)) != ESP_OK) return false;

    out->id  = msg.identifier;
    out->len = msg.data_length_code;
    memcpy(out->data, msg.data, msg.data_length_code);
    return true;
}
