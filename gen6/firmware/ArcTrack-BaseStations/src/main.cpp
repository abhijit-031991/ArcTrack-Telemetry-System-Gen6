#include <Arduino.h>
#include <LoRa.h>
#include <Storage.h>
#include <Comms.h>

LoRaRadio lora;
Storage   storage;

void setup() {
    Serial.begin(115200);
    lora.begin(915E6, /*cs*/5, /*rst*/14, /*irq*/26);
    storage.begin(/*cs*/15);
}

void loop() {
    uint8_t buf[128];
    int len = lora.receive(buf, sizeof(buf));
    if (len > 0) {
        ArcPacket pkt;
        if (Comms::decode(buf, len, pkt)) {
            char row[128];
            snprintf(row, sizeof(row), "%lu,%.6f,%.6f,%d",
                     pkt.timestamp, pkt.latitude, pkt.longitude, lora.rssi());
            storage.appendCSV("/log.csv", row);
        }
    }
}
