#include <Arduino.h>
#include <GPS.h>
#include <Power.h>
#include <Comms.h>

GPS   gps;
Power power;

const uint32_t DEVICE_ID      = 0x00000004;  // set unique ID per unit
const uint32_t TX_INTERVAL_MS = 60000;        // transmit every 60 s

// TODO: add your 4G modem library (e.g. TinyGSM with LTE) here
// #include <TinyGsmClient.h>

void setup() {
    Serial.begin(115200);
    power.begin(/*batAdc*/34);
    gps.begin(/*rx*/16, /*tx*/17);
    // TODO: initialise 4G modem, connect to APN
}

void loop() {
    gps.update();

    static uint32_t lastTx = 0;
    if (millis() - lastTx >= TX_INTERVAL_MS && gps.isFixed()) {
        ArcPacket pkt = {};
        pkt.deviceType = DEVICE_TRACKER_4G;
        pkt.deviceId   = DEVICE_ID;
        pkt.timestamp  = millis() / 1000;
        pkt.latitude   = gps.latitude();
        pkt.longitude  = gps.longitude();

        uint8_t buf[128];
        size_t  len = Comms::encode(pkt, buf, sizeof(buf));
        // TODO: transmit buf over 4G (HTTP POST / MQTT)
        lastTx = millis();
    }

    if (power.isCritical()) power.deepSleep(300e6);
}
