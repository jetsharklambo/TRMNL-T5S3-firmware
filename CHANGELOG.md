# Changelog

All notable changes to the TRMNL-T5S3 firmware will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-12-17

### 🎉 First Production Release

This is the first production-ready release of the TRMNL-T5S3 firmware for the LilyGo T5 4.7" E-Paper Display (ESP32-S3).

### Added

#### Core Features
- **WiFi Captive Portal** - Automatic AP mode with web-based credential configuration
  - DNS spoofing for seamless setup experience
  - Beautiful responsive web UI with client-side validation
  - Credential persistence in NVRAM
  - Factory reset via serial command ("clear")

- **Device Auto-Registration** - Zero-configuration device setup
  - MAC-based device identification
  - Automatic registration with TRMNL backend (`/api/setup`)
  - API key and friendly ID retrieval
  - Persistent credential storage

- **Image Display System** - Production-grade e-paper rendering
  - PNG and BMP format support (1-bit and grayscale)
  - HTTPS image downloads with streaming
  - 960x540 resolution (4.7" e-paper display)
  - Automatic image refresh from TRMNL server (`/api/display`)

- **Power Management** - Optimized battery life
  - Deep sleep with configurable intervals
  - Dynamic refresh rate (server-controlled)
  - Button wake (GPIO48) and timer wake
  - BQ27220 fuel gauge integration
  - Real-time battery telemetry (voltage, SOC, temperature)

- **OTA Firmware Updates** - Remote firmware management
  - HTTPS firmware downloads with SSL/TLS validation
  - Triple firmware source priority (custom → API → official)
  - Dual partition OTA (OTA_0 ↔ OTA_1)
  - Rollback protection (prevents bootloop on failed updates)
  - Battery level safety check (30% minimum)
  - Automatic updates when triggered by server

- **Comprehensive Logging** - Diagnostics and debugging
  - SPIFFS circular buffer (100 entries, persistent across reboots)
  - SD card daily export (human-readable text files: YYYY-MM-DD.log)
  - API log submission to server (`/api/log`)
  - Device telemetry tracking (battery, WiFi, memory, uptime)
  - Error event logging at 5 critical points

- **Button Controls** - User interaction
  - 5-second hold: Soft reset (reboot device)
  - 15-second hold: Hard reset (factory reset + credential clear)
  - Wake from deep sleep

- **Telemetry Reporting** - Real-time device status
  - Battery voltage (from BQ27220 fuel gauge)
  - Battery percentage (state of charge)
  - Battery temperature (°C)
  - Battery status (normal/low/critical/full/charging)
  - Charging status (real-time detection)
  - WiFi signal strength (RSSI in dBm)
  - Firmware version tracking
  - All telemetry visible on TRMNL dashboard

#### Technical Infrastructure
- **Modular Architecture** - Clean separation of concerns
  - `display.cpp` - E-paper rendering engine
  - `wifi.cpp` - WiFi connectivity
  - `download.cpp` - HTTP streaming downloads
  - `sleep.cpp` - Deep sleep management
  - `nvram_config.cpp` - Non-volatile storage
  - `ap_server.cpp` - Captive portal server
  - `trmnl_api_client.h` - TRMNL API integration
  - `power.cpp` - BQ27220 battery monitoring
  - `logging.cpp` - Centralized logging API
  - `ota.cpp` - OTA update engine

- **NVRAM Persistence** - Non-volatile configuration
  - WiFi credentials (SSID, password)
  - API credentials (API key, friendly ID)
  - Dynamic refresh rate
  - WiFi failure counter (auto-recovery)

- **Error Handling** - Robust failure recovery
  - WiFi connection retry with automatic recovery
  - Failed credential detection and AP mode re-entry
  - Image download fallback mechanisms
  - OTA update validation and rollback
  - Graceful SD card failure handling

### Build Information
- **Version**: 1.0.0
- **Flash Usage**: 65.1% (1,366,233 / 2,097,152 bytes)
- **RAM Usage**: 28.5% (93,320 / 327,680 bytes)
- **Heap Available**: 7.1-8.3MB (PSRAM + internal)
- **Platform**: ESP32-S3 (LilyGo T5 4.7")
- **Framework**: Arduino + PlatformIO

### Dependencies
- ESP32 Arduino Core v2.0.14+
- ArduinoJson v6.x
- HTTPClient (built-in)
- WiFi (built-in)
- SPIFFS (built-in)
- SD (built-in)
- PNGdec library
- epdiy display library
- BQ27220 I2C driver

### Known Limitations
- **I2C Glitch After WiFi Init**: BQ27220 fuel gauge may return invalid readings immediately after WiFi connection. Fallback mechanism provides cached values until I2C recovers (expected behavior).
- **WiFi Retry Delays**: Escalating retry delays deferred to v1.1 (currently uses fixed intervals).

### Testing
- ✅ All core features tested on live hardware
- ✅ Extended operation testing (multiple sleep/wake cycles)
- ✅ Battery telemetry accuracy verified
- ✅ OTA updates tested successfully
- ✅ API integration confirmed working
- ✅ Logging systems verified (SPIFFS + SD card)

### Documentation
- README.md - Comprehensive project overview and quick start
- FIRMWARE_EVOLUTION_PLAN.md - Development roadmap
- CORE_FUNCTIONALITY_PRESERVATION.md - Refactoring safety guidelines
- HOW_CORE_SYSTEMS_WORK.md - Technical deep-dive
- SAFE_REFACTORING_PATTERNS.md - Code examples
- PROJECT_STATUS.md - Current status tracking
- CHANGELOG.md - This file
- RELEASE_NOTES_v1.0.0.md - Release announcement

### Migration Guide
This is the first release, so no migration is needed.

### Contributors
- Firmware development and architecture
- Hardware integration and testing
- Documentation and release preparation

---

## [Unreleased]

### Planned for v1.1
- Escalating WiFi retry delays (exponential backoff)
- Enhanced button features (custom actions)
- Power consumption optimizations
- Additional telemetry metrics

---

**Note**: For detailed installation instructions, see README.md or RELEASE_NOTES_v1.0.0.md
