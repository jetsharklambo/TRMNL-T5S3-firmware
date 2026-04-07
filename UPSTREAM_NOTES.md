# Upstream Integration Notes for TRMNL Repository Maintainers

**Target Repository:** https://github.com/usetrmnl/trmnl-firmware
**Firmware Version:** 1.1.0
**Date:** 2026-04-03
**Status:** Ready for upstream integration

---

## Executive Summary

This firmware version (v1.1.0) fixes **critical rendering artifacts and heap corruption crashes** discovered in the FastEPD library integration. The firmware is stable, tested, and ready for production deployment to TRMNL users.

**Key Changes:**
- ✅ Fixed display rendering artifacts (repeated strip on right edge, missing pixels on left)
- ✅ Fixed heap corruption crash during WiFi shutdown
- ✅ Added 5px horizontal offset for better image centering
- ✅ Comprehensive documentation of all 15 FastEPD library patches

---

## Critical Information: FastEPD Library Patches

### Why Patches Are Required

The upstream **FastEPD library v1.3.0** has buffer calculation bugs in 4BPP (4-bit-per-pixel) mode. These bugs cause:

1. **Rendering artifacts** - Repeated vertical strip on right edge, missing pixels on left
2. **Heap corruption** - "CORRUPT HEAP" crash during WiFi shutdown after image render
3. **Deep sleep failure** - Device cannot enter sleep due to crash

The library incorrectly uses `/4` divisor (for 2BPP mode) instead of `/2` divisor (for 4BPP mode) in 15 locations.

### What Was Patched

**File:** `.pio/libdeps/T5_E_PAPER_S3_V7/FastEPD/src/FastEPD.inl`

**Patches Applied:** 15 fixes across 5 categories:

1. **Buffer allocation** (lines 1288-1289, 1295): Changed `/4` → `/2` for 4BPP pixel packing
2. **DMA buffer** (line 1295): Doubled allocation from `width/2` → `width` for double-buffering
3. **Row operations** (lines 1553, 1562, 1575, 1577, 1579, 1635, 1636, 1672): Fixed memset/memcpy sizes
4. **Indexing** (lines 1610, 1633, 1758, 1796): Fixed temp buffer offset calculations
5. **Loop iterations & writes** (lines 1614, 1621, 1655, 1663, 1762, 1769, 1799, 1800, 1820, 1828, 1846, 1848, 1849): Fixed iteration counts and write operations

**Complete technical details:** See CLAUDE.md section "FastEPD Library Patches"

### Impact of Patches

**Before patches:**
- ❌ Rendering artifacts (repeated strip, missing pixels)
- ❌ Heap corruption crash after image render
- ❌ Cannot enter deep sleep
- ❌ Device unusable for production

**After patches:**
- ✅ Clean rendering across full 960x540 display
- ✅ Stable WiFi shutdown
- ✅ Successful deep sleep entry/exit
- ✅ Production-ready firmware
- ✅ Minimal memory cost (~480 bytes for DMA buffer)

---

## Integration Checklist

### Pre-Integration Review

- [ ] Review CHANGELOG.md for v1.1.0 changes
- [ ] Review CLAUDE.md "FastEPD Library Patches" section
- [ ] Review KNOWN_ISSUES.md (all critical issues resolved)
- [ ] Understand that **FastEPD patches must be preserved** in build environment

### Build Environment Setup

**IMPORTANT:** The patches are applied to the **PlatformIO library cache**, not the project repository. This means:

1. **First build:** PlatformIO downloads FastEPD v1.3.0 → patches must be applied
2. **Clean builds:** Library cache cleared → patches must be reapplied
3. **CI/CD:** Build servers need patch automation

**Recommended approach:**

**Option A: Post-install hook** (recommended for PlatformIO):
```python
# platformio.ini
[env:T5_E_PAPER_S3_V7]
extra_scripts = post:scripts/patch_fastepd.py
```

Create `scripts/patch_fastepd.py` to automatically apply patches after library install.

**Option B: Pre-patched library fork** (alternative):
- Fork bitbank2/FastEPD repository
- Apply patches to fork
- Use fork in platformio.ini: `lib_deps = https://github.com/YOUR_ORG/FastEPD.git#patched-v1.3.0`

**Option C: Manual patching** (not recommended for production):
- Build once to download library
- Manually apply patches to `.pio/libdeps/T5_E_PAPER_S3_V7/FastEPD/src/FastEPD.inl`
- Document process in build instructions

### Testing Requirements

Before merging to main branch:

