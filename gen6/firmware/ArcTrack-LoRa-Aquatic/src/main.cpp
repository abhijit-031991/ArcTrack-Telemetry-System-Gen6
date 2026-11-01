#include <Arduino.h>
#include <GPS.h>
#include <LoRa.h>
#include <Power.h>
#include <Comms.h>

GPS       gps;
LoRaRadio lora;
Power     power;

const uint32_t DEVICE_ID  = 0x00000002;  // set unique ID per unit
const uint32_t TX_INTERVAL_MS = 30000;   // aquatic: transmit every 30 s (surface windows)

void setup() {
    Serial.begin(115200);
    power.begin(/*batAdc*/34);
    gps.begin(/*rx*/16, /*tx*/17);
    lora.begin(915E6, /*cs*/5, /*rst*/14, /*irq*/26);
    lora.setSpreadingFactor(10);  // higher SF for water attenuation
    lora.setTxPower(20);
}

void loop() {
    gps.update();

    static uint32_t lastTx = 0;
    if (millis() - lastTx >= TX_INTERVAL_MS && gps.isFixed()) {
        ArcPacket pkt = {};
        pkt.deviceType = DEVICE_TRACKER_LORA;
        pkt.deviceId   = DEVICE_ID;
        pkt.timestamp  = millis() / 1000;
        pkt.latitude   = gps.latitude();
        pkt.longitude  = gps.longitude();

        uint8_t buf[128];
        size_t  len = Comms::encode(pkt, buf, sizeof(buf));
        lora.send(buf, len);
        lastTx = millis();
    }

    if (power.isCritical()) power.deepSleep(300e6);
}
