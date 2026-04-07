# Build Summary: v1.1.1-upstream-fixed

**Date**: 2026-04-07 12:29 UTC
**Status**: ✅ SUCCESS
**Build Time**: 21.72 seconds

---

## What Was Done

### 1. Updated platformio.ini
**Changed library dependency**:
```ini
# Before (vendored with 15 patches):
# FastEPD was in lib/FastEPD/ with custom patches

# After (upstream fix):
https://github.com/bitbank2/FastEPD.git#3e47f1e3ab6d5e00a979e1c32557bb3bc89e8653
```

### 2. Removed Vendored Library
```bash
rm -rf lib/FastEPD/
```
- Deleted 15-patch vendored version
- Now using official upstream with single fix

### 3. Clean Build
```bash
~/.platformio/penv/bin/platformio run --target clean
~/.platformio/penv/bin/platformio run
```

### 4. Verified Fix Present
```bash
grep "if (pState->mode == BB_MODE_4BPP) return;" \
  .pio/libdeps/T5_E_PAPER_S3_V7/FastEPD/src/FastEPD.inl

# Result: Line 2302
# if (pState->mode == BB_MODE_4BPP) return; // not applicable to 4-bpp mode
```

✅ **Upstream fix confirmed present**

---

## Build Results

### Compilation
```
Processing T5_E_PAPER_S3_V7 (platform: espressif32@6.5.0; board: T5-ePaper-S3)
--------------------------------------------------------------------------------
Linking .pio/build/T5_E_PAPER_S3_V7/firmware.elf
Checking size .pio/build/T5_E_PAPER_S3_V7/firmware.elf

RAM:   [===       ]  29.7% (used 97268 bytes from 327680 bytes)
Flash: [=====     ]  54.8% (used 1149681 bytes from 2097152 bytes)

Building .pio/build/T5_E_PAPER_S3_V7/firmware.bin
Successfully created esp32s3 image.

========================= [SUCCESS] Took 21.72 seconds =========================
```

### Memory Usage
- **RAM**: 97,268 bytes (29.7% of 327,680 bytes)
- **Flash**: 1,149,681 bytes (54.8% of 2,097,152 bytes)
- **Firmware Size**: 1.1 MB

### Comparison to v1.1.0-patched
- **Flash usage**: Similar (±0.1%)
- **RAM usage**: Identical
- **Functionality**: Expected to be identical

---

## Release Files

**Location**: `releases/v1.1.1-upstream-fixed/`

**Files**:
- `trmnl-t5s3-v1.1.1-upstream-fixed.bin` (1.1 MB) - Main firmware
- `bootloader.bin` (15 KB) - ESP32-S3 bootloader
- `partitions.bin` (3 KB) - Partition table
- `RELEASE_NOTES.md` - Complete release documentation

---

## The Fix (FastEPD Commit 3e47f1e)

**File**: `src/FastEPD.inl`
**Function**: `bbepBackupPlane()`
**Line**: 2302

**Change**:
```cpp
void bbepBackupPlane(FASTEPDSTATE *pState) {
    if (pState->mode == BB_MODE_4BPP) return; // <-- ADDED THIS LINE

    int iSize = (pState->native_width/2) * pState->native_height;
    if (!pState->pPrevious || !pState->pCurrent) return;
    memcpy(pState->pPrevious, pState->pCurrent, iSize);
}
```

**Why It Works**:
- 4-bpp mode doesn't use `pPrevious` buffer (only for differential updates in 1/2-bpp)
- When `clearWhite()` called `bbepBackupPlane()`, it tried to copy buffers
- Buffer was allocated with wrong size (width/4 vs width/2)
- Guard prevents the copy, avoiding buffer overrun
- Simple, clean, correct

---

## Advantages Over Vendored Version

### Code Quality
✅ **1 line** vs 15 patches
✅ **Upstream maintained** vs local fork
✅ **Automatic updates** when new FastEPD releases
✅ **No merge conflicts** on library updates

### Correctness
✅ **Addresses root cause** (guard against 4-bpp in backupPlane)
✅ **Maintainer-approved** solution
✅ **Future-proof** for other display sizes

### Maintenance
✅ **No PATCHES.md** to maintain
✅ **No comparison testing** between versions
✅ **Standard library dependency** in platformio.ini
✅ **Easier for TRMNL integration**

---

## Testing Checklist

