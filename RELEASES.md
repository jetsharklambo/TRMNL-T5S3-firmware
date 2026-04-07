# Release History

This document tracks firmware releases for the LilyGo T5S3 Pro TRMNL implementation.

---

## v1.1.0-patched (April 6, 2026)

**Status**: ✅ Current Release - Production Ready

**Binary**: `trmnl-t5s3-v1.1.0-patched.bin`
**Size**: 1.2 MB
**Flash Address**: `0x0`

### Installation

```bash
esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
    write_flash 0x0 trmnl-t5s3-v1.1.0-patched.bin
```

### What's New

This release includes a **vendored FastEPD library** with critical patches for 4-bpp buffer calculation errors affecting 960×540 displays.

### Features

**Display System**:
- ✅ Clean rendering across full 960×540 display (no artifacts)
- ✅ FastEPD v1.3.0 with 15 surgical patches applied
- ✅ 5 configurable render modes for different use cases
- ✅ Custom gray scale matrix (u8M5Matrix) optimized for M5Paper-style displays
- ✅ 5px horizontal offset for better bezel centering

**Core Functionality**:
- ✅ Zero-configuration WiFi setup via captive portal
- ✅ Automatic device registration using MAC address
- ✅ Remote content updates from TRMNL dashboard
- ✅ Deep sleep power management (~1 week battery life)
- ✅ Over-the-air firmware updates with safety checks
- ✅ Battery monitoring via BQ27220 fuel gauge

