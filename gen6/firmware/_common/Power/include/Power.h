#pragma once
#include <Arduino.h>

class Power {
public:
    Power();
    bool begin(int batAdcPin = -1);

    float   batteryVoltage()    const;
    uint8_t batteryPercent()    const;
    bool    isCharging()        const;
    bool    isCritical()        const;  // below safe threshold

    void    deepSleep(uint64_t microseconds);
    void    lightSleep(uint64_t microseconds);
    void    setWakePin(int pin, int mode = RISING);

private:
    int     _adcPin;
    float   _voltage;
    bool    _charging;
};
