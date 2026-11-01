# ArcTrack Gen 6 — Device Specification

> **Status:** Draft
> **Generation:** 6

---

## 1. Overview

Gen 6 is the sixth generation of ArcTrack telemetry hardware. All Gen 6 devices share a common communication architecture built around the LoRa-E5 module, a unified packet format, and a shared set of proprietary device and status codes.

---

## 2. Device Lineup

| Device | Description |
|---|---|
| ArcTrack-BaseStations | Receives and logs LoRa transmissions from field devices |
| ArcTrack-Loggers | Records GPS position data to local storage (no LoRa) |
| ArcTrack-LoRa-Terrestrial | GPS tracker transmitting over LoRa for land use |
| ArcTrack-LoRa-Aquatic | GPS tracker transmitting over LoRa for marine/aquatic use |
| ArcTrack-GSM | GPS tracker transmitting over GSM/2G cellular and LoRa |
| ArcTrack-4G | GPS tracker transmitting over 4G LTE cellular and LoRa |
| ArcTrack-Argos | GPS tracker transmitting over the Argos satellite network |
| ArcTrack-Yagi | Directional LoRa + BLE receiver for bearing and tracking |

---

## 3. Core Requirements

### 3.1 LoRa-E5 Module

All Gen 6 hardware — **except the Logger** — is equipped with the Seeed Studio LoRa-E5 module. This module provides LoRa connectivity to every field device and base station, forming the backbone of the ArcTrack local area network.

**Applies to:** BaseStations, LoRa-Terrestrial, LoRa-Aquatic, GSM, 4G, Argos, Yagi
**Does not apply to:** Loggers

### 3.2 Shared Device & Status Codes (`codes.h`)

All LoRa-enabled Gen 6 systems must include a shared `codes.h` header that defines ArcTrack's proprietary identifiers. This ensures consistent interpretation of packets across all devices and the backend.

`codes.h` must define the following:

- **Device type codes** — a unique identifier for each device type, carried in every transmitted packet
- **System status codes** — indicate the current operational state of a device (e.g. booting, GPS searching, GPS fixed, transmitting, low battery, critical battery, charging, error)
- **Error codes** — indicate specific failure conditions (e.g. GPS timeout, LoRa init failure, LoRa TX failure, storage failure, modem failure, BLE failure)
- **Packet type codes** — indicate the purpose of a transmitted packet (e.g. position report, status beacon, error report, acknowledgement, configuration, heartbeat)

All LoRa-enabled projects must use these codes exclusively and must not define their own.

### 3.3 Shared Packet Structures (`structs.h`)

All LoRa-enabled Gen 6 systems must include a shared `structs.h` header that defines the binary packet structures used on-air. Individual projects must not define their own packet structs.

`structs.h` must define the following:

- **Common packet header** — device type, device ID, packet type, and timestamp, present in every packet
- **Position packet** — carries GPS latitude, longitude, altitude, speed, satellite count, and battery level
- **Status beacon packet** — carries system status code, battery level, and last received RSSI
- **Error report packet** — carries an error code and optional context bytes
- **Acknowledgement packet** — carries the ID and packet type of the packet being acknowledged

All structs must be packed (`#pragma pack`) to ensure consistent binary layout across different compilers and architectures.

---

## 4. Per-Device Capabilities

### ArcTrack-BaseStations

- Receives position, status, and error packets from all LoRa-enabled field devices
- Logs received packets to local storage (SD or flash)
- Uses: LoRa-E5, Storage, Comms

### ArcTrack-Loggers

- Records GPS position data to local storage at a configurable interval
- No LoRa capability — standalone data logger only
- Uses: GPS, Power, Storage, Comms

### ArcTrack-LoRa-Terrestrial

- Acquires GPS fix and transmits position packets over LoRa
- Optimised for land deployment (spreading factor, TX power TBD)
- Uses: GPS, LoRa-E5, Power, Comms

### ArcTrack-LoRa-Aquatic

- Acquires GPS fix and transmits position packets over LoRa
- Optimised for aquatic deployment — higher spreading factor and TX power to compensate for water attenuation and surface intermittency
- Uses: GPS, LoRa-E5, Power, Comms

### ArcTrack-GSM

- Acquires GPS fix and transmits position packets over GSM/2G cellular (primary) and LoRa (local fallback)
- Both radios transmit simultaneously on each cycle
- Uses: GPS, LoRa-E5, Power, Comms, GSM modem library (TBD)

### ArcTrack-4G

- Acquires GPS fix and transmits position packets over 4G LTE (primary) and LoRa (local fallback)
- Both radios transmit simultaneously on each cycle
- Uses: GPS, LoRa-E5, Power, Comms, 4G modem library (TBD)

### ArcTrack-Argos

- Acquires GPS fix and transmits position packets over the Argos satellite network
- No LoRa capability — satellite-only uplink
- Uses: GPS, Power, Comms, Argos module library (TBD)

### ArcTrack-Yagi

- Receives LoRa packets from nearby field devices and reports RSSI and SNR for direction-finding
- Also scans for BLE-advertising devices and reports signal strength
- Uses: LoRa-E5, BLE, Comms

---

## 5. Shared Library Summary

| Library | Purpose | Used by |
|---|---|---|
| `GPS` | GPS position acquisition | Loggers, LoRa-Terrestrial, LoRa-Aquatic, GSM, 4G, Argos |
| `LoRa` | LoRa-E5 radio send/receive | BaseStations, LoRa-Terrestrial, LoRa-Aquatic, GSM, 4G, Yagi |
| `Power` | Battery monitoring and sleep | Loggers, LoRa-Terrestrial, LoRa-Aquatic, GSM, 4G, Argos |
| `Storage` | SD/flash log and CSV append | BaseStations, Loggers |
| `Comms` | Packet encode/decode and CRC | All devices |
| `BLE` | BLE scan and GATT | Yagi |
| `codes.h` | Device, status, error, packet type codes | All LoRa-enabled devices |
| `structs.h` | Shared binary packet structures | All LoRa-enabled devices |
