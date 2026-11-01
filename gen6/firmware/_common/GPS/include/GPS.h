#pragma once
#include <Arduino.h>

class GPS {
public:
    GPS();
    bool begin(int rxPin, int txPin, uint32_t baud = 9600);
    bool update();

    float    latitude()  const;
    float    longitude() const;
    float    altitude()  const;
    float    speed()     const;  // km/h
    uint8_t  satellites() const;
    bool     isFixed()   const;

    // Raw NMEA sentence (last received)
    const char* lastNMEA() const;

private:
    float    _lat, _lon, _alt, _speed;
    uint8_t  _sats;
    bool     _fixed;
    char     _nmea[128];
};
