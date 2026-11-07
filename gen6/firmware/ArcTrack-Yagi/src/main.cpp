#include <Arduino.h>
#include <RadioLib.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <definitions.h>
#include <codes.h>

// ── BLE includes ──────────────────────────────────────────────────────────────
#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_ESP32S3)
  #include <NimBLEDevice.h>
#elif defined(TARGET_NRF_BLUEFRUIT)
  #include <bluefruit.h>
#endif

// ── LoRa — Seeed Wio-SX1262 for XIAO (pin aliases consistent across XIAO BSPs)
#define LORA_NSS   D4
#define LORA_DIO1  D1
#define LORA_NRST  D2
#define LORA_BUSY  D3
#define LORA_RXEN  D5

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);

volatile bool packetReceived        = false;
unsigned long lastBatteryCheckTime  = 0;
const unsigned long BATTERY_INTERVAL_MS = 60000;

void setRxFlag() { packetReceived = true; }

// ── NimBLE globals ────────────────────────────────────────────────────────────
#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_ESP32S3)
static NimBLEServer*         _pServer  = nullptr;
static NimBLECharacteristic* _pingChar = nullptr;
static NimBLECharacteristic* _dataChar = nullptr;
static NimBLECharacteristic* _metaChar = nullptr;
#endif

// ── Bluefruit globals ─────────────────────────────────────────────────────────
#ifdef TARGET_NRF_BLUEFRUIT
BLEService        dataService("00000000-0000-0000-0000-00000000189a");
BLECharacteristic pingCharacteristic("c4850de5-2ca0-464b-8e4a-ae45ad4460b7");
BLECharacteristic dataCharacteristic("9e150970-35ad-400c-b46d-08ed71f07709");
BLECharacteristic metaData("a0fa056d-716d-42cd-bfd6-f48c72e2cbe6");
#endif

// ── BLE helpers — isolate stack-specific calls from application logic ─────────
bool bleConnected() {
#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_ESP32S3)
    return _pServer && _pServer->getConnectedCount() > 0;
#elif defined(TARGET_NRF_BLUEFRUIT)
    return Bluefruit.connected();
#endif
}

void notifyPing(const char* json, size_t len) {
#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_ESP32S3)
    if (_pingChar) { _pingChar->setValue((uint8_t*)json, len); _pingChar->notify(); }
#elif defined(TARGET_NRF_BLUEFRUIT)
    pingCharacteristic.notify((uint8_t*)json, (uint16_t)len);
#endif
}

void notifyData(const char* json, size_t len) {
#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_ESP32S3)
    if (_dataChar) { _dataChar->setValue((uint8_t*)json, len); _dataChar->notify(); }
#elif defined(TARGET_NRF_BLUEFRUIT)
    dataCharacteristic.notify((uint8_t*)json, (uint16_t)len);
#endif
}

void notifyMeta(const char* json, size_t len) {
#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_ESP32S3)
    if (_metaChar) { _metaChar->setValue((uint8_t*)json, len); _metaChar->notify(); }
#elif defined(TARGET_NRF_BLUEFRUIT)
    metaData.notify((uint8_t*)json, (uint16_t)len);
#endif
}

