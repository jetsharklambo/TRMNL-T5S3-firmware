# TRMNL-T5S3 Firmware v1.1.0

**Release Date**: January 10, 2026
**Platform**: LilyGo T5 4.7" E-Paper Display (ESP32-S3)

---

## 📦 Release Contents

This directory contains the production-ready v1.1.0 firmware binaries for the TRMNL-T5S3 device:

| File | Size | Description |
|------|------|-------------|
| `firmware.bin` | 1.3MB | Main firmware binary (flash at 0x10000) |
| `bootloader.bin` | 15KB | ESP32-S3 bootloader (flash at 0x0) |
| `partitions.bin` | 3KB | Partition table (flash at 0x8000) |
| `flash.sh` | 4KB | Automated flash script for macOS/Linux |

---

## 🚀 Quick Flash Instructions

### Option 1: Automated Script (Recommended)

```bash
# Install esptool.py
pip install esptool

# Make script executable
chmod +x flash.sh

# Run flash script (auto-detects USB port)
./flash.sh

# Or specify port manually
./flash.sh /dev/cu.usbserial-XXXX
```

### Option 2: Manual Flash

```bash
# Install esptool.py
pip install esptool

# Flash all components (replace PORT with your device port)
PORT=/dev/cu.usbserial-XXXX

esptool.py --chip esp32s3 --port $PORT --baud 921600 write_flash 0x0 bootloader.bin
esptool.py --chip esp32s3 --port $PORT --baud 921600 write_flash 0x8000 partitions.bin
esptool.py --chip esp32s3 --port $PORT --baud 921600 write_flash 0x10000 firmware.bin
```

---

## 📖 Documentation

For complete installation instructions, troubleshooting, and configuration:

- **[README.md](../../README.md)** - Complete project documentation
- **[RELEASE_NOTES_v1.1.0.md](../../RELEASE_NOTES_v1.1.0.md)** - Release notes and installation guide
- **[CHANGELOG.md](../../CHANGELOG.md)** - Detailed changelog

---

## ✨ What's New in v1.1.0

### Enhanced Features
- ✅ **BMP Image Decoder** - Smart centering for system images (sleep, low battery)
- ✅ **Boot Button Wake** - Press GPIO0 to wake from deep sleep
- ✅ **Device Model Header** - Proper API resolution with `Device-Model: m5papers3`
- ✅ **DEV_MODE Toggle** - Production mode for 3.2× battery life improvement

### Reliability Improvements
- ✅ **PNG Decode Reliability** - Automatic retry with better error handling
- ✅ **Download Integrity** - File size verification prevents corrupted downloads
- ✅ **Button Stabilization** - Fixed bootloop from GPIO38 false triggers
- ✅ **Hard Reset Fix** - Proper 5s warning vs 15s credential clear

### Power Optimization
- ✅ **Battery Life** - 3.2× improvement (2.9 → 9.2 days estimated)
- ✅ **Production Mode** - 2-second wake time vs 60-second dev mode
- ✅ **Enhanced Timing Logs** - Performance analysis and optimization metrics

---

## 🔧 Build Information

- **Version**: 1.1.0
- **Flash Usage**: 65.8% (1,380,589 / 2,097,152 bytes)
- **RAM Usage**: 28.5% (93,336 / 327,680 bytes)
- **Platform**: ESP32-S3
- **Framework**: Arduino
- **Build Tool**: PlatformIO

---

## 🆕 Upgrading from v1.0.0

All v1.0.0 features remain fully functional. WiFi credentials and API keys are preserved during OTA updates. You can:

1. **OTA Update**: Wait for automatic OTA (if enabled)
2. **Manual Flash**: Use the flash script above to update immediately

No breaking changes - fully backwards compatible!

---

## 🐛 Support

**Issues**: [GitHub Issues](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues)

**Troubleshooting**: See [RELEASE_NOTES_v1.1.0.md](../../RELEASE_NOTES_v1.1.0.md#troubleshooting) for common issues and solutions

---

## 📄 License

This firmware is provided as-is for personal and commercial use.
