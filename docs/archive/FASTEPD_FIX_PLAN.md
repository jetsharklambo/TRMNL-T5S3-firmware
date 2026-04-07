# Plan: Fix FastEPD 4-bpp Issue & Push T5S3 Support to TRMNL

**Status**: Phase 1 - In Progress
**Created**: 2026-04-07
**Last Updated**: 2026-04-07

## Context Summary

### FastEPD Maintainer's Responses

**Response on TRMNL Firmware Thread**:
> "I will address the FastEPD problem you found. This should definitely not be a patch to TRMNL, but an issue to solve in FastEPD. The LilyGo T5S3 Pro is already supported in the TRMNL firmware and needs some TLC to get it up to date with the latest changes. We don't want to create a private fork of FastEPD in TRMNL. This is backwards progression. We already removed the private fork of bb_epaper not too long ago. It's best to work from the original library in its original repo. BYODs of "X" class are a work in progress and all FastEPD supported devices will eventually get full support in TRMNL firmware."

**Response on FastEPD Thread**:
> "This problem is unrelated to memory allocation size or incorrect pitch calculation. The previous buffer is only for doing differential updates in 1 and 2-bit mode; 4-bit mode shouldn't access this pointer. Can you share your test sketch in its entirety? I don't see anywhere in the code which improperly writes to that pointer in 4-bit mode. Are you calling bbepBackupPlane()? I need to add a check in that function to make sure it's not called in 4-bit mode."

### Current State
- We have 15 patches applied to vendored FastEPD v1.3.0
- Patches change `/4` divisor to `/2` divisor in buffer calculations
- Visual symptom: Duplicate ~5-10px strip on right edge in 4-bpp mode
- Heap corruption warnings during WiFi shutdown
- FastEPD maintainer wants NO vendored library in TRMNL
- FastEPD maintainer suspects we're calling `bbepBackupPlane()` or triggering previous buffer access

### Root Cause Analysis
The FastEPD maintainer's insight is critical: **"The previous buffer is only for doing differential updates in 1 and 2-bit mode; 4-bit mode shouldn't access this pointer."**

This suggests our 15 patches may have masked the real issue rather than fixing it. We need to:
1. **Determine if we're calling `bbepBackupPlane()` or triggering previous buffer access**
2. **Provide complete test sketch** to FastEPD maintainer
3. **Get proper upstream fix** instead of maintaining vendored patches

---

## Phase 1: Create Minimal Reproduction for FastEPD Maintainer

**Goal**: Provide complete test sketch showing the issue (maintainer specifically requested this)

**Status**: 🟡 In Progress

### Tasks
- [ ] **CRITICAL**: Search codebase for ANY calls to `bbepBackupPlane()` (maintainer specifically asked about this)
- [ ] Review our exact API usage sequence in display.cpp
- [ ] Create standalone Arduino sketch demonstrating the issue:
  - LilyGo T5S3 Pro hardware specs
  - BB_PANEL_EPDIY_V7 initialization
  - 960x540 panel setup with custom `u8M5Matrix`
  - 4-bpp mode rendering
  - PNG decode callback using `drawPixelFast()`
  - Document exact sequence of FastEPD API calls
- [ ] Capture serial output showing heap corruption warnings
- [ ] Document visual artifact (duplicate strip on right edge)
- [ ] Package as complete response to maintainer's request

### Key Questions to Answer (Maintainer Asked These)
1. **Do we call `bbepBackupPlane()` anywhere?** (Maintainer specifically asked this)
2. **What does `fullUpdate()` do internally in 4-bpp mode?** (May auto-call backup)
3. **Does `clearWhite()` trigger previous buffer access?**
4. **Are we using any API that implicitly needs differential updates?**

### Deliverables
- [ ] Minimal reproduction sketch (standalone .ino file)
- [ ] Hardware specification document
- [ ] Serial output log showing errors
- [ ] Visual documentation of rendering artifact
- [ ] Complete response to maintainer's GitHub issue

---

## Phase 2: Investigate Our Usage Pattern

**Goal**: Determine if we're misusing FastEPD API

**Status**: ⚪ Not Started

### Tasks
- [ ] Map our complete rendering flow from display.cpp:224-481
- [ ] Identify all FastEPD API calls we make:
  - `initPanel()`
  - `setPanelSize()`
  - `setCustomMatrix()`
  - `setMode(BB_MODE_4BPP)`
  - `clearWhite()`
  - `drawPixelFast()`
  - `fullUpdate(CLEAR_NONE/CLEAR_FAST/CLEAR_SLOW, keepPowerOn)`
  - `smoothUpdate()`
