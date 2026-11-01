#include <Arduino.h>
#include <LoRa.h>
#include <Comms.h>

LoRaRadio lora;

void setup() {
    Serial.begin(115200);
    lora.begin(915E6, /*cs*/5, /*rst*/14, /*irq*/26);
    lora.setSpreadingFactor(9);
    Serial.println("ArcTrack Yagi ready — scanning for trackers...");
}

void loop() {
    uint8_t buf[128];
    int len = lora.receive(buf, sizeof(buf));
    if (len > 0) {
        ArcPacket pkt;
        if (Comms::decode(buf, len, pkt)) {
            // Print bearing data for direction-finding
            Serial.printf("ID:%08X  Lat:%.6f  Lon:%.6f  RSSI:%d dBm  SNR:%.1f\n",
                          pkt.deviceId, pkt.latitude, pkt.longitude,
                          lora.rssi(), lora.snr());
        }
    }
}
