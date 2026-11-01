#pragma once
#include <Arduino.h>

// BLE roles supported by this library
enum BLERole {
    BLE_ROLE_ADVERTISER, // broadcast data (tracker beaconing its ID)
    BLE_ROLE_SCANNER,    // scan for nearby devices (Yagi direction-finding)
    BLE_ROLE_PERIPHERAL, // connectable GATT server (config / data readout)
};

struct BLEScanResult {
    char    address[18];  // "XX:XX:XX:XX:XX:XX"
    char    name[32];
    int8_t  rssi;
    uint8_t payload[31];
    size_t  payloadLen;
};

class BLE {
public:
    BLE();
    bool begin(const char* deviceName, BLERole role = BLE_ROLE_PERIPHERAL);
    void stop();

    // Advertiser
    void advertise(const uint8_t* data, size_t len);
    void stopAdvertising();

    // Scanner
    void     startScan(uint32_t durationMs = 5000);
    int      scanResultCount() const;
    bool     getScanResult(int index, BLEScanResult& result) const;

    // Peripheral (GATT)
    // Returns characteristic handle; write a UUID string e.g. "6E400002-..."
    int  addCharacteristic(const char* uuid, bool readable, bool writable);
    bool writeCharacteristic(int handle, const uint8_t* data, size_t len);
    bool readCharacteristic(int handle, uint8_t* buf, size_t& len) const;
    bool isConnected() const;

private:
    BLERole _role;
    bool    _connected;
    int     _scanCount;
};
