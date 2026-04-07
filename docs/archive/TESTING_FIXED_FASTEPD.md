# Testing FastEPD Fix (Commit 3e47f1e)

**Date**: 2026-04-07
**Commit**: https://github.com/bitbank2/FastEPD/commit/3e47f1e3ab6d5e00a979e1c32557bb3bc89e8653
**Fix**: Added guard in `bbepBackupPlane()`: `if (pState->mode == BB_MODE_4BPP) return;`

---

## What The Fix Does

**FastEPD.inl - bbepBackupPlane():**
```cpp
void bbepBackupPlane(FASTEPDSTATE *pState) {
    if (pState->mode == BB_MODE_4BPP) return;  // <-- NEW: Guard against 4-bpp

    int iSize = (pState->native_width/2) * pState->native_height;
    if (!pState->pPrevious || !pState->pCurrent) return;
    memcpy(pState->pPrevious, pState->pCurrent, iSize);
}
```

**Impact**:
- When we call `clearWhite(true)` in 4-bpp mode, it no longer corrupts the buffer
- Should eliminate the duplicate strip artifact
- Should eliminate heap corruption warnings

---

## Testing Plan

### Option 1: Update Library Reference (Easiest)

**Steps:**
1. Update `platformio.ini` to use the latest FastEPD from GitHub
2. Remove vendored `lib/FastEPD/` directory
3. Rebuild and flash
4. Test on hardware

**platformio.ini change:**
```ini
lib_deps =
    https://github.com/bitbank2/FastEPD.git#3e47f1e3ab6d5e00a979e1c32557bb3bc89e8653
    bitbank2/PNGdec@^1.1.6
    bblanchon/ArduinoJson@^6.21.3
```

### Option 2: Manual Patch (Keep Vendored Copy)

**Steps:**
1. Keep existing `lib/FastEPD/` directory
2. Apply just the one-line fix to `lib/FastEPD/src/FastEPD.inl`
3. Remove our 15 other patches (revert to original, then apply just this one)
4. Rebuild and flash
5. Test on hardware

### Option 3: Fresh Clone (Most Thorough)

**Steps:**
1. Clone FastEPD repository at commit 3e47f1e
2. Replace entire `lib/FastEPD/` directory with fresh copy
3. Rebuild and flash
4. Test on hardware

---

## Recommended Approach: Option 1

Use PlatformIO's git reference to pull the exact commit directly.

### Step-by-Step Instructions

**1. Update platformio.ini**
```bash
cd /Users/nero/Downloads/TRMNL-T5S3-firmware-1.1.0/TRMNL-T5S3-FastEPD
```

Edit `platformio.ini`, find the `lib_deps` section and change to:
```ini
lib_deps =
    https://github.com/bitbank2/FastEPD.git#3e47f1e3ab6d5e00a979e1c32557bb3bc89e8653
    bitbank2/PNGdec@^1.1.6
    bblanchon/ArduinoJson@^6.21.3
```

**2. Remove vendored library**
```bash
rm -rf lib/FastEPD
```

**3. Clean build**
```bash
~/.platformio/penv/bin/platformio run --target clean
```

**4. Rebuild with new library**
```bash
~/.platformio/penv/bin/platformio run
```

**5. Flash to device**
```bash
~/.platformio/penv/bin/platformio run -t upload
```

**6. Monitor serial output**
```bash
~/.platformio/penv/bin/platformio device monitor
```

---

## What To Test

### Visual Tests

**Primary Test**: Check for duplicate strip artifact
- [ ] Flash firmware with fixed FastEPD
- [ ] Let device complete full boot cycle
- [ ] Download and display an image from TRMNL API
- [ ] Visually inspect right edge of display
- [ ] **Expected**: No duplicate strip (clean rendering edge-to-edge)
- [ ] **Previous**: ~5-10px duplicate strip on right edge

**Secondary Test**: Multiple render cycles
- [ ] Force refresh multiple times (press button)
- [ ] Check for artifacts after each refresh
- [ ] **Expected**: Consistent clean rendering

### Serial Output Tests

**Heap Corruption Check**:
- [ ] Watch serial output during full device cycle
- [ ] Pay attention during WiFi shutdown
- [ ] Before entering deep sleep
- [ ] **Expected**: No "CORRUPT HEAP" messages
- [ ] **Previous**: `CORRUPT HEAP: Bad head at 0x3fcb5fa8`

**Diagnostic Output**:
```
[IMAGE] Decoding PNG...
[IMAGE] Display update complete!
[WIFI] Disconnecting WiFi...
[WIFI] Disconnected and powered off
[SHUTDOWN] Waiting X seconds before sleep...
[MAIN] Entering deep sleep for 300 seconds
```

**Expected**: Clean output, no corruption warnings, successful deep sleep entry

### Functionality Tests

**Deep Sleep Stability**:
- [ ] Device enters deep sleep successfully
- [ ] Device wakes on timer (e.g., 5 minutes)
- [ ] Device wakes on button press
- [ ] **Expected**: Stable sleep/wake cycles
- [ ] **Previous**: Occasional failures entering sleep

**Memory Stability**:
- [ ] Check free heap before rendering
- [ ] Check free heap after rendering
- [ ] Check for memory leaks over multiple cycles
- [ ] **Expected**: Consistent memory usage

