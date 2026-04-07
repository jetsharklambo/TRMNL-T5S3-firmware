# 🎉 TRMNL-T5S3 Firmware v1.0.0 Release

**Release Date**: December 17, 2025
**Status**: Production Ready
**Platform**: LilyGo T5 4.7" E-Paper Display (ESP32-S3)

---

## 🚀 What's New

We're excited to announce the **first production-ready release** of the TRMNL-T5S3 firmware! This release brings a complete, feature-rich firmware for the LilyGo T5 4.7" e-paper display with seamless TRMNL cloud integration.

### ✨ Key Features

#### 🌐 Zero-Configuration Setup
- **WiFi Captive Portal**: Beautiful web interface for credential setup
- **Automatic Device Registration**: MAC-based authentication with TRMNL backend
- **Plug-and-Play**: Flash firmware → Connect to WiFi → Device auto-registers → Content displays

#### 📱 Smart Device Management
- **Dynamic Refresh Rates**: Server-controlled sleep intervals
- **OTA Firmware Updates**: Remote firmware management with rollback protection
- **Battery Monitoring**: Real-time telemetry with BQ27220 fuel gauge
- **Button Controls**: 5s soft reset, 15s hard reset/factory reset

#### 🔋 Power Efficiency
- **Deep Sleep Management**: Optimized for battery life
- **Wake Sources**: Button press or timer-based wakeup
- **Battery Telemetry**: Voltage, SOC, temperature, charging status
- **Smart Updates**: OTA only when battery >30%

#### 📊 Comprehensive Logging
- **SPIFFS Circular Buffer**: 100 entries, persistent across reboots
- **SD Card Daily Export**: Human-readable logs (YYYY-MM-DD.log)
- **API Log Submission**: Errors automatically sent to server
- **Device Telemetry**: Battery, WiFi, memory, uptime tracking

#### 🖼️ Reliable Display System
- **E-Paper Rendering**: PNG and BMP support (960x540)
- **HTTPS Downloads**: Secure image fetching with streaming
- **Automatic Refresh**: Content updates from TRMNL server

---

## 📦 Installation

### Option 1: Automated Flash Script (Recommended)

1. **Download Release Assets**:
   - Download `firmware.bin`, `bootloader.bin`, `partitions.bin`, and `flash.sh` from the [releases](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/releases/tag/v1.0.0) page

2. **Install esptool.py**:
   ```bash
   pip install esptool
   ```

3. **Run Flash Script**:
   ```bash
   cd releases/v1.0.0
   chmod +x flash.sh
   ./flash.sh
   ```

   The script will auto-detect your USB port and flash all components.

### Option 2: Manual Flash (Advanced Users)

```bash
# Install esptool.py
pip install esptool

# Flash bootloader
esptool.py --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
    write_flash 0x0 bootloader.bin

# Flash partition table
esptool.py --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
    write_flash 0x8000 partitions.bin

# Flash firmware
esptool.py --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
    write_flash 0x10000 firmware.bin
```

### Option 3: Build from Source

```bash
# Clone repository
git clone https://github.com/jetsharklambo/TRMNL-T5S3-firmware/TRMNL-T5S3.git
cd TRMNL-T5S3

# Install PlatformIO
pip install platformio

# Build and flash
pio run -t upload

# Monitor serial output
pio device monitor
```

---

## 🎯 First Boot Setup

After flashing the firmware:

1. **Device boots into AP mode** (no WiFi credentials)
2. **Connect** to WiFi network `TRMNL-XXXX` (using last 2 MAC bytes)
3. **Browser auto-redirects** to captive portal (192.168.4.1)
4. **Enter WiFi credentials** and submit
5. **Device reboots** and:
   - ✅ Connects to your WiFi
   - ✅ Auto-registers with TRMNL backend (MAC-based)
   - ✅ Retrieves API key from server
   - ✅ Downloads and displays your content
   - ✅ Enters deep sleep (server-controlled interval)

---

## 📊 Technical Specifications

### Build Information
- **Firmware Version**: 1.0.0
- **Flash Usage**: 65.1% (1,366,233 / 2,097,152 bytes)
- **RAM Usage**: 28.5% (93,320 / 327,680 bytes)
- **Heap Available**: 7.1-8.3MB (PSRAM + internal)
- **Compile Time**: ~8 seconds
- **Flash Time**: ~10 seconds

### Hardware Requirements
- **Device**: LilyGo T5 4.7" E-Paper Display (ESP32-S3)
- **Flash**: 2MB minimum
- **RAM**: 320KB SRAM + 8MB PSRAM
- **Battery**: LiPo battery with BQ27220 fuel gauge (recommended)
- **SD Card**: Optional (for daily log export)

