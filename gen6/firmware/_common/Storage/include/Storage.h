#pragma once
#include <Arduino.h>

class Storage {
public:
    Storage();
    bool begin(int csPin = -1);   // -1 = use default SPI CS

    bool     writeLog(const char* filename, const char* line);
    bool     appendCSV(const char* filename, const char* row);
    bool     readLine(const char* filename, uint32_t lineNum, char* buf, size_t bufLen);
    bool     exists(const char* filename);
    uint32_t freeKB() const;
    void     flush();

private:
    int  _csPin;
    bool _ready;
};
