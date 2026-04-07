# GitHub Issue Response - Corrected Version

**For**: https://github.com/bitbank2/FastEPD/issues/29
**Date**: 2026-04-07

---

Thanks for looking into this! Here are answers to your questions:

## "Are you calling bbepBackupPlane()?"

**Short answer**: Not directly, but **yes via clearWhite()**.

I don't call `backupPlane()` or `bbepBackupPlane()` explicitly in my code, but I do call:

```cpp
bbep.clearWhite(true);  // Called before rendering in 4-bpp mode
```

Looking at `FastEPD.cpp`, `clearWhite()` always calls `backupPlane()`:

```cpp
void FASTEPD::clearWhite(bool bKeepOn) {
    backupPlane();  // <-- This is the issue
    // ... rest of clearWhite
}
```

So **yes, I'm triggering `bbepBackupPlane()` in 4-bpp mode via clearWhite()**, which you mentioned shouldn't happen.

---

## Complete API Usage Sequence

Here's exactly what I call:

```cpp
// 1. Hardware initialization
bbep.initPanel(BB_PANEL_EPDIY_V7);
bbep.setPanelSize(960, 540, BB_PANEL_FLAG_NONE);
bbep.setCustomMatrix(u8M5Matrix, sizeof(u8M5Matrix));

// 2. Set 4-bpp mode
bbep.setMode(BB_MODE_4BPP);

// 3. Pre-clear display (THIS CALLS bbepBackupPlane!)
bbep.clearWhite(true);  // <-- Problem is here

// 4. Render PNG via callback
png.decode(NULL, 0);
// Inside callback: bbep.drawPixelFast(x, y, pixel);

// 5. Update display
bbep.fullUpdate(CLEAR_NONE, false);
```

**The issue**: `clearWhite()` calls `backupPlane()` which calls `bbepBackupPlane()`, even in 4-bpp mode.

---

## Why This Causes Problems

In `bbepBackupPlane()` (FastEPD.inl line 1988):

```cpp
void bbepBackupPlane(FASTEPDSTATE *pState) {
    int iSize = (pState->native_width/2) * pState->native_height;
    if (!pState->pPrevious || !pState->pCurrent) return;
    memcpy(pState->pPrevious, pState->pCurrent, iSize);
}
```

**For 960×540 in 4-bpp mode:**
- `iSize = (960/2) * 540 = 259,200 bytes` to copy
- But `pPrevious` was allocated at offset `(960/4) * 540 = 129,600` in buffer allocation
- **Result**: `memcpy()` writes 259,200 bytes into a space that's only sized for 129,600 bytes
- **Heap corruption + visual artifacts**

---

## Hardware Details

- **Device**: LilyGo T5S3 Pro (ESP32-S3, 8MB PSRAM)
- **Display**: 960×540, 4.7" grayscale e-paper
- **Panel**: BB_PANEL_EPDIY_V7
- **Mode**: BB_MODE_4BPP exclusively
- **Custom matrix**: u8M5Matrix (similar to your grayscale_test.ino example)

---

## Symptoms

**Visual**: Duplicate ~5-10px vertical strip on right edge of display

**Serial output**:
```
CORRUPT HEAP: Bad head at 0x3fcb5fa8. Expected 0xabba1234 got 0x3fcb3e80
assertion "heap != NULL && "free() target pointer is outside heap areas"" failed
```

**When**: After rendering, during WiFi shutdown, before deep sleep

---

## Minimal Reproduction

```cpp
#include <FastEPD.h>

FASTEPD bbep;

void setup() {
    Serial.begin(115200);

    // Init for 960x540 display
    bbep.initPanel(BB_PANEL_EPDIY_V7);
    bbep.setPanelSize(960, 540, BB_PANEL_FLAG_NONE);

    // 4-bpp mode
    bbep.setMode(BB_MODE_4BPP);

    // This triggers bbepBackupPlane() in 4-bpp mode
    bbep.clearWhite(true);  // <-- Bug here

    // Draw something
    bbep.fillRect(100, 100, 200, 200, 0x00);

    // Update
    bbep.fullUpdate(CLEAR_NONE, false);
}

void loop() {}
```

---

## Solution

Your suggestion to add a check in `bbepBackupPlane()` would fix it:

```cpp
void bbepBackupPlane(FASTEPDSTATE *pState) {
    // Guard against 4-bpp mode
    if (pState->mode == BB_MODE_4BPP) {
        return;  // Previous buffer not used in 4-bpp
    }

    int iSize = (pState->native_width/2) * pState->native_height;
    if (!pState->pPrevious || !pState->pCurrent) return;
    memcpy(pState->pPrevious, pState->pCurrent, iSize);
}
```

That would prevent `clearWhite()`/`clearBlack()` from corrupting the buffer when called in 4-bpp mode.

---

## Temporary Workaround

I'm currently using a vendored FastEPD with buffer calculation patches, but I'd much rather use the official library once this is fixed!

Let me know if you need any other information or testing.
