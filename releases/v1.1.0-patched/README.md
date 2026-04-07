# Release v1.1.0-patched

**Release Date**: April 6, 2026
**Binary**: `trmnl-t5s3-v1.1.0-patched.bin`
**Size**: 1.2 MB
**Flash Address**: `0x0`

## What's New

✅ **FastEPD Patches Applied** - Clean rendering with no duplicate strip artifact
✅ All 15 buffer calculation fixes for 4-bpp mode
✅ Production-ready firmware with vendored patched library
✅ Improved stability (no heap corruption, stable deep sleep)

## Installation

### Quick Start

1. **Enter bootloader mode**:
   - Press and hold **BOOT** button
   - Plug in USB-C cable while holding BOOT
   - Release **BOOT** button

2. **Flash firmware**:
   ```bash
   esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
       write_flash 0x0 trmnl-t5s3-v1.1.0-patched.bin
   ```

⚠️ **Important**: Must flash to address `0x0` (not 0x1000)

## Changes from v1.1.0

### Fixed
- **Display artifact eliminated**: No more duplicate ~5-10px strip on right edge
- **Heap corruption fixed**: Stable WiFi shutdown and deep sleep
- **Buffer calculations corrected**: All 15 locations updated for 4-bpp mode

### Technical Details
- Applied patches to `lib/FastEPD/src/FastEPD.inl`
- Changed `/4` divisor → `/2` divisor for 4-bpp pixel packing
- Doubled DMA buffer allocation for correct double-buffering

See `lib/FastEPD/PATCHES.md` for complete patch documentation.
See `COMPARISON.md` for visual before/after comparison.

## Compatibility

✅ **1-bpp mode**: Unaffected (different code paths)
✅ **2-bpp mode**: Unaffected (divisor already correct)
✅ **4-bpp mode**: Fixed (was broken, now works correctly)

## Build Information

**Flash Usage**: 54.6% (1,145,005 / 2,097,152 bytes)
**RAM Usage**: 29.7% (97,252 / 327,680 bytes)
**Build Status**: ✅ Successful

## Support

For issues or questions:
- Check main `README.md` for troubleshooting guide
- Review `KNOWN_ISSUES.md` for known limitations
- Open an issue on GitHub

---

**Upgrade from v1.0.0**: Simply flash this binary to 0x0. All WiFi credentials and settings are preserved in NVS storage.
