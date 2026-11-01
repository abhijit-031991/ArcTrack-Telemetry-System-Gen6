#include "GPS.h"

GPS::GPS() : _lat(0), _lon(0), _alt(0), _speed(0), _sats(0), _fixed(false) {}

bool GPS::begin(int rxPin, int txPin, uint32_t baud) {
    // TODO: initialise hardware serial / SoftwareSerial on rxPin/txPin
    return true;
}

bool GPS::update() {
    // TODO: read NMEA sentences and parse into _lat, _lon, etc.
    return _fixed;
}

float   GPS::latitude()   const { return _lat; }
float   GPS::longitude()  const { return _lon; }
float   GPS::altitude()   const { return _alt; }
float   GPS::speed()      const { return _speed; }
uint8_t GPS::satellites() const { return _sats; }
bool    GPS::isFixed()    const { return _fixed; }
const char* GPS::lastNMEA() const { return _nmea; }
