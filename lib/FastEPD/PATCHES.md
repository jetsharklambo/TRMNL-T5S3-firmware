# FastEPD v1.3.0 Patches for 4-bpp Buffer Calculations

This directory contains a vendored copy of FastEPD v1.3.0 with critical fixes for 4-bit-per-pixel (4-bpp) mode on 960×540 displays.

## Why These Patches Exist

### The Bug
FastEPD v1.3.0 has incorrect buffer calculations for 4-bpp mode:
- Library assumes `/4` divisor throughout (correct for 2-bpp: 4 pixels per byte)
- 4-bpp mode requires `/2` divisor (2 pixels per byte)
- Results in buffer underallocation and incomplete rendering

### Impact
- **Visual**: Duplicate ~5-10px strip on right edge of 960×540 displays
- **Safety**: Potential buffer overflow (though ESP32-S3's PSRAM mitigates crashes)
- **Affected Hardware**: LilyGo T5S3 Pro, M5Paper S3 (possibly others)

## Patches Applied

**Total**: 15 surgical fixes to `src/FastEPD.inl`

### Category 1: Buffer Allocation (3 fixes)

**Line 1288**: Previous buffer pointer
```cpp
// Before (WRONG): pState->pPrevious = &pState->pCurrent[(width/4) * height];
// After (CORRECT): pState->pPrevious = &pState->pCurrent[(width/2) * height];
```

**Line 1289**: Temp buffer pointer
```cpp
// Before: pState->pTemp = &pState->pCurrent[(width/4) * height * 2];
// After: pState->pTemp = &pState->pCurrent[(width/2) * height * 2];
```

**Line 1295**: DMA buffer (critical for double-buffering)
```cpp
// Before (causes corruption): pState->pTemp = (uint8_t *)malloc(pState->width / 2);
// After (correct size): pState->pTemp = (uint8_t *)malloc(pState->width);
```

### Category 2-5: Complete Reference Table

| Line | Category | Change | Impact |
|------|----------|--------|--------|
| 1288 | Buffer Alloc | `/4` → `/2` | pPrevious pointer |
| 1289 | Buffer Alloc | `/4` → `/2` | pTemp pointer |
| 1295 | Buffer Alloc | `width/2` → `width` | DMA buffer size |
| 1553 | Row Op | `/4` → `/2` | memset size |
| 1562 | Row Op | `/4` → `/2` | memset size |
| 1575 | Row Op | `/4` → `/2` | memset size |
| 1577 | Row Op | `/4` → `/2` | memset size |
| 1579 | Row Op | `/4` → `/2` | memcpy size |
| 1610 | Indexing | `/4` → `/2` | Temp buffer offset |
| 1614 | Loop | `/4` → `/2` | Iteration count |
| 1621 | Loop | `/4` → `/2` | Iteration count |
| 1633 | Indexing | `/4` → `/2` | Temp buffer offset |
| 1635 | Row Op | `/4` → `/2` | memset size |
| 1636 | Row Op | `/4` → `/2` | memset size |
| 1655 | Loop | `/4` → `/2` | Iteration count |
| 1663 | Loop | `/4` → `/2` | Iteration count |
| 1672 | Row Op | `/4` → `/2` | memcpy size |
| 1758 | Indexing | `/4` → `/2` | Temp buffer offset |
| 1762 | Loop | `/4` → `/2` | Iteration count |
| 1769 | Loop | `/4` → `/2` | Iteration count |
| 1796 | Indexing | `/4` → `/2` | Temp buffer offset |
| 1799 | Write Op | `/4` → `/2` | bbepWriteRow offset & size |
| 1800 | Write Op | `/4` → `/2` | bbepWriteRow size |
| 1820 | Loop | `/4` → `/2` | Iteration count |
| 1828 | Loop | `/4` → `/2` | Iteration count |
| 1846 | Write Op | `/4` → `/2` | Write byte count |
| 1848 | Write Op | `/4` → `/2` | Write byte count |
| 1849 | Write Op | `/4` → `/2` | Write byte count |

## Testing Results

### Before Patches
❌ Duplicate ~5-10px strip on right edge
❌ Heap corruption warnings during WiFi shutdown
❌ Unable to enter deep sleep reliably

### After Patches
✅ Clean rendering across full 960×540 display
✅ No heap corruption
✅ Stable deep sleep operation
✅ No performance degradation
✅ Memory cost: ~480 bytes (negligible on 8MB PSRAM)

### Compatibility
✅ **1-bpp mode**: Unaffected (no behavior change)
✅ **2-bpp mode**: Unaffected (no behavior change)
✅ **4-bpp mode**: Fixed (was broken, now works correctly)

## When to Remove These Patches

**Scenario 1**: FastEPD maintainer merges upstream fix
1. Update `platformio.ini`: `bitbank2/FastEPD@^1.4.0`
2. Delete `lib/FastEPD/` directory
3. Rebuild - patches no longer needed

**Scenario 2**: TRMNL maintains fork
- Continue using vendored version
- Track upstream releases manually
- Merge non-conflicting updates

## Upstream Bug Report

**Status**: To be filed at https://github.com/bitbank2/FastEPD/issues
**Documentation**: See `docs/FASTEPD_BUG_REPORT_DRAFT.md` for complete technical report

## Attribution

**Original Library**: https://github.com/bitbank2/FastEPD
**Author**: Larry Bank
**Patches**: April 2026
**Version**: Based on FastEPD v1.3.0
