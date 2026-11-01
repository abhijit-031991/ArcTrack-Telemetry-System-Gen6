#include "Storage.h"

Storage::Storage() : _csPin(-1), _ready(false) {}

bool Storage::begin(int csPin) {
    _csPin = csPin;
    // TODO: SD.begin(csPin) or LittleFS.begin()
    _ready = true;
    return _ready;
}

bool Storage::writeLog(const char* filename, const char* line) {
    if (!_ready) return false;
    // TODO: open file, write line + newline, close
    return true;
}

bool Storage::appendCSV(const char* filename, const char* row) {
    if (!_ready) return false;
    // TODO: open file in append mode, write row + newline, close
    return true;
}

bool Storage::readLine(const char* filename, uint32_t lineNum, char* buf, size_t bufLen) {
    // TODO: seek to lineNum and copy into buf
    return false;
}

bool     Storage::exists(const char* filename) { return false; /* TODO */ }
uint32_t Storage::freeKB()                     { return 0;     /* TODO */ }
void     Storage::flush()                      {               /* TODO */ }
