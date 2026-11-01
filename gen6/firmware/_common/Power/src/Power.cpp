#include "Power.h"

Power::Power() : _adcPin(-1), _voltage(0), _charging(false) {}

bool Power::begin(int batAdcPin) {
    _adcPin = batAdcPin;
    // TODO: configure ADC, charge-detect pin
    return true;
}

float   Power::batteryVoltage() const { return _voltage; }
uint8_t Power::batteryPercent() const {
    // Simple linear map between 3.3 V (0%) and 4.2 V (100%)
    float pct = (_voltage - 3.3f) / (4.2f - 3.3f) * 100.0f;
    if (pct < 0)   pct = 0;
    if (pct > 100) pct = 100;
    return (uint8_t)pct;
}
bool Power::isCharging()  const { return _charging; }
bool Power::isCritical()  const { return _voltage < 3.4f; }

void Power::deepSleep(uint64_t microseconds) {
    // TODO: esp_deep_sleep(microseconds) or platform equivalent
}

void Power::lightSleep(uint64_t microseconds) {
    // TODO: esp_light_sleep_start() after configuring timer wakeup
}

void Power::setWakePin(int pin, int mode) {
    // TODO: configure EXT0/EXT1 wakeup source
}