- [ ] Build firmware successfully
- [ ] Flash to LilyGo T5S3 Pro hardware
- [ ] Verify no rendering artifacts (check right/left edges)
- [ ] Verify WiFi connection and image download
- [ ] Verify successful deep sleep entry
- [ ] Verify device wakes from sleep without crash
- [ ] Test multiple sleep/wake cycles (at least 5)
- [ ] Verify battery telemetry reporting
- [ ] Test OTA update process

### Deployment Considerations

**For end users:**
- No action required - flash v1.1.0 firmware using standard procedure
- Existing v1.0.0 users can upgrade via OTA or manual flash
- No configuration changes needed

**For developers:**
- FastEPD patches **must be applied** in build environment
- Do NOT upgrade FastEPD library without verifying upstream fix
- See CONTRIBUTING.md for development guidelines

---

## Known Limitations & Future Work

### Minor Known Issues

**I2C Bus Conflicts** (low priority):
- PCA9555 I/O expander and BQ27220 fuel gauge share I2C bus
- Occasional spurious button events or invalid battery readings
- Mitigation: Retry logic, fallback values, event filtering
- Impact: Does not affect core functionality

### Future Improvements

**Upstream FastEPD library fix** (long-term):
- Submit patches to bitbank2/FastEPD maintainer
- If accepted, remove local patches and use upstream version
- Timeline: Unknown (library maintainer responsiveness varies)

**Rendering optimization**:
- 5 rendering modes implemented (RENDER_MODE 1-5)
- Mode 1 (default) provides fastest rendering with minimal flashing
- Users can experiment with other modes in display.cpp

---

## Comparison Variant (Testing/Verification)

A **comparison variant** using unpatched public FastEPD library is included in the release:

**Location:** `../TRMNL-T5S3-FastEPD-Public/`

**Purpose:**
- Demonstrate bugs present in upstream FastEPD library
- Verify necessity of patches
- Side-by-side comparison for testing

**Expected behavior:**
- Rendering artifacts visible
- Heap corruption crash after image render
- Device cannot enter deep sleep

**Use case:**
- New contributors who want to understand the bug
- QA testing to verify patches are effective
- Documentation/screenshots of before/after

See VARIANTS.md in parent directory for detailed comparison.

---

## Documentation Files

### Primary Documentation
- **CLAUDE.md** - Complete developer guide with architecture, build commands, and technical details
- **CHANGELOG.md** - Version history with v1.1.0 changes
- **KNOWN_ISSUES.md** - All critical issues marked as resolved
- **README.md** - User installation guide

### Supporting Documentation
- **UPSTREAM_NOTES.md** - This file
- **CONTRIBUTING.md** - Guidelines for future contributors
- **VARIANTS.md** - Comparison between patched and unpatched variants
- **DIAGNOSTIC_TESTING.md** - Display diagnostic procedures
- **FLASH_INSTRUCTIONS.md** - User flashing guide

---

## Contact & Support

For questions about this integration:

1. **Technical issues:** Review CLAUDE.md and KNOWN_ISSUES.md first
2. **Build problems:** Check that FastEPD patches are applied correctly
3. **Rendering artifacts:** Verify patches are present in FastEPD.inl
4. **Crash on sleep:** Verify DMA buffer patch (line 1295) is applied

---

## Release Artifacts

### Firmware Binaries (v1.1.0)
- `.pio/build/T5_E_PAPER_S3_V7/firmware.bin` - Main firmware
- `.pio/build/T5_E_PAPER_S3_V7/bootloader.bin` - ESP32 bootloader
- `.pio/build/T5_E_PAPER_S3_V7/partitions.bin` - Partition table

### Checksums
- MD5 checksums provided in release notes
- Verify integrity before deployment

### Build Information
- **Platform:** ESP32-S3 (espressif32@6.5.0)
- **Framework:** Arduino
- **FastEPD:** v1.3.0 (locally patched)
- **PNGdec:** v1.1.6
- **ArduinoJson:** v6.21.3

---

## Approval & Sign-off

This firmware has been:
- ✅ Thoroughly tested on live hardware
- ✅ All critical bugs resolved
- ✅ Documentation completed
- ✅ Build artifacts generated
- ✅ Comparison variant created for verification

**Status:** Ready for upstream integration to https://github.com/usetrmnl/trmnl-firmware

**Recommended branch strategy:**
1. Merge v1.1.0 to `main` branch
2. Tag release as `v1.1.0`
3. Update GitHub releases with binaries and checksums
4. Document FastEPD patch requirement in main README.md

---

**Last Updated:** 2026-04-03
**Firmware Version:** 1.1.0
**FastEPD Version:** 1.3.0 (locally patched)
