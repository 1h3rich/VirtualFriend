#pragma once
#include <stdint.h>

class OBDManager {
public:
    OBDManager();
    void begin();
    void update();

    bool isConnected()    const { return _connected; }
    int  getRPM()         const { return _rpm; }
    int  getSpeed()       const { return _speed; }
    int  getEngineTemp()  const { return _engineTemp; }
    bool hasEngineError() const { return _engineError; }

private:
    bool     _connected;
    int      _rpm;
    int      _speed;
    int      _engineTemp;
    bool     _engineError;
    uint32_t _lastSimMs;
};