### API Endpoints
- `GET /api/setup` - Device registration (MAC → API key)
- `GET /api/display` - Image URL and refresh rate
- `POST /api/log` - Error log submission

---

## 🔧 Configuration

All settings are in `include/config.h`:

```cpp
// Firmware Version
#define FIRMWARE_VERSION "1.0.0"

// Deep Sleep Interval (server-controlled, fallback value)
#define TRMNL_DEEP_SLEEP_SECONDS 300  // 5 minutes

// OTA Update Settings
#define OTA_ENABLE 1                   // Enable OTA updates
#define OTA_MIN_BATTERY_PERCENT 30     // Minimum battery for OTA
#define OTA_AUTO_UPDATE 1              // Auto-update when server sends command

// Logging
#define LOGGING_ENABLE 1               // Enable logging system
#define LOGGING_MAX_ENTRIES 100        // SPIFFS circular buffer size
#define SD_LOGGING_ENABLE 1            // Enable SD card daily export
```

---

## ⚠️ Known Limitations

1. **I2C Glitch After WiFi Init**: BQ27220 fuel gauge may return invalid readings immediately after WiFi connection. Fallback mechanism provides cached values until I2C recovers. This is expected behavior and does not affect operation.

2. **WiFi Retry Delays**: Currently uses fixed retry intervals. Escalating delays (exponential backoff) deferred to v1.1.

3. **First Boot Registration**: Device must be within WiFi range for initial setup. Once credentials are saved, the device can operate independently.

---

## 📚 Documentation

- **[README.md](README.md)** - Comprehensive project overview and quick start
- **[CHANGELOG.md](CHANGELOG.md)** - Detailed changelog
- **[FIRMWARE_EVOLUTION_PLAN.md](FIRMWARE_EVOLUTION_PLAN.md)** - Development roadmap
- **[PROJECT_STATUS.md](PROJECT_STATUS.md)** - Current status tracking
- **[CORE_FUNCTIONALITY_PRESERVATION.md](CORE_FUNCTIONALITY_PRESERVATION.md)** - Refactoring safety guidelines
- **[HOW_CORE_SYSTEMS_WORK.md](HOW_CORE_SYSTEMS_WORK.md)** - Technical deep-dive

---

## 🐛 Troubleshooting

### Device won't enter AP mode
- Hold the button for 15 seconds to trigger hard reset
- Serial command "clear" during 5-second boot window will erase credentials

### Can't flash firmware
- **macOS/Linux**: Install USB drivers (usually automatic)
- **Windows**: Install [CP210x drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- Try different baud rates: 460800 or 115200 instead of 921600
- Press and hold BOOT button while connecting USB cable

### Display not updating
- Check WiFi connection (device will re-enter AP mode on failure)
- Check battery level (>20% for operation, >30% for OTA)
- Monitor serial output for error messages: `screen /dev/cu.usbserial-XXXX 115200`
- Check TRMNL dashboard for device status

### OTA update fails
- Ensure battery >30%
- Check WiFi signal strength (RSSI in telemetry)
- Review error logs (SPIFFS or SD card)
- Firmware will rollback on failed update (bootloop protection active)

---

## 🛣️ Roadmap (v1.1 and Beyond)

### Planned for v1.1
- **Escalating WiFi Retry Delays**: Exponential backoff for failed connections
- **Enhanced Button Features**: Custom actions and multi-press detection
- **Power Consumption Optimizations**: Further battery life improvements
- **Additional Telemetry**: More detailed device health metrics

### Future Considerations
- **Multi-language Support**: Localized captive portal
- **Advanced Display Modes**: Partial refresh, grayscale rendering
- **Custom Plugin Support**: User-defined content rendering
- **Bluetooth Provisioning**: Alternative to WiFi AP setup

---

## 🙏 Acknowledgments

This release represents months of development, testing, and refinement. Special thanks to:

- **LilyGo** for the excellent T5 4.7" hardware
- **TRMNL** for the backend API and cloud infrastructure
- **ESP32 Community** for libraries and support
- **Beta Testers** for real-world validation

---

## 📞 Support

### Issues and Questions
- **GitHub Issues**: [https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues)
- **Documentation**: See links above
- **Serial Debugging**: Enable verbose logging in config.h

### Contributing
Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Test thoroughly on hardware
4. Submit a pull request with detailed description

---

## 📄 License

This firmware is provided as-is for personal and commercial use.
Based on ESP32 Arduino framework and epdiy library.

Original TRMNL firmware: [https://github.com/usetrmnl/firmware](https://github.com/usetrmnl/firmware)

---

## 🎉 Thank You!

We hope you enjoy using TRMNL-T5S3 firmware v1.0.0. This is just the beginning—stay tuned for future updates!

**Happy displaying! 📟✨**