**Full Lifecycle**:
- [ ] WiFi captive portal (if no credentials)
- [ ] Device registration (/api/setup)
- [ ] Image download (/api/display)
- [ ] Display rendering
- [ ] Deep sleep
- [ ] Wake and refresh
- [ ] **Expected**: All features work without crashes

---

## Success Criteria

### Must Pass (Critical)
✅ **No visual artifact**: Right edge renders cleanly, no duplicate strip
✅ **No heap corruption**: Clean serial output during WiFi shutdown
✅ **Stable deep sleep**: Device enters and exits sleep reliably

### Should Pass (Important)
✅ **Memory stable**: No leaks over multiple cycles
✅ **Full functionality**: All TRMNL features work (WiFi, API, OTA, logging)
✅ **Performance**: No degradation vs vendored patched version

### Nice to Have
✅ **Render quality**: Image quality matches previous version
✅ **Battery life**: No increase in power consumption
✅ **Build clean**: No compilation warnings

---

## Comparison Test (Bonus)

### Side-by-Side Comparison

**Device A**: Keep vendored patched FastEPD (current working version)
**Device B**: Use fixed upstream FastEPD (commit 3e47f1e)

Test both devices with same image and compare:
- Visual quality
- Serial output
- Memory usage
- Stability over 24 hours

**Expected**: Both should perform identically (proving the upstream fix works)

---

## If Tests Pass

### Next Steps (Phase 5)

1. **Update documentation**:
   - Remove references to "vendored library"
   - Remove PATCHES.md
   - Archive COMPARISON.md
   - Update CLAUDE.md

2. **Clean up codebase**:
   - Keep using git reference or switch to version tag when released
   - Remove all patch documentation
   - Update README

3. **Prepare TRMNL integration**:
   - Sync with latest TRMNL firmware
   - Submit PR with clean FastEPD dependency
   - Document T5S3 support

4. **Thank maintainer**:
   - Comment on GitHub issue confirming fix works
   - Offer to test future releases

---

## If Tests Fail

### Debug Steps

1. **Check library version**:
   ```bash
   cat .pio/libdeps/T5_E_PAPER_S3_V7/FastEPD/.git/HEAD
   ```
   Should show commit 3e47f1e or later

2. **Verify guard is present**:
   ```bash
   grep -A2 "bbepBackupPlane" .pio/libdeps/T5_E_PAPER_S3_V7/FastEPD/src/FastEPD.inl
   ```
   Should show: `if (pState->mode == BB_MODE_4BPP) return;`

3. **Check build output**:
   Look for library path in compilation
   ```
   Compiling .pio/build/.../FastEPD/...
   ```

4. **Enable debug logging**:
   Add to display.cpp before clearWhite():
   ```cpp
   Serial.println("[DEBUG] About to call clearWhite in 4-bpp mode");
   ```

5. **Compare builds**:
   - Build with vendored (working) → note firmware.bin size
   - Build with upstream (testing) → note firmware.bin size
   - Sizes should be similar

---

## Rollback Plan

If the upstream fix doesn't work:

1. **Restore vendored library**:
   ```bash
   git checkout lib/FastEPD/
   ```

2. **Restore platformio.ini**:
   ```ini
   lib_deps =
       bitbank2/PNGdec@^1.1.6
       bblanchon/ArduinoJson@^6.21.3
   # FastEPD is vendored in lib/
   ```

3. **Rebuild and reflash**:
   ```bash
   ~/.platformio/penv/bin/platformio run --target clean
   ~/.platformio/penv/bin/platformio run -t upload
   ```

4. **Report findings**:
   - Comment on GitHub issue
   - Provide serial logs and photos
   - Work with maintainer on additional fixes

---

## Testing Checklist

### Pre-Test Setup
- [ ] Backup current working firmware
- [ ] Note current firmware.bin location
- [ ] Have USB cable ready for serial monitoring
- [ ] Have camera ready for visual comparison photos

### Build Steps
- [ ] Update platformio.ini with git reference
- [ ] Remove lib/FastEPD directory
- [ ] Clean build environment
- [ ] Rebuild firmware
- [ ] Verify build succeeds
- [ ] Flash to device

### Runtime Tests
- [ ] Device boots successfully
- [ ] Display initializes
- [ ] Image renders cleanly (no right-edge artifact)
- [ ] No heap corruption in serial output
- [ ] WiFi connects and disconnects cleanly
- [ ] Device enters deep sleep
- [ ] Device wakes on timer
- [ ] Device wakes on button press
- [ ] Full lifecycle test (registration → display → sleep)

### Documentation
- [ ] Take photos of display (before/after comparison)
- [ ] Capture serial logs (clean vs corrupted)
- [ ] Note any differences in behavior
- [ ] Measure memory usage
- [ ] Test over 24-hour period

### Reporting
- [ ] Comment on GitHub issue with results
- [ ] Thank maintainer for quick fix
- [ ] Provide test data (photos, logs)
- [ ] Confirm moving to official library

---

**Recommendation**: Start with **Option 1** (git reference in platformio.ini). It's the cleanest approach and easiest to rollback if needed.

**Timeline**: Testing should take ~2-4 hours including build, flash, and multiple test cycles.
