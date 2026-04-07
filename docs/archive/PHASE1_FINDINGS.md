# Phase 1 Findings: FastEPD 4-bpp Investigation

**Date**: 2026-04-07
**Purpose**: Answer FastEPD maintainer's question: "Are you calling bbepBackupPlane()?"

---

## Key Finding: We Do NOT Call bbepBackupPlane()

**Answer to maintainer's question**: **NO**, we do not call `bbepBackupPlane()` anywhere in our code.

### Search Results
```bash
# Searched entire src/ and include/ directories
grep -r "bbepBackupPlane" src/
grep -r "bbepBackupPlane" include/
# Result: NO MATCHES
```

**Conclusion**: We never explicitly call `backupPlane()` or `bbepBackupPlane()`.

---

## Critical Discovery: fullUpdate() Uses pPrevious Buffer in 1-bpp Mode Only

### Analysis of FastEPD.inl Line 1759

In `bbepFullUpdate()`, there is **mode-specific behavior**:

**1-bpp mode (lines 1749-1803)**:
```cpp
memcpy(&pState->pPrevious[i * (pState->native_width/8)], s, pState->native_width / 8);
// ^^^ ACCESSES pPrevious buffer
```

**4-bpp mode (lines 1804-1854)**:
```cpp
// NO access to pPrevious buffer at all!
// Works directly with pCurrent buffer only
```

**Maintainer's statement confirmed**: *"The previous buffer is only for doing differential updates in 1 and 2-bit mode; 4-bit mode shouldn't access this pointer."*

This is correct! The 4-bpp code path never touches `pPrevious`.

---

## Our Complete FastEPD API Usage Sequence

### Hardware Initialization (display.cpp:225-245)

```cpp
// 1. Initialize panel (sets GPIO pins, panel type)
bbep.initPanel(BB_PANEL_EPDIY_V7);

// 2. Set panel dimensions
bbep.setPanelSize(960, 540, BB_PANEL_FLAG_NONE);

// 3. Override default gray scale matrix
bbep.setCustomMatrix(u8M5Matrix, sizeof(u8M5Matrix));
```

### Rendering Sequence (display.cpp:358-459)

```cpp
// 4. Force 4-bpp mode
bbep.setMode(BB_MODE_4BPP);

// 5. Pre-clear display to white (keep power on)
bbep.clearWhite(true);  // bKeepOn = true

// 6. Decode PNG - callback writes pixels via drawPixelFast()
int decode_rc = png.decode(NULL, 0);
// Inside png_draw_callback (display.cpp:140-211):
//   bbep.drawPixelFast(x + IMAGE_X_OFFSET, y, pixel);

// 7. Transfer framebuffer to display
bbep.fullUpdate(CLEAR_NONE, false);  // iClearMode=CLEAR_NONE, bKeepOn=false
```

### API Calls We Make (Complete List)

| Function | Purpose | Triggers pPrevious? |
|----------|---------|---------------------|
| `initPanel(BB_PANEL_EPDIY_V7)` | GPIO/panel setup | No |
| `setPanelSize(960, 540, BB_PANEL_FLAG_NONE)` | Set dimensions | No |
| `setCustomMatrix(u8M5Matrix, size)` | Override gray matrix | No |
| `setMode(BB_MODE_4BPP)` | Enable 4-bpp mode | No |
| `clearWhite(true)` | Pre-clear display | **Need to check** |
| `drawPixelFast(x, y, pixel)` | Write pixel to framebuffer | No |
| `fullUpdate(CLEAR_NONE, false)` | Transfer to display | **No (in 4-bpp mode)** |

### API Calls We Do NOT Make

- ❌ `backupPlane()` - Never called
- ❌ `bbepBackupPlane()` - Never called
- ❌ `partialUpdate()` - Never called (would need pPrevious)
- ❌ Any differential update APIs

---

## Hypothesis: Where Does pPrevious Get Allocated?

### Buffer Allocation Code (FastEPD.inl lines 1288-1295)

```cpp
// Line 1288 (PATCHED: /4 -> /2)
pState->pPrevious = &pState->pCurrent[(width/4) * height];

// Line 1289 (PATCHED: /4 -> /2)
pState->pTemp = &pState->pCurrent[(width/4) * height * 2];

// Line 1295 (PATCHED: width/2 -> width)
pState->pTemp = (uint8_t *)malloc(pState->width);
```

**The Issue**: `pPrevious` buffer is allocated even in 4-bpp mode, even though it's never used!

### Our Patches "Fixed" The Wrong Thing

We patched buffer calculations assuming `pPrevious` was being used in 4-bpp mode. But it's **not** used - the allocation is just wasting memory and potentially causing corruption.

**Question for maintainer**: Should `pPrevious` buffer **not be allocated at all** in 4-bpp mode?

---

## The Real Problem: Buffer Allocation Calculation

### Unpatched Code (WRONG)
```cpp
// Assumes 2-bpp: 4 pixels per byte
pState->pPrevious = &pState->pCurrent[(width/4) * height];
```

For 960x540 display in 4-bpp mode:
- Calculated size: `(960/4) * 540 = 129,600 bytes`
- Actual needed: `(960/2) * 540 = 259,200 bytes` (if it were used)
- **Result**: pPrevious pointer points into middle of where pTemp should be

