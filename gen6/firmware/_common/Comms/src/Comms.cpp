#include "Comms.h"
#include <string.h>

uint16_t Comms::crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

size_t Comms::encode(const ArcPacket& pkt, uint8_t* buf, size_t bufLen) {
    size_t needed = 19 + pkt.payloadLen + 2;
    if (bufLen < needed) return 0;

    size_t i = 0;
    buf[i++] = pkt.deviceType;
    memcpy(buf + i, &pkt.deviceId,   4); i += 4;
    memcpy(buf + i, &pkt.timestamp,  4); i += 4;
    memcpy(buf + i, &pkt.latitude,   4); i += 4;
    memcpy(buf + i, &pkt.longitude,  4); i += 4;
    memcpy(buf + i, &pkt.payloadLen, 2); i += 2;
    memcpy(buf + i, pkt.payload, pkt.payloadLen); i += pkt.payloadLen;

    uint16_t crc = crc16(buf, i);
    memcpy(buf + i, &crc, 2); i += 2;
    return i;
}

bool Comms::decode(const uint8_t* buf, size_t len, ArcPacket& pkt) {
    if (len < 21) return false;

    size_t i = 0;
    pkt.deviceType = buf[i++];
    memcpy(&pkt.deviceId,   buf + i, 4); i += 4;
    memcpy(&pkt.timestamp,  buf + i, 4); i += 4;
    memcpy(&pkt.latitude,   buf + i, 4); i += 4;
    memcpy(&pkt.longitude,  buf + i, 4); i += 4;
    memcpy(&pkt.payloadLen, buf + i, 2); i += 2;
    if (len < i + pkt.payloadLen + 2) return false;
    memcpy(pkt.payload, buf + i, pkt.payloadLen); i += pkt.payloadLen;

    uint16_t received, expected = crc16(buf, i);
    memcpy(&received, buf + i, 2);
    return received == expected;
}