// ── Debug helper ──────────────────────────────────────────────────────────────
void dumpFrame(const char* label, const uint8_t* buf, size_t len) {
    Serial.print(label);
    Serial.print(F(" ("));
    Serial.print(len);
    Serial.print(F(" bytes): "));
    for (size_t i = 0; i < len; i++) {
        if (buf[i] < 0x10) Serial.print('0');
        Serial.print(buf[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
}

// ── BLE write handler — shared logic, called from both NimBLE and Bluefruit ───
void handleBLEWrite(const uint8_t* data_ptr, uint16_t len) {
    char setIn[128] = {0};
    uint16_t copyLen = len < sizeof(setIn) - 1 ? len : sizeof(setIn) - 1;
    memcpy(setIn, data_ptr, copyLen);

    Serial.print("BLE Write Received: ");
    Serial.println(setIn);

    StaticJsonDocument<192> doc;
    if (deserializeJson(doc, setIn)) return;

    if (doc.containsKey("ID") && doc.containsKey("Msg")) {
        reqPing r;
        r.tag     = doc["ID"];
        r.request = (byte)doc["Msg"];
        Serial.print(F("Transmitting LoRa Ping for Tag ID: "));
        Serial.println(r.tag);
        dumpFrame("TX reqPing", (uint8_t*)&r, sizeof(r));
        radio.standby();
        radio.transmit((uint8_t*)&r, sizeof(r));
        radio.startReceive();
    }

    if (doc.containsKey("ID") && doc.containsKey("gfrq")) {
        settings s;
        s.tag        = doc["ID"];
        s.gpsFrq     = doc["gfrq"];
        s.gpsTout    = doc["gtout"];
        s.hdop       = doc["hdop"];
        s.radioFrq   = doc["rfrq"];
        s.startHour  = doc["starth"];
        s.endHour    = doc["endh"];
        s.scheduled  = doc["sched"];
        s.accSamples = doc["smpl"];
        Serial.println(F("Transmitting LoRa Settings Payload"));
        dumpFrame("TX settings", (uint8_t*)&s, sizeof(s));
        radio.standby();
        radio.transmit((uint8_t*)&s, sizeof(s));
        radio.startReceive();
    }
}

// ── NimBLE write callback ─────────────────────────────────────────────────────
#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_ESP32S3)
class PingWriteCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        std::string val = pChar->getValue();
        handleBLEWrite((const uint8_t*)val.data(), (uint16_t)val.length());
    }
};
#endif

// ── Bluefruit write callback ──────────────────────────────────────────────────
#ifdef TARGET_NRF_BLUEFRUIT
void writeCallback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data_ptr, uint16_t len) {
    handleBLEWrite(data_ptr, len);
}
#endif

// ── Battery reading ───────────────────────────────────────────────────────────
float readBatteryVoltage() {
#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_NRF_BLUEFRUIT)
    // XIAO nRF52840: 1MΩ + 510kΩ voltage divider, 3.6V internal reference
    pinMode(VBAT_ENABLE, OUTPUT);
    digitalWrite(VBAT_ENABLE, LOW);
    analogReference(AR_INTERNAL);
    analogReadResolution(12);
    delayMicroseconds(50);
    uint32_t adcRaw = 0;
    for (int i = 0; i < 8; i++) { adcRaw += analogRead(PIN_VBAT); delay(2); }
    adcRaw /= 8;
    digitalWrite(VBAT_ENABLE, HIGH);
    pinMode(VBAT_ENABLE, INPUT);
    return ((float)adcRaw / 4095.0f) * 3.6f * 2.96078f;
#elif defined(TARGET_ESP32S3)
    // XIAO ESP32S3 has no built-in battery ADC on the base board.
    // Implement here when a battery monitor circuit is present.
    return 0.0f;
#endif
}

// ── BLE setup ─────────────────────────────────────────────────────────────────
#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_ESP32S3)
void setupBLE() {
    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::setPower(9);

    _pServer = NimBLEDevice::createServer();

    NimBLEService* pService = _pServer->createService("00000000-0000-0000-0000-00000000189a");

    _pingChar = pService->createCharacteristic(
        "c4850de5-2ca0-464b-8e4a-ae45ad4460b7",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    _pingChar->setCallbacks(new PingWriteCallback());

    _dataChar = pService->createCharacteristic(
        "9e150970-35ad-400c-b46d-08ed71f07709",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );

    _metaChar = pService->createCharacteristic(
        "a0fa056d-716d-42cd-bfd6-f48c72e2cbe6",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );

    pService->start();

    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID("00000000-0000-0000-0000-00000000189a");
    pAdv->setScanResponse(true);
    NimBLEDevice::startAdvertising();

    Serial.println("NimBLE configured");
}

#elif defined(TARGET_NRF_BLUEFRUIT)
void setupBLE() {
    Bluefruit.configPrphConn(247, BLE_GAP_EVENT_LENGTH_DEFAULT, 4, 4);
    Bluefruit.begin();
    Bluefruit.setTxPower(4);
    Bluefruit.setName(DEVICE_NAME);

    dataService.begin();

    pingCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE | CHR_PROPS_NOTIFY);
    pingCharacteristic.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    pingCharacteristic.setMaxLen(244);
    pingCharacteristic.setWriteCallback(writeCallback);
    pingCharacteristic.begin();

    dataCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE | CHR_PROPS_NOTIFY);
    dataCharacteristic.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    dataCharacteristic.setMaxLen(244);
    dataCharacteristic.begin();

    metaData.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE | CHR_PROPS_NOTIFY);
    metaData.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    metaData.setMaxLen(244);
    metaData.begin();

    Bluefruit.Advertising.clearData();
    Bluefruit.ScanResponse.clearData();
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addService(dataService);
    Bluefruit.ScanResponse.addName();
    Bluefruit.ScanResponse.addTxPower();
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244);
    Bluefruit.Advertising.start(0);

    Serial.println("BLE Configured with 247 MTU Capacity");
}
#endif

