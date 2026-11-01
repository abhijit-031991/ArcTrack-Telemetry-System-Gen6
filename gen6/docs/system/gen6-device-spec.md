# ArcTrack Gen 6 — Device Specification

> **Status:** Draft
> **Generation:** 6

---

## 1. Overview

Gen 6 is the sixth generation of ArcTrack telemetry hardware. All Gen 6 devices share a common hardware capability baseline — accelerometer, GPS, power monitoring — and a unified communication architecture built around the LoRa-E5 module, a shared packet format, and ArcTrack's proprietary device and status codes.

---

## 2. Device Lineup

| Device | Description |
|---|---|
| ArcTrack-BaseStations | Receives and logs LoRa transmissions from field devices |
| ArcTrack-Loggers | Records GPS position data to local storage (no radio) |
| ArcTrack-LoRa-Terrestrial | GPS tracker transmitting over LoRa for land use |
| ArcTrack-LoRa-Aquatic | GPS tracker transmitting over LoRa for marine/aquatic use |
| ArcTrack-GSM | GPS tracker — GSM/2G primary, LoRa secondary |
| ArcTrack-4G | GPS tracker — 4G LTE primary, LoRa secondary |
| ArcTrack-Argos | GPS tracker — Argos satellite primary, LoRa secondary |
| ArcTrack-Yagi | Directional LoRa + BLE receiver for bearing and tracking |

---

## 3. Hardware Capabilities

The following capabilities are present on **all Gen 6 devices** unless explicitly noted otherwise.

### 3.1 Accelerometer

Every Gen 6 device includes an accelerometer. This enables:

- **Motion detection** — wake from deep sleep on movement
- **Mortality alerts** — detect when a tagged animal has been stationary beyond a configurable threshold, indicating potential mortality, and trigger an alert transmission

A shared accelerometer library will be defined and referenced in a future update. All Gen 6 projects must use this shared library rather than implementing accelerometer handling independently.

### 3.2 GPS Module

Every Gen 6 device includes a GPS module. All devices will use one of the following supported modules:

| Module | Notes |
|---|---|
| **Quectel LC86** | Standard option |
| **u-blox SAM-M10Q** | Alternative option |

The firmware GPS library must support both modules through a common interface so that projects are not tied to a specific hardware choice.

### 3.3 Radio Communication

All Gen 6 devices — **except the Logger** — include radio communication via the LoRa-E5 module. Radio is classified as either primary or secondary depending on the device's primary uplink technology:

| Device | Primary | Secondary |
|---|---|---|
| ArcTrack-BaseStations | LoRa (E5) | — |
| ArcTrack-Loggers | — | — |
| ArcTrack-LoRa-Terrestrial | LoRa (E5) | — |
| ArcTrack-LoRa-Aquatic | LoRa (E5) | — |
| ArcTrack-GSM | GSM / 2G | LoRa (E5) |
| ArcTrack-4G | 4G LTE | LoRa (E5) |
| ArcTrack-Argos | Argos Satellite | LoRa (E5) |
| ArcTrack-Yagi | LoRa (E5) | — |

For GSM, 4G, and Argos devices, LoRa serves as a local-area fallback channel. Both radios transmit on each cycle regardless of whether the primary uplink succeeds.

### 3.4 Power Monitoring

Every Gen 6 device includes a power monitoring system that provides:

- **Battery voltage** — raw ADC reading converted to volts
- **Battery percentage** — mapped from voltage curve (0–100%)
- **Charge status** — whether the device is currently charging
- **Critical battery detection** — triggers a flush and enters low-power sleep when the battery falls below a safe threshold

Power monitoring is implemented via the shared `Power` library.

---

## 4. Communication Architecture

### 4.1 LoRa-E5 Module

All Gen 6 hardware — **except the Logger** — is equipped with the Seeed Studio LoRa-E5 module. This module provides LoRa connectivity to every field device and base station, forming the backbone of the ArcTrack local area network.

**Applies to:** BaseStations, LoRa-Terrestrial, LoRa-Aquatic, GSM, 4G, Argos, Yagi
**Does not apply to:** Loggers

### 4.2 Shared Device & Status Codes (`codes.h`)

All LoRa-enabled Gen 6 systems must include a shared `codes.h` header that defines ArcTrack's proprietary identifiers. This ensures consistent interpretation of packets across all devices and the backend.

`codes.h` must define the following:

- **Device type codes** — a unique identifier for each device type, carried in every transmitted packet
- **System status codes** — indicate the current operational state of a device (e.g. booting, GPS searching, GPS fixed, transmitting, low battery, critical battery, charging, error)
- **Error codes** — indicate specific failure conditions (e.g. GPS timeout, LoRa init failure, LoRa TX failure, storage failure, modem failure, BLE failure)
- **Packet type codes** — indicate the purpose of a transmitted packet (e.g. position report, status beacon, error report, acknowledgement, configuration, heartbeat)

