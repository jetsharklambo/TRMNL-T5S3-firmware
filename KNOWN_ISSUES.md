# Known Issues - TRMNL T5S3 FastEPD Firmware

## No Known Critical Issues

**Status:** All previously identified rendering and stability issues have been resolved in firmware v1.1.0.

---

## Fixed Issues (Historical Reference)

### Issue 1: Display Rendering Artifacts (FIXED in v1.1.0)

**Previous Symptom:** Repeated vertical strip on right edge, missing pixels on left edge

**Root Cause:** FastEPD library v1.3.0 has buffer calculation bugs in 4BPP mode. The library incorrectly uses `/4` divisor (for 2-bit-per-pixel mode) instead of `/2` divisor (for 4-bit-per-pixel mode) in 15 locations across buffer allocation, indexing, and transfer operations.

**Resolution:** Applied local patches to `.pio/libdeps/T5_E_PAPER_S3_V7/FastEPD/src/FastEPD.inl`:
- Fixed 13 buffer calculation instances (changed `/4` → `/2`)
- Doubled DMA buffer allocation for proper double-buffering
- Added 5px horizontal offset (IMAGE_X_OFFSET) for better centering

**Impact:**
- ✅ Eliminates rendering artifacts
- ✅ Prevents heap corruption crashes
- ✅ Enables stable deep sleep operation
- ✅ Full 960x540 pixel display with correct positioning

See CLAUDE.md "FastEPD Library Patches" section for complete technical details.

### Issue 2: Heap Corruption Crash After Render (FIXED in v1.1.0)

**Previous Symptom:** Device crashed during WiFi shutdown with "CORRUPT HEAP: Bad head at 0x3fcb5fa8"

**Root Cause:** DMA buffer allocated only 480 bytes (`width/2`) but double-buffering with XOR toggle required 960 bytes (`width`) for 4BPP mode.

**Resolution:** Line 1295 in FastEPD.inl - doubled DMA buffer allocation from `(pState->width / 2)` to `(pState->width)`

**Impact:**
- ✅ Device successfully enters deep sleep without crashes
- ✅ Stable WiFi shutdown sequence
- ✅ Minimal memory cost (~480 bytes, negligible on ESP32-S3 with 8MB PSRAM)

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

## FastEPD Library Dependency

**⚠️ IMPORTANT:** This firmware uses **locally patched FastEPD v1.3.0**.

**DO NOT upgrade FastEPD library** without one of the following:
1. Reapplying all 15 patches to the new version
2. Confirming upstream library has fixed 4BPP buffer calculation bugs
3. Extensive testing to verify rendering stability

**For testing:** A comparison variant using unpatched public FastEPD library is available in `../TRMNL-T5S3-FastEPD-Public/` to demonstrate the bugs present in upstream library.

See VARIANTS.md in parent directory for comparison details.

---

**Last Updated:** 2026-04-03
**Firmware Version:** 1.1.0
**FastEPD Version:** 1.3.0 (locally patched)
**Panel Config:** BB_PANEL_EPDIY_V7
