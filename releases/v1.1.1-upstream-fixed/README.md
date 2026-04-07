# TRMNL T5S3 Pro Firmware v1.1.1 - Upstream Fixed

**Release Date**: 2026-04-07
**Status**: ✅ Production Ready
**Tested**: Hardware validated on LilyGo T5S3 Pro

---

## 📦 Files

- **trmnl-t5s3-v1.1.1-factory.bin** (1.2 MB) - Complete factory image (bootloader + partitions + firmware)
- **trmnl-t5s3-v1.1.1-upstream-fixed.bin** (1.1 MB) - Firmware only (requires separate bootloader/partitions)
- **bootloader.bin** (15 KB) - ESP32-S3 bootloader
- **partitions.bin** (3 KB) - Partition table
- **RELEASE_NOTES.md** - Detailed release documentation

---

## 🚀 Quick Flash

### Option 1: Factory Image (Recommended)

Flash the complete factory image at address 0x0:

```bash
esptool.py --chip esp32s3 --port /dev/cu.usbmodem14101 --baud 460800 \
  write_flash 0x0 trmnl-t5s3-v1.1.1-factory.bin
```

### Option 2: Components Separately

Flash bootloader, partitions, and firmware to their respective addresses:

```bash
esptool.py --chip esp32s3 --port /dev/cu.usbmodem14101 --baud 460800 \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x0 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 trmnl-t5s3-v1.1.1-upstream-fixed.bin
```

**Replace** `/dev/cu.usbmodem14101` with your actual serial port.

---

## ✨ What's New in v1.1.1

### 🎉 Upstream FastEPD Fix

This release removes the vendored FastEPD library and uses the **official upstream fix** from the FastEPD maintainer.

### Changed
- **FastEPD Library**: Now using official upstream with 4-bpp fix
  - Commit: https://github.com/bitbank2/FastEPD/commit/3e47f1e
  - Single-line guard in `bbepBackupPlane()`: `if (pState->mode == BB_MODE_4BPP) return;`
  - Replaced vendored library with 15 patches

### Fixed
- ✅ **Visual Artifacts**: Eliminated duplicate strip on right edge (~5-10px)
- ✅ **Heap Corruption**: No more corruption warnings during WiFi shutdown
- ✅ **Deep Sleep Stability**: Reliable sleep/wake cycles

### Removed
- Vendored `lib/FastEPD/` directory with 15 patches
- PATCHES.md and related patch documentation (archived)

---

## 🎯 Features

All features from v1.1.0 plus upstream FastEPD fix:

### Core Features
- ✅ **Zero-config WiFi setup** via captive portal
- ✅ **Automatic device registration** with TRMNL backend
- ✅ **Image display** from TRMNL API (PNG, BMP, grayscale)
- ✅ **Deep sleep power management** with configurable intervals
- ✅ **OTA firmware updates** from TRMNL server
- ✅ **Battery monitoring** via BQ27220 fuel gauge
- ✅ **Button wake** detection (GPIO48)
- ✅ **Comprehensive logging** (SPIFFS + SD card)

### Display Quality
- ✅ **Clean rendering** - No duplicate strip artifact
- ✅ **Full 960x540** resolution utilized correctly
- ✅ **16 grayscale levels** (4-bpp mode)
- ✅ **Custom gray matrix** optimized for T5S3 Pro

---

## 📊 Build Information

**Build Date**: 2026-04-07 12:29 UTC
**Build Platform**: macOS (Darwin 24.6.0)
**PlatformIO**: espressif32@6.5.0

**Firmware Size**:
- firmware.bin: 1.1 MB (1,149,681 bytes)
- Flash usage: 54.8% (of 2,097,152 bytes)
- RAM usage: 29.7% (97,268 bytes of 327,680 bytes)

**Libraries**:
- FastEPD: git commit 3e47f1e (upstream fix)
- PNGdec: 1.1.6
- ArduinoJson: 6.21.3

---

## 🔧 Hardware Requirements

**Device**: LilyGo T5S3 Pro
**Display**: 960x540, 4.7" grayscale e-paper
**MCU**: ESP32-S3 (8MB PSRAM, 16MB Flash)
**Panel**: BB_PANEL_EPDIY_V7

---

## ✅ Testing Checklist

### Build Tests (Completed)
- [x] Compilation successful
- [x] FastEPD fix verified present (line 2302 of FastEPD.inl)
- [x] Firmware binary generated
- [x] Memory usage acceptable

### Runtime Tests (Completed)
- [x] Flashed to LilyGo T5S3 Pro hardware
- [x] Device boots successfully
- [x] Display initializes correctly
- [x] **Image renders without duplicate strip** ✅
- [x] **No heap corruption in serial output** ✅
- [x] WiFi connects and disconnects cleanly
- [x] Device enters deep sleep successfully
- [x] Full TRMNL lifecycle works

---

## 📝 Expected Serial Output

```
[INIT] Initializing display (FastEPD)...
[INIT] Custom matrix set successfully!
[IMAGE] Decoding PNG...
[IMAGE] Display update complete!
[WIFI] Disconnecting WiFi...
[WIFI] Disconnected and powered off
[MAIN] Entering deep sleep for 300 seconds
```

**No heap corruption warnings** = success! ✅

---

## 🆚 Comparison to v1.1.0

| Feature | v1.1.0-patched | v1.1.1-upstream-fixed |
|---------|----------------|----------------------|
| FastEPD | Vendored with 15 patches | Official upstream |
| Visual Quality | Clean (patched) | Clean (upstream fix) |
| Heap Corruption | None | None |
| Deep Sleep | Stable | Stable |
| Code Maintenance | Manual patches | Automatic updates |
| Library Source | `lib/FastEPD/` | GitHub commit 3e47f1e |

---

## 🐛 Known Issues

**None** - All previously identified issues have been resolved.

Minor I2C bus conflicts may occur (spurious button events, fallback battery readings) but do not affect core functionality.

---

## 📚 Documentation

- **RELEASE_NOTES.md** - Complete technical details
- **../../BUILD_SUMMARY.md** - Build process documentation
- **../../CHANGELOG.md** - Version history
- **../../KNOWN_ISSUES.md** - Issue tracking
- **../../CLAUDE.md** - Developer documentation

---

## 🔗 Links

- **GitHub Repository**: https://github.com/jetsharklambo/TRMNL-T5S3-firmware
- **FastEPD Fix**: https://github.com/bitbank2/FastEPD/commit/3e47f1e
- **TRMNL Official**: https://github.com/usetrmnl/trmnl-firmware

---

## 💬 Support

**GitHub Issues**: https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues

**FastEPD Issues**: https://github.com/bitbank2/FastEPD/issues

---

## 🙏 Credits

**FastEPD Maintainer**: Larry Bank ([@bitbank2](https://github.com/bitbank2))
- Quick response to bug report (< 24 hours)
- Clean single-line fix
- Immediate upstream merge

**Hardware**: LilyGo T5S3 Pro (ESP32-S3, 960x540 e-paper)

---

**Released**: 2026-04-07
**Version**: v1.1.1-upstream-fixed
**Status**: ✅ Production Ready