// ── setup ─────────────────────────────────────────────────────────────────────
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(LORA_RXEN, OUTPUT);
    digitalWrite(LORA_RXEN, HIGH);

    Serial.begin(115200);
    uint32_t startTime = millis();
    while (!Serial && (millis() - startTime < 3000));

    Serial.print(F("Initializing SX1262... "));
    int state = radio.begin(867.0, 125.0, 12, 7, 0x12, 10, 8, 1.8, false);

    if (state == RADIOLIB_ERR_NONE) {
        radio.setDio2AsRfSwitch(true);
        radio.setDio1Action(setRxFlag);
        radio.startReceive();
        Serial.println(F("SX1262 Initialized Successfully (Code 0)!"));
    } else {
        Serial.print(F("SX1262 Init Failed, code: "));
        Serial.println(state);
    }

    setupBLE();
}

// ── processLoRaPacket ─────────────────────────────────────────────────────────
void processLoRaPacket() {
    // DIO1 fires on both RxDone and TxDone. Guard against TxDone false triggers.
    uint32_t irqFlags = radio.getIrqFlags();
    if (!(irqFlags & (1UL << RADIOLIB_IRQ_RX_DONE))) {
        radio.clearIrqFlags(1UL << RADIOLIB_IRQ_TX_DONE);
        radio.startReceive();
        return;
    }

    size_t len  = radio.getPacketLength();
    float  rssi = radio.getRSSI();
    Serial.println(len);

    if (len == sizeof(resPing)) {
        resPing r;
        radio.readData((uint8_t*)&r, len);
        Serial.write((uint8_t*)&r, len);
        StaticJsonDocument<128> doc;
        doc[F("ID")]   = r.tag;
        doc[F("Msg")]  = r.resp;
        doc[F("RSSI")] = rssi;
        char dat[128]; serializeJson(doc, dat);
        if (r.resp == DATA_DOWNLOAD_BEGIN || r.resp == DATA_DOWNLOAD_END || r.resp == DATA_DOWNLOAD_ERROR)
            notifyData(dat, strlen(dat));
        else
            notifyPing(dat, strlen(dat));
        Serial.println(dat);

    } else if (len == sizeof(longPing)) {
        longPing p;
        radio.readData((uint8_t*)&p, len);
        Serial.write((uint8_t*)&p, len);
        StaticJsonDocument<256> doc;
        doc[F("ID")]   = p.ta;
        doc[F("Lat")]  = String(p.la, 6);
        doc[F("Lng")]  = String(p.ln, 6);
        doc[F("DTyp")] = p.devtyp;
        doc[F("cnt")]  = p.cnt;
        doc[F("RSSI")] = rssi;
        doc[F("Mort")] = p.mortality;
        char dat[128]; serializeJson(doc, dat);
        notifyPing(dat, strlen(dat));
        Serial.println(dat);

    } else if (len == sizeof(lngPngid)) {
        lngPngid p;
        radio.readData((uint8_t*)&p, len);
        Serial.write((uint8_t*)&p, len);
        StaticJsonDocument<256> doc;
        doc[F("ID")]   = p.ta;
        doc[F("Lat")]  = String(p.la, 6);
        doc[F("Lng")]  = String(p.ln, 6);
        doc[F("DTyp")] = p.devtyp;
        doc[F("cnt")]  = p.cnt;
        doc[F("RSSI")] = rssi;
        doc[F("Mort")] = p.mortality;
        char dat[128]; serializeJson(doc, dat);
        notifyPing(dat, strlen(dat));
        Serial.println(dat);

    } else if (len == sizeof(data)) {
        data d;
        radio.readData((uint8_t*)&d, len);
        StaticJsonDocument<512> doc;
        doc[F("Date")]  = d.datetime;
        doc[F("Lat")]   = d.lat;
        doc[F("Lng")]   = d.lng;
        doc[F("LckTm")] = d.locktime;
        doc[F("hdop")]  = d.hdop;
        doc[F("ID")]    = d.id;
        doc[F("x")]     = d.x;
        doc[F("y")]     = d.y;
        doc[F("z")]     = d.z;
        doc[F("cnt")]   = d.count;
        char dat[512]; serializeJson(doc, dat);
        notifyData(dat, strlen(dat));
        Serial.println(dat);

    } else if (len == sizeof(calibrationData) || len == 25 || len == 28) {
        calibrationData calData;
        radio.readData((uint8_t*)&calData, len);
        StaticJsonDocument<256> doc;
        doc[F("Lat")]  = String(calData.lat, 6);
        doc[F("Lng")]  = String(calData.lng, 6);
        doc[F("Hdop")] = String(calData.hdop, 1);
        doc[F("Bat")]  = String(calData.bat, 2);
        doc[F("Sig")]  = String(calData.signal, 2);
        doc[F("Mqtt")] = calData.mqtt;
        doc[F("Gprs")] = calData.gprs;
        doc[F("Net")]  = calData.network;
        doc[F("ID")]   = calData.tag;
        char dat[256]; serializeJson(doc, dat);
        notifyPing(dat, strlen(dat));
        Serial.println(dat);

    } else {
        Serial.print(F("Unknown packet length: "));
        Serial.println(len);
        uint8_t buffer[16] = {0};
        size_t bytesToRead = (len >= 16) ? 16 : len;
        int state = radio.readData(buffer, bytesToRead);
        if (state == RADIOLIB_ERR_NONE) {
            Serial.print(F("Raw 16-byte payload (HEX): "));
            for (size_t i = 0; i < bytesToRead; i++) {
                if (buffer[i] < 0x10) Serial.print('0');
                Serial.print(buffer[i], HEX);
                Serial.print(' ');
            }
            Serial.println();
        } else {
            Serial.print(F("Failed to read payload, code: "));
            Serial.println(state);
        }
    }

    radio.startReceive();
}

// ── sendBatteryUpdate ─────────────────────────────────────────────────────────
void sendBatteryUpdate() {
    float vbat = readBatteryVoltage();
    StaticJsonDocument<64> doc;
    doc[F("BAT")] = serialized(String(vbat, 2));
    char dat[64];
    size_t jsonLen = serializeJson(doc, dat);
    if (bleConnected()) {
        notifyMeta(dat, jsonLen);
        Serial.print(F("Battery update sent via BLE: "));
    } else {
        Serial.print(F("Battery reading (No BLE connection): "));
    }
    Serial.println(dat);
}

// ── loop ──────────────────────────────────────────────────────────────────────
void loop() {
    if (packetReceived) {
        packetReceived = false;
        Serial.println("PKT");
        Serial.flush();
        processLoRaPacket();
    }

    unsigned long now = millis();
    if (now - lastBatteryCheckTime >= BATTERY_INTERVAL_MS) {
        lastBatteryCheckTime = now;
        sendBatteryUpdate();
    }

#if defined(TARGET_NRF_NIMBLE) || defined(TARGET_NRF_BLUEFRUIT)
    waitForEvent();
#endif
}
