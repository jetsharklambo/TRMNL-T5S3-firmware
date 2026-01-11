# 🎉 TRMNL-T5S3 Firmware v1.1.0 Release

**Release Date**: January 10, 2026
**Status**: Production Ready
**Platform**: LilyGo T5 4.7" E-Paper Display (ESP32-S3)

---

## 🚀 What's New

We're excited to announce **v1.1.0** of the TRMNL-T5S3 firmware! This release brings significant improvements to image rendering, power management, and user experience based on real-world testing and user feedback.

### ✨ Key Features

#### 🖼️ Enhanced Image Display

**1. BMP Image Decoder with Smart Centering**
- Full support for 1-bit monochrome BMP system images
- Intelligent centering algorithm for resolution mismatches
- Automatically centers 800×480 images on 960×540 display with black letterbox bars
- Handles BMP format quirks (bottom-to-top row order, 4-byte padding)
- Display clearing before BMP render prevents ghosting
- **Result**: Sleep screens and low battery warnings now display correctly ✅

**Technical Details:**
- New modules: `src/bmp_decoder.cpp`, `include/bmp_decoder.h`
- Integration with display pipeline in `src/display.cpp`
- Automatic format detection (PNG vs BMP)

#### 🔋 Power & Battery Optimization

**2. Development vs Production Mode**
- New `DEV_MODE` compile-time flag in `include/config.h`
- **Production mode** (DEV_MODE=0): Optimized for battery life
  - 2-second wake time before sleep
  - Minimal serial output
  - **3.2× battery life improvement** (2.9 → 9.2 days estimated)
- **Development mode** (DEV_MODE=1): Optimized for debugging
  - 60-second wake time for serial monitoring
  - Verbose logging and diagnostics
  - Enhanced timing reports

**3. Enhanced Timing Logs**
- Comprehensive cycle timing summaries
- Performance analysis metrics
- Battery usage insights
- Helps developers optimize power consumption

#### 🎮 User Experience Improvements

**4. Boot Button Wake from Deep Sleep**
- Press **GPIO0 (boot button)** to wake device instantly
- On-demand refresh without waiting for timer
- Provides manual control alongside automatic updates
- Perfect for testing or viewing latest content immediately

**Technical Details:**
- Wake source configured in `src/sleep.cpp`
- Button handling in `src/button.cpp`
- Works alongside timer-based wake

#### 🌐 API Integration Enhancement

**5. Device Model Identification**
- Firmware now sends `Device-Model: m5papers3` header to TRMNL API
- Enables server-side resolution lookup (960×540)
- Plugin images automatically rendered at correct resolution
- Better compatibility with TRMNL ecosystem

**Technical Details:**
- Modified `src/trmnl_api_client.h`
- Proper device identification in API headers

#### 🛡️ Reliability Improvements

**6. PNG Decode Reliability**
- Automatic retry logic on decode failures
- Better PSRAM allocation with RAM fallback
- Detailed error reporting with diagnostics
- **Result**: All previously failing images now render successfully ✅

**7. Download Integrity Verification**
- File size validation against HTTP `Content-Length` header
- SPIFFS write verification
- Corrupted download detection and prevention
- Reduces failed display updates

**8. Button Boot Stabilization**
- Fixed bootloop caused by GPIO38 false triggers
- 3-second hardware stabilization period on boot
- Proper initial state tracking
- More reliable boot sequence

**9. 15-Second Hard Reset Fix**
- 5-second hold now shows warning (no immediate reboot)
- User can continue holding to 15 seconds for credential clear
- Proper distinction between soft reset (5-15s) and hard reset (15s+)
- Better user feedback

---

## 📦 Installation

### Option 1: Automated Flash Script (Recommended)

1. **Download Release Assets**:
   - Download `firmware.bin`, `bootloader.bin`, `partitions.bin`, and `flash.sh` from the [v1.1.0 release page](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/releases/tag/v1.1.0)

2. **Install esptool.py**:
   ```bash
   pip install esptool
   ```

3. **Run Flash Script**:
   ```bash
   cd releases/v1.1.0
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
git clone https://github.com/jetsharklambo/TRMNL-T5S3-firmware.git
cd TRMNL-T5S3-firmware

# Checkout v1.1.0 tag
git checkout v1.1.0

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
- **Firmware Version**: 1.1.0
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

### New Headers in v1.1.0
- `Device-Model: m5papers3` - Enables proper server-side resolution

---

## 🔧 Configuration

All settings are in `include/config.h`:

```cpp
// Firmware Version
#define FIRMWARE_VERSION "1.1.0"