All LoRa-enabled projects must use these codes exclusively and must not define their own.

### 4.3 Shared Packet Structures (`structs.h`)

All LoRa-enabled Gen 6 systems must include a shared `structs.h` header that defines the binary packet structures used on-air. Individual projects must not define their own packet structs.

`structs.h` must define the following:

- **Common packet header** — device type, device ID, packet type, and timestamp, present in every packet
- **Position packet** — carries GPS latitude, longitude, altitude, speed, satellite count, and battery level
- **Status beacon packet** — carries system status code, battery level, and last received RSSI
- **Error report packet** — carries an error code and optional context bytes
- **Acknowledgement packet** — carries the ID and packet type of the packet being acknowledged

All structs must be packed (`#pragma pack`) to ensure consistent binary layout across different compilers and architectures.

---

## 5. Per-Device Capabilities

### ArcTrack-BaseStations

- Receives position, status, and error packets from all LoRa-enabled field devices
- Logs received packets to local storage (SD or flash)
- **Hardware:** Accelerometer, GPS, LoRa-E5 (primary), Power monitoring
- **Libraries:** LoRa, Storage, Comms, Power, Accelerometer (TBD)

### ArcTrack-Loggers

- Records GPS position data to local storage at a configurable interval
- No radio capability — standalone data logger only
- **Hardware:** Accelerometer, GPS (LC86 or SAM-M10Q), Power monitoring
- **Libraries:** GPS, Power, Storage, Comms, Accelerometer (TBD)

### ArcTrack-LoRa-Terrestrial

- Acquires GPS fix and transmits position packets over LoRa
- Mortality alert transmitted as a LoRa packet when stationary threshold exceeded
- **Hardware:** Accelerometer, GPS (LC86 or SAM-M10Q), LoRa-E5 (primary), Power monitoring
- **Libraries:** GPS, LoRa, Power, Comms, Accelerometer (TBD)

### ArcTrack-LoRa-Aquatic

- Acquires GPS fix and transmits position packets over LoRa
- Higher spreading factor and TX power to compensate for water attenuation and surface intermittency
- Mortality alert transmitted as a LoRa packet when stationary threshold exceeded
- **Hardware:** Accelerometer, GPS (LC86 or SAM-M10Q), LoRa-E5 (primary), Power monitoring
- **Libraries:** GPS, LoRa, Power, Comms, Accelerometer (TBD)

### ArcTrack-GSM

- Acquires GPS fix and transmits over GSM/2G (primary) and LoRa (secondary) simultaneously
- Mortality alert transmitted on both channels when threshold exceeded
- **Hardware:** Accelerometer, GPS (LC86 or SAM-M10Q), LoRa-E5 (secondary), GSM modem, Power monitoring
- **Libraries:** GPS, LoRa, Power, Comms, Accelerometer (TBD), GSM modem library (TBD)

### ArcTrack-4G

- Acquires GPS fix and transmits over 4G LTE (primary) and LoRa (secondary) simultaneously
- Mortality alert transmitted on both channels when threshold exceeded
- **Hardware:** Accelerometer, GPS (LC86 or SAM-M10Q), LoRa-E5 (secondary), 4G modem, Power monitoring
- **Libraries:** GPS, LoRa, Power, Comms, Accelerometer (TBD), 4G modem library (TBD)

### ArcTrack-Argos

- Acquires GPS fix and transmits over Argos satellite (primary) and LoRa (secondary) simultaneously
- Mortality alert transmitted on both channels when threshold exceeded
- **Hardware:** Accelerometer, GPS (LC86 or SAM-M10Q), LoRa-E5 (secondary), Argos module, Power monitoring
- **Libraries:** GPS, LoRa, Power, Comms, Accelerometer (TBD), Argos module library (TBD)

### ArcTrack-Yagi

- Receives LoRa packets from nearby field devices and reports RSSI and SNR for direction-finding
- Scans for BLE-advertising devices and reports signal strength
- **Hardware:** Accelerometer, GPS (LC86 or SAM-M10Q), LoRa-E5 (primary), Power monitoring
- **Libraries:** LoRa, BLE, Comms, Power, Accelerometer (TBD)

---

## 6. Shared Library Summary

| Library | Purpose | Used by |
|---|---|---|
| `GPS` | GPS position acquisition; supports LC86 and SAM-M10Q | All devices |
| `LoRa` | LoRa-E5 radio send/receive | All except Loggers |
| `Power` | Battery monitoring and sleep management | All devices |
| `Storage` | SD/flash log and CSV append | BaseStations, Loggers |
| `Comms` | Packet encode/decode and CRC | All devices |
| `BLE` | BLE scanning and GATT | Yagi |
| `Accelerometer` | Motion detection and mortality alert logic (TBD) | All devices |
| `codes.h` | Device, status, error, and packet type codes | All LoRa-enabled devices |
| `structs.h` | Shared binary packet structures | All LoRa-enabled devices |
