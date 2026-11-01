#include <Arduino.h>
#include <LoRa.h>
#include <BLE.h>
#include <Comms.h>

LoRaRadio lora;
BLE       ble;

void setup() {
    Serial.begin(115200);
    lora.begin(915E6, /*cs*/5, /*rst*/14, /*irq*/26);
    lora.setSpreadingFactor(9);
    ble.begin("ArcTrack-Yagi", BLE_ROLE_SCANNER);
    Serial.println("ArcTrack Yagi ready — scanning for trackers...");
}

void loop() {
    // LoRa — receive ArcPacket from any tracker in range
    uint8_t buf[128];
    int len = lora.receive(buf, sizeof(buf));
    if (len > 0) {
        ArcPacket pkt;
        if (Comms::decode(buf, len, pkt)) {
            Serial.printf("[LoRa] ID:%08X  Lat:%.6f  Lon:%.6f  RSSI:%d dBm  SNR:%.1f\n",
                          pkt.deviceId, pkt.latitude, pkt.longitude,
                          lora.rssi(), lora.snr());
        }
    }

    // BLE — scan for nearby trackers advertising over BLE
    ble.startScan(1000);
    for (int i = 0; i < ble.scanResultCount(); i++) {
        BLEScanResult r;
        if (ble.getScanResult(i, r)) {
            Serial.printf("[BLE]  %s  %-32s  RSSI:%d dBm\n",
                          r.address, r.name, r.rssi);
        }
    }
}
