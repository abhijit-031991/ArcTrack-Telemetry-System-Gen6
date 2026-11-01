#include "LoRa.h"

LoRaRadio::LoRaRadio() : _freq(0), _rssi(0), _snr(0) {}

bool LoRaRadio::begin(long frequency, int csPin, int resetPin, int irqPin) {
    _freq = frequency;
    // TODO: configure SPI and initialise LoRa module (e.g. SX1276/SX1278)
    return true;
}

bool LoRaRadio::send(const uint8_t* data, size_t len) {
    // TODO: transmit packet
    return true;
}

int LoRaRadio::receive(uint8_t* buf, size_t maxLen) {
    // TODO: check for incoming packet, copy into buf
    return -1;
}

int   LoRaRadio::rssi()  const { return _rssi; }
float LoRaRadio::snr()   const { return _snr; }

void LoRaRadio::setSyncWord(uint8_t sw)       { /* TODO */ }
void LoRaRadio::setSpreadingFactor(uint8_t sf) { /* TODO */ }
void LoRaRadio::setTxPower(int dBm)           { /* TODO */ }
