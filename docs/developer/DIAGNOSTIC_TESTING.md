# Diagnostic Testing Guide - Double Rendering Theory

## Current Hypothesis

The repeated strip on the right edge might be caused by:
1. **Width mismatch**: BB_PANEL_EPDIY_V7 configured for wrong display width (e.g. 800px instead of 960px)
2. **Double rendering**: Image rendered twice with incorrect offset
3. **Background bleed-through**: clearWhite() leaving residue that shows through

## Diagnostic Firmware Features

This build includes diagnostic code to test these theories.

### What to Look For in Serial Output

1. **Display Dimensions Check:**
   ```
   [DIAGNOSTIC] === Display Configuration After setMode() ===
   [DIAGNOSTIC] Display width: ???
   [DIAGNOSTIC] Display height: ???
   [DIAGNOSTIC] Expected buffer size: ??? bytes
   ```

   **Expected values:**
   - Width: 960
   - Height: 540
   - Buffer size: 259,200 bytes (960×540÷2 for 4BPP)

   **If different:** This is likely the root cause! The panel config has wrong dimensions.

2. **Warning Message:**
   ```
   [DIAGNOSTIC] ⚠️  WARNING: Display dimensions don't match expected 960x540!
   [DIAGNOSTIC] This could cause the repeated strip issue!
   ```

   **If you see this:** We found the smoking gun!

## Testing Procedure

### Test 1: Baseline (Current Behavior)

1. Flash firmware using flash script:
   ```bash
   ./flash_firmware.sh /dev/cu.usbmodem14101
   ```

2. Open serial monitor (115200 baud)

3. Wait for device to download and display image

4. **Record:**
   - Display width/height from `[DIAGNOSTIC]` messages
   - Whether repeated strip still appears
   - Serial log output

### Test 2: Skip clearWhite() (Background Bleed Test)

**Purpose:** If repeated strip disappears or changes to black/garbage, it was background bleed-through.

1. Edit `src/display.cpp` line 350-351:
   ```cpp
   // OPTION 1: Normal clear (default)
   // Serial.println("[IMAGE] Clearing display to white...");
   // bbep.clearWhite(true);  // COMMENT THESE OUT

   // OPTION 2: Skip clear (test if background bleed-through is the issue)
   Serial.println("[DIAGNOSTIC] SKIPPING clearWhite() to test double-render theory");
   ```

2. Rebuild and flash:
   ```bash
   ~/.platformio/penv/bin/platformio run
   ./flash_firmware.sh /dev/cu.usbmodem14101
   ```

3. **Record:**
   - Does repeated strip disappear?
   - Does it turn black/garbage?
   - Or does it stay the same?

### Test 3: Black Background (Color Test)

**Purpose:** If strip color changes to match background, it's bleed-through.

1. Edit `src/display.cpp` line 350-358:
   ```cpp
   // OPTION 1: Normal clear (default)
   // Serial.println("[IMAGE] Clearing display to white...");
   // bbep.clearWhite(true);  // COMMENT OUT

   // OPTION 3: Fill with black instead of white (test background color)
   Serial.println("[DIAGNOSTIC] Filling with BLACK instead of white");
   bbep.fillScreen(0x00);  // UNCOMMENT THIS
   ```

2. Rebuild and flash

3. **Record:**
   - Does repeated strip turn black?
   - If YES: background bleed-through confirmed
   - If NO: not a background issue

### Test 4: Fast Refresh Mode

**Purpose:** Test if CLEAR_SLOW mode causes double-rendering artifact.

1. Edit `src/display.cpp` line 368-374:
   ```cpp
   // OPTION 1: CLEAR_SLOW, power off (current default)
   // Serial.println("[IMAGE] Updating display (CLEAR_SLOW, power off)...");
   // bbep.fullUpdate(CLEAR_SLOW, false);  // COMMENT OUT

   // OPTION 2: CLEAR_FAST, power off
   Serial.println("[DIAGNOSTIC] Testing CLEAR_FAST mode...");
   bbep.fullUpdate(CLEAR_FAST, false);  // UNCOMMENT THIS
   ```

2. Rebuild and flash

3. **Record:**
   - Does repeated strip disappear with CLEAR_FAST?
   - Does refresh quality change?

## Expected Results

### Scenario A: Width Mismatch (Most Likely)

**Symptoms:**
- `[DIAGNOSTIC]` shows width ≠ 960 (e.g. 800)
- Repeated strip persists in all tests
- Buffer calculations mismatch

**Conclusion:** BB_PANEL_EPDIY_V7 has wrong panel dimensions configured.

**Solution:** Need to patch FastEPD library or find correct panel config.

### Scenario B: Background Bleed-Through

**Symptoms:**
- Width = 960 (correct)
- Skipping clearWhite() makes strip disappear/change
- Black background makes strip turn black

**Conclusion:** clearWhite() isn't fully clearing, leaving residue.

**Solution:** Call clearWhite() twice, or use fillScreen() directly.

### Scenario C: Refresh Mode Issue

**Symptoms:**
- Width = 960 (correct)
- CLEAR_FAST eliminates repeated strip
- CLEAR_SLOW causes artifact

**Conclusion:** CLEAR_SLOW mode has a bug with two-pass rendering.

**Solution:** Use CLEAR_FAST mode instead.

## Reporting Results

After testing, please report:

1. **Display dimensions from [DIAGNOSTIC] logs**
2. **Which tests were performed**
3. **Results of each test (strip appearance)**
4. **Full serial log output**

This will help determine the root cause and proper fix!

---

**Last Updated:** 2026-04-03
**Firmware Version:** Diagnostic Build
**Purpose:** Root cause analysis of repeated strip issue