**Hardware Support**:
- ✅ ESP32-S3 (LilyGo T5 4.7" E-Paper Display)
- ✅ 960×540 grayscale e-paper display
- ✅ PCA9555 I/O expander (button detection)
- ✅ BQ27220 fuel gauge (battery monitoring)

**API Integration**:
- ✅ TRMNL API client (/api/setup, /api/display, /api/log)
- ✅ Device telemetry (battery, WiFi RSSI, firmware version)
- ✅ Server-controlled refresh intervals
- ✅ Remote firmware update triggers

### FastEPD Patches

**What was fixed**: 15 buffer calculation errors in FastEPD v1.3.0

**Categories**:
1. **Buffer Allocation** (3 fixes) - Lines 1288, 1289, 1295
2. **Row Operations** (8 fixes) - Lines 1553, 1562, 1575, 1577, 1579, 1635, 1636, 1672
3. **Array Indexing** (4 fixes) - Lines 1610, 1633, 1758, 1796
4. **Loop Iterations** (8 fixes) - Lines 1614, 1621, 1655, 1663, 1762, 1769, 1820, 1828
5. **Write Operations** (5 fixes) - Lines 1799, 1800, 1846, 1848, 1849

**Impact**:
- ✅ Eliminates duplicate right-edge rendering artifact
- ✅ Prevents heap corruption during WiFi operations
- ✅ Enables stable deep sleep operation
- ✅ No performance degradation
- ✅ Minimal memory increase (~480 bytes)

**Details**: See `lib/FastEPD/PATCHES.md`

### Build Information

**Platform**: espressif32@6.5.0
**Framework**: Arduino
**Board**: T5-ePaper-S3 (ESP32-S3)
**Flash Usage**: 54.7% (1,146,673 / 2,097,152 bytes)
**RAM Usage**: 29.7% (97,252 / 327,680 bytes)

### Installation

Download `trmnl-t5s3-v1.1.0-patched.bin` and flash to device:

```bash
esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
  write_flash 0x0 trmnl-t5s3-v1.1.0-patched.bin
```

See `README.md` for detailed installation instructions.

### Known Issues

**Minor Issues**:
- I2C bus conflicts: PCA9555 and BQ27220 on same bus can cause spurious button events or invalid battery readings after WiFi/display operations
- Workaround: Multiple retries, fallback values, ignore spurious events
- Impact: Low - does not affect normal operation

**Resolved Issues**:
- ✅ FastEPD rendering artifact (fixed via vendored patches)
- ✅ Heap corruption during WiFi shutdown (fixed via buffer patches)
- ✅ Unstable deep sleep operation (fixed via buffer patches)

### Documentation

**User Guides**:
- `README.md` - Installation and setup guide
- `FLASH_INSTRUCTIONS.md` - Detailed flashing instructions
- `KNOWN_ISSUES.md` - Current limitations

**Developer Guides**:
- `CLAUDE.md` - Complete architecture documentation
- `CONTRIBUTING.md` - Development guidelines
- `DIAGNOSTIC_TESTING.md` - Testing procedures

**Technical Details**:
- `lib/FastEPD/PATCHES.md` - Detailed patch documentation
- `COMPARISON.md` - Patched vs unpatched comparison
- `CHANGELOG.md` - Detailed change log

### Upstream Status

**TRMNL PR**: Pending submission to https://github.com/usetrmnl/trmnl-firmware

**FastEPD Bug**: To be filed at https://github.com/bitbank2/FastEPD/issues

**Purpose**: This release serves as:
- ✅ Working reference implementation for upstream contribution
- ✅ Proof of concept showing patches resolve rendering artifacts
- ✅ Community resource for T5S3 users who want clean rendering today
- ✅ Portfolio showcase of embedded systems debugging

---

## v1.1.0 (April 2026)

**Status**: Superseded by v1.1.0-patched

### Summary

Initial release using unpatched FastEPD v1.3.0 library. All core functionality working, but exhibits known rendering artifact on right edge due to upstream library bug.

### Features

Same as v1.1.0-patched except:
- ❌ Uses unpatched FastEPD v1.3.0 from PlatformIO registry
- ❌ Duplicate vertical strip on right edge (~5-10 pixels)
- ⚠️ Heap corruption warnings during WiFi shutdown
- ⚠️ Occasional deep sleep failures

### Why Superseded

Vendored FastEPD patches in v1.1.0-patched eliminate rendering artifact and improve stability with minimal overhead.

---

## Development Builds

### v1.0.0-dev (March 2026)

**Status**: Development / Testing Only

Early development build using EPDiy library directly. Functional but significantly larger codebase (751 lines in display.cpp vs 456 lines with FastEPD).

**Migration**: Switched to FastEPD library for cleaner API and reduced code complexity.

---

## Version Naming

**Format**: `vMAJOR.MINOR.PATCH[-suffix]`

**Suffixes**:
- `-patched` - Includes vendored FastEPD patches
- `-dev` - Development/testing builds
- (none) - Standard release using upstream libraries

**Examples**:
- `v1.1.0-patched` - Production release with vendored FastEPD patches
- `v1.1.0` - Standard release using upstream FastEPD v1.3.0
- `v1.0.0-dev` - Development build

---

## Upgrade Path

### From v1.1.0 → v1.1.0-patched

**Recommended**: Yes - eliminates rendering artifact with no functional changes

**Process**:
1. Download `trmnl-t5s3-v1.1.0-patched.bin`
2. Flash to device (same process as initial installation)
3. Device retains WiFi credentials and API keys
4. No reconfiguration needed

**Changes**:
- ✅ Clean rendering (no right-edge artifact)
- ✅ More stable deep sleep operation
- ✅ No heap corruption warnings
- ⏺ All credentials and settings preserved

### When Upstream FastEPD is Fixed

**Process**:
1. Update `platformio.ini`: `bitbank2/FastEPD@^1.4.0` (or version with fix)
2. Delete `lib/FastEPD/` directory
3. Rebuild: `~/.platformio/penv/bin/platformio run`
4. Flash updated firmware

**Result**: Same clean rendering using official library (no vendored patches needed)

---

## Release Artifacts

Each release includes:

**Binary Files**:
- `trmnl-t5s3-vX.X.X-patched.bin` - Full firmware binary (flash to 0x0)

**Documentation**:
- `README.md` - Installation guide
- `CHANGELOG.md` - Detailed changes
- `KNOWN_ISSUES.md` - Known limitations
- `CLAUDE.md` - Architecture documentation

**Source Code**:
- Complete PlatformIO project
- All source modules (14 total)
- Vendored FastEPD library (patched versions only)
- Build configuration (platformio.ini, partitions.csv)

---

## Hardware Compatibility

**Supported Devices**:
- ✅ LilyGo T5 4.7" E-Paper Display (ESP32-S3)
- ✅ Model: T5-4.7-inch-e-paper-V2.3
- ✅ Display: 960×540 grayscale e-paper
- ✅ Processor: ESP32-S3-WROOM-1 (16MB flash, 8MB PSRAM)

**Not Supported**:
- ❌ T5 4.7" with ESP32 (non-S3) - different pin mappings
- ❌ Other display sizes (540×960, 1024×758, etc.) - would need testing
- ❌ Non-LilyGo EPDiy boards - different hardware configuration

---

## Support & Feedback

**Issues**: https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues

**Before reporting**:
1. Check `KNOWN_ISSUES.md`
2. Check existing GitHub issues
3. Review troubleshooting section in `README.md`

**When reporting**:
- Device model and firmware version
- Serial output (115200 baud)
- Steps to reproduce
- Expected vs actual behavior

---

## License

GPL-3.0 (matching TRMNL upstream)

Derived from TRMNL project. FastEPD library by Larry Bank.

---

**Last Updated**: April 6, 2026
**Current Release**: v1.1.0-patched
**Next Release**: TBD (depends on upstream FastEPD fix)
