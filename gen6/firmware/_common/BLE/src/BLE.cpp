#include "BLE.h"

// TODO: replace stub with ESP32 BLE Arduino library calls
//       (BLEDevice, BLEServer, BLEScan from <BLEDevice.h>)

BLE::BLE() : _role(BLE_ROLE_PERIPHERAL), _connected(false), _scanCount(0) {}

bool BLE::begin(const char* deviceName, BLERole role) {
    _role = role;
    // TODO: BLEDevice::init(deviceName)
    return true;
}

void BLE::stop() {
    // TODO: BLEDevice::deinit()
}

void BLE::advertise(const uint8_t* data, size_t len) {
    // TODO: configure BLEAdvertisementData and start advertising
}

void BLE::stopAdvertising() {
    // TODO: BLEDevice::getAdvertising()->stop()
}

void BLE::startScan(uint32_t durationMs) {
    _scanCount = 0;
    // TODO: BLEDevice::getScan()->start(durationMs / 1000)
}

int  BLE::scanResultCount()                              const { return _scanCount; }
bool BLE::getScanResult(int i, BLEScanResult& r)         const { return false; /* TODO */ }
int  BLE::addCharacteristic(const char*, bool, bool)           { return -1;    /* TODO */ }
bool BLE::writeCharacteristic(int, const uint8_t*, size_t)     { return false; /* TODO */ }
bool BLE::readCharacteristic(int, uint8_t*, size_t&)     const { return false; /* TODO */ }
bool BLE::isConnected()                                  const { return _connected; }
