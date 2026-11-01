#include <Arduino.h>
#include <GPS.h>
#include <Power.h>
#include <Storage.h>
#include <Comms.h>

GPS     gps;
Power   power;
Storage storage;

void setup() {
    Serial.begin(115200);
    power.begin(/*batAdc*/34);
    gps.begin(/*rx*/16, /*tx*/17);
    storage.begin(/*cs*/15);
}

void loop() {
    if (gps.update() && gps.isFixed()) {
        char row[128];
        snprintf(row, sizeof(row), "%lu,%.6f,%.6f,%.1f,%.1f%%",
                 millis() / 1000,
                 gps.latitude(), gps.longitude(),
                 gps.altitude(), (float)power.batteryPercent());
        storage.appendCSV("/log.csv", row);
    }

    if (power.isCritical()) {
        storage.flush();
        power.deepSleep(60e6);  // sleep 60 s on low battery
    }

    delay(10000);  // log every 10 s
}