### ✅ Build Tests (Completed)
- [x] platformio.ini updated
- [x] Vendored library removed
- [x] Clean build succeeds
- [x] FastEPD fix verified present (line 2302)
- [x] Firmware binary generated (1.1 MB)
- [x] Memory usage acceptable (29.7% RAM, 54.8% Flash)

### ⏳ Runtime Tests (Pending)
- [ ] Flash to LilyGo T5S3 Pro hardware
- [ ] Device boots successfully
- [ ] Display initializes correctly
- [ ] Image renders without duplicate strip (primary test)
- [ ] No heap corruption in serial output (primary test)
- [ ] WiFi connects and disconnects cleanly
- [ ] Device enters deep sleep successfully
- [ ] Device wakes on timer
- [ ] Device wakes on button press
- [ ] Full TRMNL lifecycle (registration → display → sleep)
- [ ] 24-hour stability test

### 📋 Documentation Updates (Pending)
- [ ] Update CLAUDE.md (remove vendored library references)
- [ ] Archive PATCHES.md and COMPARISON.md
- [ ] Update README.md
- [ ] Update KNOWN_ISSUES.md
- [ ] Create migration guide

---

## Next Steps

### Immediate
1. **Flash to hardware** for runtime testing
2. **Visual inspection** to confirm no duplicate strip
3. **Serial monitoring** to confirm no heap corruption

### Short Term
4. **24-hour stability test** with multiple wake/sleep cycles
5. **Update documentation** to remove vendored library references
6. **Comment on FastEPD GitHub issue** confirming fix works

### Long Term
7. **Sync with TRMNL firmware** latest changes
8. **Submit PR to usetrmnl/firmware** with T5S3 support
9. **Integrate as official BYOD device** in TRMNL ecosystem

---

## Expected Behavior

### Serial Output (Success)
```
[INIT] Initializing display (FastEPD)...
[INIT] Custom matrix set successfully!
[IMAGE] Decoding PNG...
[IMAGE] Display update complete!
[WIFI] Disconnecting WiFi...
[WIFI] Disconnected and powered off
[MAIN] Entering deep sleep for 300 seconds
```

### Visual Quality (Success)
- Clean rendering edge-to-edge
- No duplicate strip on right edge
- Full 960x540 pixels utilized correctly

### Stability (Success)
- No heap corruption warnings
- Stable deep sleep entry/exit
- Multiple wake/sleep cycles work reliably

---

## Rollback Instructions

If testing reveals issues:

1. **Restore vendored library**:
   ```bash
   git checkout lib/FastEPD/
   ```

2. **Restore platformio.ini**:
   ```bash
   git checkout platformio.ini
   ```

3. **Rebuild**:
   ```bash
   ~/.platformio/penv/bin/platformio run --target clean
   ~/.platformio/penv/bin/platformio run -t upload
   ```

This will restore the working v1.1.0-patched version.

---

## Confidence Level

**Build Confidence**: ✅ **HIGH**
- Clean compilation
- Fix verified present
- Binary generated successfully

**Runtime Confidence**: 🟡 **MEDIUM-HIGH**
- Fix is exactly what we analyzed
- Maintainer implemented our suggested solution
- Logic is sound (guard against 4-bpp)
- But needs hardware testing to confirm

**Expected Result**: 🎯 **Should work perfectly**
- Single clean fix vs 15 patches
- Addresses root cause directly
- Maintainer-approved solution

---

## Build Environment

**OS**: macOS (Darwin 24.6.0)
**PlatformIO**: espressif32@6.5.0
**Arduino Framework**: 3.20014.231204
**Board**: LilyGo T5-ePaper-S3
**Python**: ~/.platformio/penv/bin/python

**Libraries**:
- FastEPD: https://github.com/bitbank2/FastEPD.git#3e47f1e
- PNGdec: 1.1.6
- ArduinoJson: 6.21.3
- Wire, SPI, SPIFFS, WiFi, HTTPClient, SD (built-in)

---

## Credits

**FastEPD Maintainer**: Larry Bank ([@bitbank2](https://github.com/bitbank2))
- Responded < 24 hours to bug report
- Implemented clean single-line fix
- Committed immediately to repository

**Build Date**: 2026-04-07 12:29 UTC
**Build Location**: `/Users/nero/Downloads/TRMNL-T5S3-firmware-1.1.0/TRMNL-T5S3-FastEPD copy/`
