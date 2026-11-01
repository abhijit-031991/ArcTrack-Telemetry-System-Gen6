#include <Arduino.h>
#include <GPS.h>
#include <Power.h>
#include <Comms.h>

GPS   gps;
Power power;

const uint32_t DEVICE_ID      = 0x00000005;  // set unique ID per unit
const uint32_t TX_INTERVAL_MS = 90000;        // Argos: 90 s between transmissions

// TODO: add your Argos module library (e.g. SparkFun ARTIC R2) here

void setup() {
    Serial.begin(115200);
    power.begin(/*batAdc*/34);
    gps.begin(/*rx*/16, /*tx*/17);
    // TODO: initialise Argos module (SPI or UART)
}

void loop() {
    gps.update();

    static uint32_t lastTx = 0;
    if (millis() - lastTx >= TX_INTERVAL_MS && gps.isFixed()) {
        ArcPacket pkt = {};
        pkt.deviceType = DEVICE_TRACKER_ARGOS;
        pkt.deviceId   = DEVICE_ID;
        pkt.timestamp  = millis() / 1000;
        pkt.latitude   = gps.latitude();
        pkt.longitude  = gps.longitude();

        uint8_t buf[128];
        Comms::encode(pkt, buf, sizeof(buf));
        // TODO: transmit via Argos module
        lastTx = millis();
    }

    if (power.isCritical()) power.deepSleep(600e6);  // 10 min sleep on low battery
}
