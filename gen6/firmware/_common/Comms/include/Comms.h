#pragma once
#include <Arduino.h>

// Shared ArcTrack message format used across all devices
// -----------------------------------------------------------
// Packet layout (binary, little-endian):
//   [0]     device_type  (1 B)
//   [1-4]   device_id    (4 B)
//   [5-8]   timestamp    (4 B, Unix epoch)
//   [9-12]  latitude     (4 B, float)
//   [13-16] longitude    (4 B, float)
//   [17-18] payload_len  (2 B)
//   [19..]  payload      (variable)
//   [last2] crc16        (2 B)

enum DeviceType : uint8_t {
    DEVICE_TRACKER_LORA  = 0x01,
    DEVICE_TRACKER_GSM   = 0x02,
    DEVICE_TRACKER_4G    = 0x03,
    DEVICE_TRACKER_ARGOS = 0x04,
    DEVICE_BASE_STATION  = 0x10,
    DEVICE_LOGGER        = 0x20,
    DEVICE_YAGI          = 0x30,
};

struct ArcPacket {
    uint8_t  deviceType;
    uint32_t deviceId;
    uint32_t timestamp;
    float    latitude;
    float    longitude;
    uint16_t payloadLen;
    uint8_t  payload[64];
    uint16_t crc;
};

class Comms {
public:
    static size_t   encode(const ArcPacket& pkt, uint8_t* buf, size_t bufLen);
    static bool     decode(const uint8_t* buf, size_t len, ArcPacket& pkt);
    static uint16_t crc16(const uint8_t* data, size_t len);
};
