#include "SensorMonitor.h"
#include "Config.h"

SensorMonitor::SensorMonitor()
    : _ax(0), _ay(0), _az(0),
      _cax(0), _cay(0), _caz(0), _prevCay(0),
      _lastEventMs(0), _imuOk(false) {}

void SensorMonitor::begin() {
    _imuOk = (M5.Imu.getType() != m5::imu_none);
    Serial.printf("[IMU] %s\n", _imuOk ? "OK" : "no disponible");
}

SensorEvent SensorMonitor::update() {
    if (!_imuOk) return SensorEvent::NONE;

    uint32_t now = millis();
    bool onCooldown = (now - _lastEventMs) < EVENT_COOLDOWN_MS;

    M5.Imu.update();
    if (!M5.Imu.getAccel(&_ax, &_ay, &_az)) return SensorEvent::NONE;

    // Aplicar offsets (la gravedad queda eliminada)
    _cax = _ax - IMU_OFFSET_AX;   // ~0 en reposo
    _cay = _ay - IMU_OFFSET_AY;   // ~0 en reposo (era +1G)
    _caz = _az - IMU_OFFSET_AZ;   // ~0 en reposo

    // Delta en eje vertical para detección de bache
    float dvert = fabsf(_cay - _prevCay);
    _prevCay = _cay;

    if (onCooldown) return SensorEvent::NONE;

    // Bache: sacudida brusca en el eje vertical (Y)
    if (dvert > BUMP_THRESHOLD) {
        _lastEventMs = now;
        return SensorEvent::BUMP;
    }

    // Aceleración/frenada: eje horizontal dominante (X o Z, el de mayor magnitud)
    float domH = (fabsf(_cax) > fabsf(_caz)) ? _cax : _caz;

    if (domH > ACCEL_THRESHOLD) {
        _lastEventMs = now;
        return SensorEvent::HARD_ACCEL;
    }
    if (domH < -BRAKE_THRESHOLD) {
        _lastEventMs = now;
        return SensorEvent::HARD_BRAKE;
    }

    return SensorEvent::NONE;
}
