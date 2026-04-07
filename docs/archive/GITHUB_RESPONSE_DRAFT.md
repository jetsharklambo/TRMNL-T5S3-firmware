# GitHub Issue Response Draft

**For**: https://github.com/bitbank2/FastEPD/issues/29
**Date**: 2026-04-07

---

## Response to Maintainer's Questions

Thanks for looking into this! Here are answers to your questions:

### Q: "Are you calling bbepBackupPlane()?"

**Answer: NO**, we do not call `bbepBackupPlane()` or `backupPlane()` anywhere in our code.

I searched the entire codebase:
```bash
grep -r "bbepBackupPlane" src/
grep -r "bbepBackupPlane" include/
# Result: NO MATCHES
```

### Q: "Can you share your test sketch in its entirety?"

**Our complete API usage sequence:**

```cpp
// Hardware initialization
bbep.initPanel(BB_PANEL_EPDIY_V7);
bbep.setPanelSize(960, 540, BB_PANEL_FLAG_NONE);
bbep.setCustomMatrix(u8M5Matrix, sizeof(u8M5Matrix));

// Rendering sequence
bbep.setMode(BB_MODE_4BPP);
bbep.clearWhite(true);  // bKeepOn = true

// PNG decode - callback writes pixels
png.decode(NULL, 0);
// Inside callback: bbep.drawPixelFast(x + IMAGE_X_OFFSET, y, pixel);

// Transfer to display
bbep.fullUpdate(CLEAR_NONE, false);  // iClearMode=CLEAR_NONE, bKeepOn=false
```

**API calls we make:**
- `initPanel(BB_PANEL_EPDIY_V7)`
- `setPanelSize(960, 540, BB_PANEL_FLAG_NONE)`
- `setCustomMatrix(u8M5Matrix, sizeof(u8M5Matrix))`
- `setMode(BB_MODE_4BPP)`
- `clearWhite(true)`
- `drawPixelFast(x, y, pixel)` (thousands of times during PNG decode)
- `fullUpdate(CLEAR_NONE, false)`

**API calls we do NOT make:**
- ❌ `backupPlane()`
- ❌ `bbepBackupPlane()`
- ❌ `partialUpdate()`
- ❌ Any differential update functions

---

## What We Think We Found

After reviewing your comment about pPrevious buffer only being used in 1/2-bpp mode, I examined the `bbepFullUpdate()` code in FastEPD.inl and confirmed you're correct:

**Line 1759 (1-bpp mode):**
```cpp
memcpy(&pState->pPrevious[i * (pState->native_width/8)], s, pState->native_width / 8);
// ^^^ Accesses pPrevious
```

**Line 1804+ (4-bpp mode):**
```cpp
// NO access to pPrevious buffer at all!
```

So 4-bpp mode never touches `pPrevious`. **However**, I think the buffer allocation code still allocates it incorrectly.

### The Issue: Buffer Allocation (Lines 1288-1295)

```cpp
// Line 1288
pState->pPrevious = &pState->pCurrent[(width/4) * height];

// Line 1289
pState->pTemp = &pState->pCurrent[(width/4) * height * 2];

// Line 1295
pState->pTemp = (uint8_t *)malloc(pState->width / 2);
```

**Problem**: These calculations use `/4` divisor, which is correct for 2-bpp (4 pixels per byte) but wrong for 4-bpp (2 pixels per byte).

### Memory Layout (960x540 in 4-bpp mode)

**Unpatched (causes issues):**
```
pCurrent buffer:  [0 -------- 259,200 bytes total --------->]
pPrevious pointer:          ^--- Points at 129,600 (width/4 * height)
pTemp pointer:                           ^--- Points at 259,200 (width/4 * height * 2)
```

**The problem**: `pPrevious` points to offset 129,600, which is halfway through where `pCurrent` should be, causing `pTemp` to point beyond the allocated buffer.

Even though 4-bpp mode never *uses* `pPrevious`, the incorrect pointer arithmetic causes `pTemp` (the DMA buffer) to be misaligned, resulting in:
1. **Visual artifact**: Duplicate ~5-10px strip on right edge
2. **Heap corruption**: `CORRUPT HEAP: Bad head at 0x3fcb5fa8`
3. **Instability**: Occasional failures entering deep sleep

---

## Proposed Fixes

### Option A: Mode-Specific Buffer Allocation (Cleanest)

```cpp
if (pState->mode == BB_MODE_1BPP || pState->mode == BB_MODE_2BPP) {
    // 1/2-bpp modes: need pPrevious for differential updates
    pState->pPrevious = &pState->pCurrent[(width/4) * height];
    pState->pTemp = &pState->pCurrent[(width/4) * height * 2];
} else { // BB_MODE_4BPP
    // 4-bpp mode: pPrevious not used, don't allocate
    pState->pPrevious = NULL;
    pState->pTemp = &pState->pCurrent[(width/2) * height];
}

// DMA buffer allocation (line 1295)
if (pState->mode == BB_MODE_4BPP) {
    pState->pTemp = (uint8_t *)malloc(pState->width);  // 2 pixels per byte
} else {
    pState->pTemp = (uint8_t *)malloc(pState->width / 2);  // 4 pixels per byte
}
```

### Option B: Mode-Aware Divisor

```cpp
int divisor = (pState->mode == BB_MODE_4BPP) ? 2 : 4;
pState->pPrevious = &pState->pCurrent[(width/divisor) * height];
pState->pTemp = &pState->pCurrent[(width/divisor) * height * 2];
pState->pTemp = (uint8_t *)malloc(pState->width / divisor);
```

### Option C: Add Safety Check in bbepBackupPlane()

```cpp
void bbepBackupPlane(FASTEPDSTATE *pState) {
    if (pState->mode == BB_MODE_4BPP) {
        #ifdef ARDUINO
        Serial.println("ERROR: bbepBackupPlane() called in 4-bpp mode (not supported)");
        #endif
        return;
    }
    // ... rest of function
}
```

**Note**: Option C alone won't fix the buffer allocation issue, but would prevent misuse.

---

## Hardware Details

**Device**: LilyGo T5S3 Pro
**Display**: 960x540, 4.7" grayscale e-paper
**Panel**: BB_PANEL_EPDIY_V7
**MCU**: ESP32-S3 (8MB PSRAM)
**Mode**: BB_MODE_4BPP exclusively

**Custom gray matrix**: We use `u8M5Matrix` optimized for 960x540 M5Paper-style displays (similar to the one in your `grayscale_test.ino` example).

---

## Minimal Reproduction Sketch

I can provide a complete standalone Arduino sketch if that would help, but the core issue appears to be in the buffer allocation logic rather than our API usage pattern.

Would you like me to create a minimal `.ino` file that reproduces the issue?

---

## Our Temporary Workaround

We currently maintain a vendored copy of FastEPD v1.3.0 with 15 patches changing `/4` to `/2` in buffer calculations throughout FastEPD.inl. This fixes the visual artifact and heap corruption, but we'd much rather use the official library once this is fixed upstream!

See our patch documentation: [PATCHES.md reference]

---

## Summary

1. ✅ **We do NOT call `bbepBackupPlane()`**
2. ✅ **4-bpp mode never accesses `pPrevious` buffer** (confirmed in code)
3. ❌ **But buffer allocation still uses `/4` divisor** (wrong for 4-bpp)
4. ❌ **This causes `pTemp` (DMA buffer) to be misaligned**
5. ❌ **Result: Visual artifacts + heap corruption**

The fix likely involves making buffer allocation mode-aware (Option A or B above).

Let me know if you need more information or a complete test sketch!
