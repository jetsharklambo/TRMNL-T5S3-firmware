# Known Issues - TRMNL T5S3 FastEPD Firmware

**Current Version**: v1.1.1
**Last Updated**: 2026-04-07

---

## No Known Critical Issues ✅

All previously identified rendering and stability issues have been resolved in firmware v1.1.1 with the upstream FastEPD fix.

---

## Fixed Issues (Historical Reference)

### Issue 1: Display Rendering Artifacts (FIXED in v1.1.1 - April 7, 2026)

**Symptom**: Duplicate vertical strip on right edge (~5-10px), heap corruption warnings

**Root Cause**: FastEPD v1.3.0 called `bbepBackupPlane()` in 4-bpp mode when it shouldn't. The "previous buffer" is only for differential updates in 1/2-bpp mode, but the library tried to copy it anyway in 4-bpp mode.

**Resolution**: Upstream fix by FastEPD maintainer (commit 3e47f1e)
- Added guard: `if (pState->mode == BB_MODE_4BPP) return;` in `bbepBackupPlane()`
- Single-line fix, merged April 7, 2026
- See: https://github.com/bitbank2/FastEPD/commit/3e47f1e

**Impact**:
- ✅ Eliminates visual artifacts completely
- ✅ Prevents heap corruption
- ✅ Enables stable deep sleep
- ✅ Official upstream solution (no vendored code)

**Historical Note**: v1.1.0 used vendored FastEPD with 15 patches as temporary workaround. v1.1.1 uses clean upstream fix.

---

## Minor Known Limitations

### I2C Bus Conflicts (Low Priority)

**Symptom:** Occasional spurious button press events or invalid battery readings

**Cause:** PCA9555 I/O expander and BQ27220 fuel gauge share I2C bus (GPIO39 SDA, GPIO40 SCL). Display operations can temporarily corrupt bus state.

**Impact:**
- Fuel gauge may return invalid SOC (65535%) → firmware uses fallback value (50%)
- Button driver may detect ghost press events → firmware ignores spurious events

**Mitigation:** Multiple retries, fallback values, event filtering

**Status:** Acceptable - does not affect core functionality

---

## Library Status

### FastEPD (E-Paper Driver)

**Current**: Official upstream with fix (commit 3e47f1e)
**Status**: ✅ **STABLE** - No known issues in 4-bpp mode
**Source**: https://github.com/bitbank2/FastEPD.git#3e47f1e

### PNGdec (Image Decoder)

**Current**: v1.1.6 from PlatformIO registry
**Status**: ✅ **STABLE** - No known issues

### ArduinoJson (API Parsing)

**Current**: v6.21.3 from PlatformIO registry
**Status**: ✅ **STABLE** - No known issues

---

## Reporting Issues

**GitHub**: https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues
**FastEPD Issues**: https://github.com/bitbank2/FastEPD/issues

When reporting issues, please include:
- Firmware version (from serial output)
- Hardware model (LilyGo T5S3 Pro)
- Serial output logs
- Steps to reproduce
- Photos if visual issue

---

**Last Updated**: 2026-04-07
**Firmware Version**: v1.1.1-upstream-fixed
**FastEPD Version**: commit 3e47f1e (with 4-bpp fix)
