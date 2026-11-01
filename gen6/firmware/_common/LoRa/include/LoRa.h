#pragma once
#include <Arduino.h>

class LoRaRadio {
public:
    LoRaRadio();
    bool begin(long frequency, int csPin, int resetPin, int irqPin);

    bool send(const uint8_t* data, size_t len);
    int  receive(uint8_t* buf, size_t maxLen); // returns bytes read, -1 if none
    int  rssi() const;
    float snr() const;

    void setSyncWord(uint8_t sw);
    void setSpreadingFactor(uint8_t sf);   // 7–12
    void setTxPower(int dBm);

private:
    long  _freq;
    int   _rssi;
    float _snr;
};
