# StickS3 compile probe evidence

## Scope

Isolated compile-only probe. It did not modify the KMB Bus Clock source tree, connect to a StickS3, flash hardware or prove runtime behavior.

## Configuration

- PlatformIO platform: `espressif32@6.12.0`
- Board: `esp32-s3-devkitc-1`
- Partition table: `default_8MB.csv`
- Flash/PSRAM memory type: `qio_opi`
- Build flags: `ESP32S3`, `BOARD_HAS_PSRAM`, native USB CDC/OTG flags from M5Stack's StickS3 documentation
- M5Unified: `0.2.19`, commit `4fb4447`
- M5GFX: `0.2.28`
- M5PM1: `1.0.7`, commit `be9a545`
- ArduinoJson: `6.21.5`
- Included and linked: M5Unified/M5GFX display bootstrap, M5PM1, Wi-Fi, WiFiClientSecure, HTTPClient, WebServer, Preferences and ArduinoJson

## Result

- PlatformIO build: PASS
- Framework: Arduino-ESP32 `3.20017.241212`, ESP-IDF 4.4 generation
- Static RAM: 48,348 / 327,680 bytes (14.8%)
- Flash: 960,965 / 3,342,336 bytes (28.8%)
- Firmware binary: 961,328 bytes
- SHA-256: `1af3a7076a4ab85652fafc19eb5459c21e895f96a3f6307c96d1e6183b1d8def`

## Interpretation

The official Arduino/PlatformIO driver stack can compile the core facilities the KMB Bus Clock needs. This does not prove the existing 320×240 application fits unchanged, that PSRAM is available at runtime, that the LCD orientation/offset is correct on a physical unit, that native USB WebSerial survives reset, or that the 250mAh battery meets the intended runtime. Those remain hardware acceptance tests.