- [ ] Check if any of these trigger "previous buffer" allocation/access in 4-bpp
- [ ] Test different rendering modes to isolate the issue
- [ ] Document correct usage pattern for 4-bpp mode

### Current Rendering Flow
```cpp
// display.cpp:358-459
bbep.setMode(BB_MODE_4BPP);                     // Force 4-bpp mode
bbep.clearWhite(true);                          // Pre-clear (Mode 1)
int decode_rc = png.decode(NULL, 0);            // PNG callback writes via drawPixelFast()
bbep.fullUpdate(CLEAR_NONE, false);             // Update display
```

**Question**: Does `fullUpdate()` internally call `bbepBackupPlane()` for differential updates?

### Deliverables
- [ ] Complete API usage documentation
- [ ] Identification of problematic API calls (if any)
- [ ] Alternative rendering approaches to test

---

## Phase 3: Test Without Vendored Patches

**Goal**: Get clean baseline to show maintainer

**Status**: ⚪ Not Started

### Tasks
- [ ] Use unpatched FastEPD from PlatformIO registry (`bitbank2/FastEPD@1.3.0`)
- [ ] Remove vendored `lib/FastEPD/` directory temporarily
- [ ] Flash unpatched firmware to device
- [ ] Capture exact behavior:
  - Serial output (heap corruption messages)
  - Visual artifact (photograph of duplicate strip)
  - Crash logs (if any)
- [ ] Compare to patched version behavior
- [ ] Document what our patches actually "fixed" vs masked

### Test Locations
- Unpatched reference: `/TRMNL-T5S3-firmware-1.1.0/TRMNL-T5S3-FastEPD-PR/`
- Patched version: `/TRMNL-T5S3-firmware-1.1.0/TRMNL-T5S3-FastEPD/`
- Public test: `/TRMNL-T5S3-firmware-1.1.0/TRMNL-T5S3-FastEPD-Public/`

### Deliverables
- [ ] Unpatched behavior documentation with photos
- [ ] Serial logs from unpatched version
- [ ] Comparison report: patched vs unpatched

---

## Phase 4: Collaborate with FastEPD Maintainer

**Goal**: Get proper upstream fix

**Status**: ⚪ Not Started

### Tasks
- [ ] Respond to maintainer's request with all Phase 1-3 deliverables:
  - Complete test sketch (maintainer specifically asked for this)
  - Hardware specifications (LilyGo T5S3 Pro)
  - Serial output logs
  - Visual documentation
  - Our API usage pattern
  - Answer: "Are we calling bbepBackupPlane()?" (yes/no with proof)
- [ ] Wait for maintainer's diagnosis
- [ ] Implement maintainer-recommended fix
- [ ] Test fixed version thoroughly on LilyGo T5S3 Pro
- [ ] Confirm fix resolves:
  - Visual artifact (duplicate strip)
  - Heap corruption warnings
  - Deep sleep stability

### Response Format to Maintainer
```markdown
Thanks for looking into this! Here's the complete information you requested:

## Are we calling bbepBackupPlane()?
[YES/NO with code references]

## Complete Test Sketch
[Attach standalone .ino sketch showing our exact usage]

## Hardware Specifications
- Device: LilyGo T5S3 Pro
- Display: 960x540 grayscale e-paper
- Panel: BB_PANEL_EPDIY_V7
- MCU: ESP32-S3 (8MB PSRAM)

## API Call Sequence
[Document exact order of FastEPD calls]

## Serial Output
[Attach logs showing heap corruption]

## Visual Documentation
[Attach photos of duplicate strip artifact]
```

### Deliverables
- [ ] Complete response to maintainer's request
- [ ] Maintainer response received
- [ ] Proper fix identified and implemented
- [ ] Testing confirms fix works

---

## Phase 5: Update TRMNL Firmware for T5S3

**Goal**: Push clean T5S3 support to main TRMNL repo

**Status**: ⚪ Not Started

**Depends on**: Phase 4 completion (FastEPD upstream fix)

### Tasks
- [ ] Once FastEPD is fixed upstream:
  - Remove vendored `lib/FastEPD/` directory
  - Update `platformio.ini` to use fixed FastEPD version
  - Remove all references to "patched version" from documentation
- [ ] Sync with latest TRMNL firmware (maintainer mentioned "TLC needed"):
  - Review TRMNL firmware repository for recent changes
  - Update API integration if needed
  - Align with current TRMNL architecture
  - Add T5S3 as official BYOD device class
- [ ] Test complete device lifecycle:
  - WiFi captive portal setup
  - Device auto-registration (/api/setup)
  - Image download and display (/api/display)
  - Deep sleep power management
  - Button wake events
  - Battery monitoring
  - OTA firmware updates
  - SD card logging (optional)
