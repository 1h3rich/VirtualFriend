#pragma once
#include <M5Unified.h>
#include <stdint.h>

enum class SensorEvent {
    NONE,
    HARD_ACCEL,
    HARD_BRAKE,
    BUMP,
};

class SensorMonitor {
public:
    SensorMonitor();
    void begin();
    SensorEvent update();

    float getCorrAX() const { return _cax; }
    float getCorrAY() const { return _cay; }
    float getCorrAZ() const { return _caz; }

private:
    float _ax, _ay, _az;         // raw
    float _cax, _cay, _caz;      // corregidos (offset restado)
    float _prevCay;              // eje vertical anterior (para delta bache)

    uint32_t    _lastEventMs;
    SensorEvent _lastEvent;
    bool        _imuOk;
};