// Development vs Production Mode
#define DEV_MODE 0  // 0 = Production (battery optimized), 1 = Development (debug friendly)

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

2. **WiFi Retry Delays**: Currently uses fixed retry intervals. Escalating delays (exponential backoff) planned for v1.2.

3. **First Boot Registration**: Device must be within WiFi range for initial setup. Once credentials are saved, the device can operate independently.

---

## 🆕 What Changed from v1.0.0

### Files Modified
- `include/config.h` - Added DEV_MODE, updated version to 1.1.0
- `src/main.cpp` - Enhanced timing logs, button event handling
- `src/button.cpp` - Boot stabilization, GPIO0 wake source
- `src/display.cpp` - PNG retry logic, BMP integration
- `src/download.cpp` - Download integrity verification
- `src/sleep.cpp` - GPIO0 wake configuration
- `src/trmnl_api_client.h` - Device-Model header

### Files Added
- `src/bmp_decoder.cpp` - BMP decoder implementation (NEW)
- `include/bmp_decoder.h` - BMP decoder interface (NEW)
- `RELEASE_NOTES_v1.1.0.md` - These release notes (NEW)

### No Breaking Changes
- All v1.0.0 features remain fully functional
- WiFi credentials, API keys, and settings preserved during OTA update
- Backwards compatible with TRMNL API

---

## 📚 Documentation

- **[README.md](README.md)** - Comprehensive project overview and quick start
- **[CHANGELOG.md](CHANGELOG.md)** - Detailed changelog
- **[FIRMWARE_EVOLUTION_PLAN.md](FIRMWARE_EVOLUTION_PLAN.md)** - Development roadmap
- **[PROJECT_STATUS.md](PROJECT_STATUS.md)** - Current status tracking
- **[BATTERY_OPTIMIZATION_REPORT.md](BATTERY_OPTIMIZATION_REPORT.md)** - Battery life analysis
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

### BMP images not displaying
- Ensure BMP is 1-bit monochrome format
- Check that image file downloaded successfully (check serial logs)
- Verify SPIFFS has enough space

### OTA update fails
- Ensure battery >30%
- Check WiFi signal strength (RSSI in telemetry)
- Review error logs (SPIFFS or SD card)
- Firmware will rollback on failed update (bootloop protection active)

---

## 🛣️ Roadmap (v1.2 and Beyond)

### Planned for v1.2
- **JPEG Image Support** - Add JPEG decoding for more plugin compatibility
- **QR Code WiFi Setup** - Display QR code for easier mobile setup
- **Escalating WiFi Retry Delays** - Exponential backoff for failed connections
- **Loading Animation** - Visual feedback during downloads

### Future Considerations
- **Multi-Board Support** - Support for other ESP32-S3 e-paper boards
- **Bluetooth Provisioning** - Alternative to WiFi AP setup
- **Partial Display Refresh** - Faster updates for e-paper
- **Custom Plugin Support** - User-defined content rendering

---

## 🙏 Acknowledgments

This release represents the culmination of extensive testing and refinement following the successful v1.0.0 launch. Special thanks to:

- **LilyGo** for the excellent T5 4.7" hardware
- **TRMNL** for the backend API and cloud infrastructure
- **ESP32 Community** for libraries and support
- **Early Adopters** for real-world testing and feedback

---

## 📞 Support

### Issues and Questions
- **GitHub Issues**: [https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues)
- **Documentation**: See links above
- **Serial Debugging**: Enable DEV_MODE in config.h for verbose logging

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

Original TRMNL firmware: [https://github.com/usetrmnl/trmnl-firmware](https://github.com/usetrmnl/trmnl-firmware)

---

## 🎉 Thank You!

We hope you enjoy using TRMNL-T5S3 firmware v1.1.0. This release brings significant improvements to reliability, power efficiency, and user experience.

**Upgrade from v1.0.0 today to experience:**
- ✅ Better image compatibility (BMP support)
- ✅ Improved power efficiency (3.2× battery life)
- ✅ Enhanced user control (boot button wake)
- ✅ More reliable operation (retry logic, integrity checks)

**Happy displaying! 📟✨**