- [ ] Prepare TRMNL PR:
  - Clean code (no vendored libraries)
  - Updated documentation
  - Testing results
  - Photos of working device

### TRMNL Integration Checklist
- [ ] Remove all vendored FastEPD code
- [ ] Use registry version: `bitbank2/FastEPD@^1.X.X` (fixed version)
- [ ] Sync with latest TRMNL firmware API changes
- [ ] Add T5S3 hardware profile to TRMNL
- [ ] Test with TRMNL backend (staging/production)
- [ ] Document T5S3-specific configuration
- [ ] Add to BYOD device documentation

### Deliverables
- [ ] Pull request to usetrmnl/firmware
- [ ] T5S3 support documentation
- [ ] Testing report with photos
- [ ] Migration guide from vendored version

---

## Phase 6: Documentation & Cleanup

**Goal**: Clean up historical references to vendored library

**Status**: ⚪ Not Started

**Depends on**: Phase 5 completion

### Tasks
- [ ] Update CLAUDE.md:
  - Remove "FastEPD Library Patches" section
  - Document correct 4-bpp usage pattern
  - Reference upstream FastEPD fix
- [ ] Archive historical documentation:
  - Move `lib/FastEPD/PATCHES.md` to `docs/archive/`
  - Move `COMPARISON.md` to `docs/archive/`
  - Add "HISTORICAL - Not Needed" prefix
- [ ] Add credits:
  - Thank FastEPD maintainer (Larry Bank) for the fix
  - Document the resolution approach
- [ ] Update all references in:
  - README.md
  - KNOWN_ISSUES.md
  - PROJECT_STATUS.md
  - platformio.ini comments

### Final Documentation Structure
```
docs/
├── archive/
│   ├── HISTORICAL_PATCHES.md (former lib/FastEPD/PATCHES.md)
│   ├── HISTORICAL_COMPARISON.md (former COMPARISON.md)
│   └── VENDORED_LIBRARY_NOTES.md (lessons learned)
├── FASTEPD_4BPP_USAGE.md (correct usage guide)
└── T5S3_INTEGRATION.md (TRMNL integration notes)
```

### Deliverables
- [ ] Updated documentation (no vendored library references)
- [ ] Archived historical documentation
- [ ] Credits and attribution added
- [ ] Clean, maintainable codebase

---

## Timeline Estimate

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| Phase 1 | 1-2 days | None |
| Phase 2 | 1 day | Phase 1 |
| Phase 3 | 1 day | Phase 2 |
| Phase 4 | 1-7 days | Phase 1-3 (varies by maintainer response) |
| Phase 5 | 2-3 days | Phase 4 (FastEPD fix) |
| Phase 6 | 1 day | Phase 5 |

**Total**: ~1-2 weeks (depending on FastEPD maintainer response time)

---

## Success Criteria

✅ **FastEPD**: Maintainer confirms and fixes 4-bpp issue upstream
✅ **No Vendored Code**: Clean library dependency via PlatformIO registry
✅ **TRMNL PR**: T5S3 support accepted with clean code
✅ **Visual Quality**: No duplicate strip artifact on display
✅ **Stability**: No heap corruption, clean deep sleep operation
✅ **Full Functionality**: All TRMNL features work (WiFi, API, OTA, logging)

---

## Risk Assessment

### High Risk
- **FastEPD maintainer slow response**: Could delay entire project
  - **Mitigation**: Provide extremely detailed response with reproduction
  - **Fallback**: Ask maintainer if temporary workaround is acceptable

### Medium Risk
- **Fundamental API misuse**: We may need to redesign rendering flow
  - **Mitigation**: Phase 2 investigation will identify this early
  - **Fallback**: Use 2-bpp mode if 4-bpp is fundamentally broken

### Low Risk
- **TRMNL firmware API changes**: May need updates to match current version
  - **Mitigation**: Review TRMNL repo before starting Phase 5
  - **Fallback**: Sync changes are usually straightforward

---

## Notes

- **Current firmware version**: 1.1.0
- **FastEPD version (vendored)**: 1.3.0 with 15 patches
- **Hardware**: LilyGo T5S3 Pro (ESP32-S3, 960x540 e-paper)
- **Panel config**: BB_PANEL_EPDIY_V7
- **Render mode**: BB_MODE_4BPP
- **Custom matrix**: `u8M5Matrix` (optimized for 960x540 M5Paper-style displays)
- **Maintainer's key question**: "Are you calling bbepBackupPlane()?"

---

**Last Updated**: 2026-04-07
**Next Review**: After Phase 1 completion
