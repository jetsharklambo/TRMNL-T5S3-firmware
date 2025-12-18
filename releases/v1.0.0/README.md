# TRMNL-T5S3 Firmware v1.0.0

**Release Date**: December 17, 2025
**Platform**: LilyGo T5 4.7" E-Paper Display (ESP32-S3)

---

## 📦 Release Contents

This directory contains the production-ready v1.0.0 firmware binaries for the TRMNL-T5S3 device:

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
- **[RELEASE_NOTES_v1.0.0.md](../../RELEASE_NOTES_v1.0.0.md)** - Release notes and installation guide
- **[CHANGELOG.md](../../CHANGELOG.md)** - Detailed changelog

---

## ✨ What's Included in v1.0.0

- ✅ WiFi captive portal with auto-setup
- ✅ Automatic device registration with TRMNL backend
- ✅ E-paper display rendering (PNG/BMP support)
- ✅ OTA firmware updates with rollback protection
- ✅ Battery monitoring and telemetry
- ✅ Comprehensive logging (SPIFFS + SD card)
- ✅ Button controls (5s soft reset, 15s hard reset)
- ✅ Deep sleep power management
- ✅ Dynamic server-controlled refresh rates

---

## 🔧 Build Information

- **Version**: 1.0.0
- **Flash Usage**: 65.1% (1,366,233 / 2,097,152 bytes)
- **RAM Usage**: 28.5% (93,320 / 327,680 bytes)
- **Platform**: ESP32-S3
- **Framework**: Arduino
- **Build Tool**: PlatformIO

---

## 🐛 Support

**Issues**: [GitHub Issues](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues)

**Troubleshooting**: See [README.md](../../README.md#troubleshooting) for common issues and solutions

---

## 📄 License

This firmware is provided as-is for personal and commercial use.