### Visual Memory Layout (Unpatched)

```
pCurrent:   [0 -------- 129,600 bytes -------- 259,200]
pPrevious:              ^--- Points here (WRONG!)
pTemp:                              ^--- Should start here
```

This causes `pPrevious` to overlap with `pTemp`, corrupting the DMA buffer!

---

## Question for FastEPD Maintainer

We think we've identified the issue:

1. **pPrevious buffer is allocated even in 4-bpp mode** (lines 1288-1289)
2. **The allocation uses `/4` divisor** (correct for 2-bpp, wrong for 4-bpp)
3. **4-bpp mode never uses pPrevious** (confirmed in fullUpdate code)
4. **But the wrong allocation size causes pPrevious to overlap pTemp**
5. **This corrupts the DMA buffer, causing visual artifacts and heap corruption**

### Proposed Fix Options

**Option A**: Don't allocate pPrevious in 4-bpp mode at all
```cpp
if (pState->mode == BB_MODE_1BPP || pState->mode == BB_MODE_2BPP) {
    pState->pPrevious = &pState->pCurrent[(width/4) * height];
    pState->pTemp = &pState->pCurrent[(width/4) * height * 2];
} else { // 4-bpp mode
    pState->pPrevious = NULL; // Not used in 4-bpp
    pState->pTemp = &pState->pCurrent[(width/2) * height]; // Correct offset
}
```

**Option B**: Use correct divisor for buffer size calculation
```cpp
int divisor = (pState->mode == BB_MODE_4BPP) ? 2 : 4;
pState->pPrevious = &pState->pCurrent[(width/divisor) * height];
pState->pTemp = &pState->pCurrent[(width/divisor) * height * 2];
```

**Option C**: Add check in bbepBackupPlane() (maintainer mentioned this)
```cpp
void bbepBackupPlane(FASTEPDSTATE *pState) {
    if (pState->mode == BB_MODE_4BPP) {
        // ERROR: Cannot backup plane in 4-bpp mode
        return;
    }
    // ... rest of function
}
```

---

## Hardware Specifications

### Device: LilyGo T5S3 Pro (ESP32-S3)

**Display**:
- Resolution: 960 x 540 pixels
- Type: 4.7" grayscale e-paper
- Panel: BB_PANEL_EPDIY_V7
- Orientation: Landscape
- Pixel format: 4-bpp (16 grayscale levels)

**MCU**:
- Chip: ESP32-S3
- PSRAM: 8MB (external)
- Flash: 16MB
- Framework: Arduino + PlatformIO

**E-Paper Interface**:
- Controller: EPDiy V7 (parallel bus)
- Power management: Integrated
- Update modes: Full refresh only (we don't use partial)

**Custom Configuration**:
- Gray scale matrix: `u8M5Matrix` (custom for 960x540 M5Paper-style displays)
- Horizontal offset: 5px (`IMAGE_X_OFFSET`) for bezel centering
- Render mode: `BB_MODE_4BPP` exclusively

---

## Visual Artifact Description

### Symptom (Unpatched FastEPD v1.3.0)

**Duplicate vertical strip on right edge (~5-10 pixels wide)**

```
+-----------------------------------------------+
|                                               |
|         Main Content Area (Clean)             |
|                                               |
|                                           ||| | <- Duplicate strip
+-----------------------------------------------+
                                          ↑
                               ~5-10px repeated content
```

**Characteristics**:
- Strip contains vertically repeated content from earlier in framebuffer
- Width varies slightly (5-10px depending on image)
- Appears consistently on ALL images rendered in 4-bpp mode
- Main content area (95%) renders correctly

### Heap Corruption Warnings (Serial Output)

```
[WIFI] Disconnecting WiFi...
CORRUPT HEAP: Bad head at 0x3fcb5fa8. Expected 0xabba1234 got 0x3fcb3e80
assertion "heap != NULL && "free() target pointer is outside heap areas"" failed: file "/path/to/heap/heap_caps.c", line 360, function: heap_caps_free
```

**When it occurs**:
- After display rendering completes
- During WiFi shutdown sequence
- Before entering deep sleep

**Impact**:
- Device sometimes fails to enter deep sleep
- Occasional soft reboot required
- ESP32-S3's 8MB PSRAM mitigates crashes (smaller devices might crash)

---

## Next Steps

1. ✅ **Answered**: We do NOT call `bbepBackupPlane()`
2. ✅ **Documented**: Complete API usage sequence
3. ✅ **Identified**: Buffer allocation issue with pPrevious/pTemp overlap
4. 🔲 **Create**: Minimal reproduction sketch (standalone .ino)
5. 🔲 **Submit**: Findings to FastEPD GitHub issue
6. 🔲 **Wait**: For maintainer's diagnosis and proper fix
7. 🔲 **Remove**: Vendored patches once upstream is fixed

---

**Key Takeaway**: The maintainer was right - we're not calling `bbepBackupPlane()`, but the buffer allocation code doesn't account for 4-bpp mode and allocates `pPrevious` with the wrong size (using `/4` divisor instead of `/2`), causing buffer overlap and corruption.
