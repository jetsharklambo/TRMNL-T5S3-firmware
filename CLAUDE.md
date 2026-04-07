# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

TRMNL firmware for LilyGo T5S3 Pro (ESP32-S3) e-paper display device. This is an ESP32-based IoT device that connects to WiFi, auto-registers with TRMNL backend, downloads content images, and displays them on a 960x540 grayscale e-paper screen with deep sleep power management.

**Hardware**: LilyGo T5 4.7" E-Paper Display (ESP32-S3, 960x540 e-paper, PCA9555 I/O expander, BQ27220 fuel gauge)

**Key Features**: Zero-config WiFi setup via captive portal, automatic device registration using MAC address, remote content updates, OTA firmware updates, deep sleep power management, battery monitoring, comprehensive logging

## Build Commands

### Standard Build & Flash

```bash
# Build firmware
~/.platformio/penv/bin/platformio run

# Flash to device
~/.platformio/penv/bin/platformio run -t upload

# Monitor serial output (115200 baud)
~/.platformio/penv/bin/platformio device monitor
```

### Flash Script (Component-Based)

The flash scripts flash bootloader, partitions, and firmware separately to correct addresses:

```bash
# macOS/Linux
./flash_firmware.sh /dev/cu.usbmodem14101

# Windows
flash_firmware.bat COM3
```

**Important**: Replace port with your actual serial port. Find it using:
- macOS: `ls /dev/cu.usbmodem* /dev/cu.usbserial*`
- Linux: `ls /dev/ttyACM* /dev/ttyUSB*`
- Windows: Check Device Manager

### Factory Reset

Clear all stored credentials via serial command:

```bash
# Connect serial monitor, press RESET button
# Within 5 seconds of boot, type:
clear
```

## Architecture

### Modular Design

The firmware uses a **modular architecture** where each major feature is separated into its own file:

**Core Modules**:
- `display.cpp` - E-paper rendering (FastEPD library)
- `wifi.cpp` - WiFi connectivity and connection management
- `ap_server.cpp` - Access point mode + captive portal for WiFi setup
- `download.cpp` - Image downloading via HTTPS
- `sleep.cpp` - Deep sleep power management
- `nvram_config.cpp` - Non-volatile storage (WiFi credentials, API keys)

**Hardware Modules**:
- `button.cpp` - PCA9555 I/O expander button detection (GPIO38 interrupt)
- `power.cpp` - BQ27220 fuel gauge battery monitoring

**API & Cloud**:
- `trmnl_api_client.h` - TRMNL API integration (/api/setup, /api/display, /api/log)
- `ota.cpp` - Over-the-air firmware updates with safety checks
- `api_log.cpp` - Error log submission to server

**Logging & Telemetry**:
- `logging.cpp` - Centralized logging API
- `device_status.cpp` - Device telemetry collection (battery, WiFi RSSI, memory)
- `sd_logging.cpp` - SD card daily log export

**Main Flow**: `main.cpp` orchestrates the linear device flow: boot → check credentials → connect WiFi → register device → fetch image → display → sleep

### Display System (FastEPD with Vendored Patches)

**CRITICAL**: This repository uses a **vendored copy of FastEPD v1.3.0** with **15 critical patches** for 4-bpp buffer calculations. The vendored library is located in `lib/FastEPD/` and should **NOT be replaced** with the standard PlatformIO registry version.

**Why Vendored**:
- FastEPD v1.3.0 has buffer calculation bugs in 4-bpp mode
- Bugs cause duplicate rendering artifact on right edge (~5-10px)
- 15 surgical patches applied to `lib/FastEPD/src/FastEPD.inl`
- See `lib/FastEPD/PATCHES.md` for complete patch documentation
- See `COMPARISON.md` for visual comparison (patched vs unpatched)

**Migration History**:
- Original: EPDiy library (751 lines in display.cpp)
- Migrated to: FastEPD v1.3.0 (456 lines, 39% reduction)
- Current: Vendored FastEPD v1.3.0 with patches (clean rendering)

**Panel Configuration**:
- Uses `BB_PANEL_EPDIY_V7` (NOT BB_PANEL_LILYGO_T5PRO)
- Custom gray scale matrix: `u8M5Matrix` (optimized for 960x540 M5Paper-style displays)
- Overrides default `u8SixInchMatrix` (for 1024x758 panels) via `bbep.setCustomMatrix()`
- **Horizontal offset**: 5px (`IMAGE_X_OFFSET`) for better bezel centering

**Display initialization sequence** (display.cpp:224-245):
```cpp
bbep.initPanel(BB_PANEL_EPDIY_V7);              // GPIO pins
bbep.setPanelSize(960, 540, BB_PANEL_FLAG_NONE); // Dimensions
bbep.setCustomMatrix(u8M5Matrix, sizeof(u8M5Matrix)); // Gray matrix override
```

**Rendering flow**:
1. `clearWhite(true)` - Clear display to white (optional, controlled by RENDER_MODE)
2. PNG decoding via `png.decode()` callback writes to `bbep` framebuffer with IMAGE_X_OFFSET
3. `bbep.fullUpdate(CLEAR_NONE/CLEAR_FAST/CLEAR_SLOW, false)` - Transfer buffer to display

**Rendering Modes**: 5 configurable modes in display.cpp (change RENDER_MODE):
- Mode 1 (default): clearWhite + CLEAR_NONE (fastest, minimal flashing)
- Mode 2: clearWhite + CLEAR_FAST (balanced)
- Mode 3: No clearWhite + CLEAR_SLOW (traditional, full flashing)
- Mode 4: No clearWhite + CLEAR_FAST (faster)
- Mode 5: clearWhite + smoothUpdate (smoothest visual)

### Device Registration Flow

**First Boot** (no credentials):
1. Device enters AP mode: SSID `TRMNL-XXXX` (MAC-based)
2. Displays MAC address on screen for pre-registration
3. User connects to AP, enters WiFi credentials via captive portal
4. Device saves credentials to NVRAM, reboots

**Subsequent Boots** (with credentials):
1. Connects to saved WiFi network
2. Calls `/api/setup` with MAC address to auto-register
3. Retrieves and saves API key + friendly ID
4. Calls `/api/display` to fetch content
5. Downloads and displays image
6. Enters deep sleep with server-controlled interval

**Recovery Mechanism**: After 3 consecutive WiFi failures, device auto-clears credentials and re-enters AP mode.

### Deep Sleep Power Management

**Sleep Configuration** (sleep.cpp):
- Timer wakeup: Server-controlled interval (default 300s)
- Button wakeup: PCA9555 GPIO38 interrupt
- RTC memory: Preserves wakeup counter across sleep cycles

**Power Monitoring** (power.cpp):
- BQ27220 fuel gauge: Battery voltage, SOC, temperature
- Free heap monitoring: PSRAM + internal RAM (8.3MB total)
- Telemetry sent to server on each API call

**Button Actions**:
- Press: Wakes device, triggers content refresh
- 5s hold: Soft reset (reboot)
- 15s hold: Hard reset (clear credentials, enter AP mode)

### API Integration

**Headers sent on every API call** (trmnl_api_client.h):
- `X-API-Key`: Device API key
- `Battery-Voltage`: mV (e.g., 4137)
- `RSSI`: WiFi signal strength in dBm (e.g., -67)
- `FW-Version`: Firmware version (e.g., "1.1.0")

**Server Commands** (/api/display response):
- `refresh_rate`: Sleep interval in seconds (stored in NVRAM)
- `update_firmware`: Triggers OTA update
- `firmware_url`: Custom firmware download URL
- `reset_firmware`: Triggers soft reboot

### OTA Updates

**Triple URL Priority** (ota.cpp):
1. `OTA_CUSTOM_FIRMWARE_URL` (config.h) - Developer testing override
2. API `firmware_url` field - Server-managed updates
3. `OTA_OFFICIAL_FIRMWARE_URL` - GitHub releases fallback

**Safety Mechanisms**:
- Battery check: Requires ≥30% SOC
- Partition validation before write
- ESP32 bootloader validates firmware on boot
- Automatic rollback on update failure
- 60-second timeout protection

**Update Flow**: Server sends `update_firmware: true` → device checks battery → downloads firmware → writes to inactive OTA partition → reboots → bootloader validates → runs new firmware

## Configuration Files

**platformio.ini**:
- Board: `T5-ePaper-S3`
- Platform: `espressif32@6.5.0`
- Framework: Arduino
- Partition table: `partitions.csv` (custom for larger SPIFFS)
- Serial: Upload 460800 baud, Monitor 115200 baud

**src/wifi_config.h**: WiFi and API settings (DO NOT commit credentials)

**include/config.h**: System configuration (firmware version, OTA settings, deep sleep intervals)

**partitions.csv**: Custom partition table for 16MB flash (bootloader, OTA_0, OTA_1, SPIFFS, NVS)

## FastEPD Library Patches (Vendored Version)

**⚠️ CRITICAL**: This repository uses a **vendored copy** of FastEPD v1.3.0 with critical patches applied. The library is located in `lib/FastEPD/` and takes precedence over the PlatformIO registry version.

**Location**: `lib/FastEPD/` (vendored, patched)
**Patches Applied**: 15 surgical fixes to `src/FastEPD.inl`
**Documentation**: See `lib/FastEPD/PATCHES.md` for complete details

### What Was Fixed

The upstream FastEPD v1.3.0 has buffer calculation errors in 4-bpp mode. All fixes change `/4` divisor (correct for 2-bpp) to `/2` divisor (correct for 4-bpp):

1. **Buffer Allocation** (lines 1288, 1289, 1295)
   - Previous buffer pointer offset
   - Temp buffer pointer offset
   - DMA buffer allocation size (critical for double-buffering)

2. **Row Operations** (8 fixes: lines 1553, 1562, 1575, 1577, 1579, 1635, 1636, 1672)
   - memset/memcpy size calculations

3. **Array Indexing** (4 fixes: lines 1610, 1633, 1758, 1796)
   - Temp buffer offset calculations

4. **Loop Iterations** (8 fixes: lines 1614, 1621, 1655, 1663, 1762, 1769, 1820, 1828)
   - Iteration count calculations

5. **Write Operations** (5 fixes: lines 1799, 1800, 1846, 1848, 1849)
   - bbepWriteRow byte count parameters

### Why Patches Are Needed

- **Root Cause**: FastEPD v1.3.0 assumes 2-bpp mode (4 pixels per byte) throughout codebase
- **Our Display**: Uses 4-bpp mode (2 pixels per byte) on 960×540 panel
- **Symptom**: Duplicate vertical strip on right edge (~5-10px) due to buffer underallocation
- **Impact**: Heap corruption warnings, unstable deep sleep, visual artifacts

### Results

**Before Patches** (unpatched FastEPD v1.3.0):
- ❌ Duplicate ~5-10px strip on right edge
- ❌ Heap corruption warnings during WiFi shutdown
- ❌ Occasional deep sleep failures

**After Patches** (vendored version):
- ✅ Clean rendering across full 960×540 display
- ✅ No heap corruption
- ✅ Stable deep sleep operation
- ✅ No performance degradation
- ✅ Memory cost: ~480 bytes (negligible on 8MB PSRAM)

### Compatibility

- ✅ **1-bpp mode**: Unaffected (different code paths)
- ✅ **2-bpp mode**: Unaffected (divisor is already correct)
- ✅ **4-bpp mode**: Fixed (was broken, now works)

### When to Remove Vendored Version

**Scenario 1**: FastEPD maintainer merges upstream fix
1. Update `platformio.ini`: `bitbank2/FastEPD@^1.4.0` (or version with fix)
2. Delete `lib/FastEPD/` directory
3. Rebuild - patches no longer needed

**Scenario 2**: Continue using vendored version
- Track upstream releases manually
- Merge non-conflicting updates as needed
- Maintain vendored patches

### Important Notes

- **DO NOT** add `bitbank2/FastEPD@1.3.0` to platformio.ini lib_deps - it has the bugs
- **DO NOT** delete `lib/FastEPD/` directory - it contains the fixes
- **SEE** `lib/FastEPD/PATCHES.md` for line-by-line patch documentation
- **SEE** `COMPARISON.md` for visual comparison (patched vs unpatched)

## Common Issues & Debugging

### I2C Bus Conflicts

**Symptom**: Button press events when button not pressed, or invalid battery readings

**Cause**: PCA9555 I/O expander and BQ27220 fuel gauge on same I2C bus (GPIO39 SDA, GPIO40 SCL). Display operations can corrupt bus state.

**Known occurrences**:
- After WiFi initialization (fuel gauge returns invalid SOC 65535%, shows fallback 50%)
- After display operations (spurious button press events)

**Workaround**: Multiple retries, fallback values, ignore spurious events

### Serial Debug Logging

All modules use `Serial.println()` with prefixes:
- `[INIT]` - Initialization
- `[WIFI]` - WiFi operations
- `[API]` - API calls
- `[IMAGE]` - Image download/display
- `[BUTTON]` - Button events
- `[POWER]` - Battery/power
- `[SLEEP]` - Deep sleep
- `[OTA]` - Firmware updates
- `[DIAGNOSTIC]` - Debug diagnostics
- `[ERROR]` - Errors

## Testing

### Manual Testing Flow

1. Flash firmware to device
2. Device boots → AP mode (TRMNL-XXXX)
3. Connect to AP, configure WiFi credentials
4. Device registers with TRMNL backend
5. Fetches and displays content
6. Enters deep sleep
7. Wakes after interval, repeats

### Diagnostic Mode

See DIAGNOSTIC_TESTING.md for detailed testing procedures including:
- Display dimension verification
- Buffer transfer diagnostics
- Clear/render mode testing
- Refresh mode testing

### Log Analysis

**SPIFFS Logs**: Binary circular buffer (100 entries) in `/spiffs/logs.bin`

**SD Card Logs**: Human-readable daily files in `/logs/YYYY-MM-DD.log` with full device telemetry per entry

## Development Notes

### Display Code Changes

When modifying `display.cpp`:
- Always call `clearWhite(true)` before rendering (for most RENDER_MODE settings)
- PNG callback uses `bbep.drawPixelFast()` for pixel writes with IMAGE_X_OFFSET applied
- Keep `u8M5Matrix` custom matrix override
- DO NOT change panel definition from BB_PANEL_EPDIY_V7
- Adjust IMAGE_X_OFFSET in config.h (not display.cpp) for horizontal positioning
- Test all 5 RENDER_MODE options when changing rendering logic

### NVRAM Storage

Credentials stored in ESP32 NVS (Non-Volatile Storage):
- WiFi SSID/password
- API key
- Friendly ID
- Refresh rate
- WiFi failure counter

**Critical**: Always call `nvram_begin()` before reading/writing

### Deep Sleep Timing

Sleep interval controlled by server via `refresh_rate` field. Default: 300 seconds (5 minutes).

**Important**: RTC memory is preserved across sleep. Use `RTC_DATA_ATTR` for variables that need to persist.

### Library Dependencies

**FastEPD v1.3.0+patches**: E-paper display driver
- **VENDORED** in `lib/FastEPD/` with critical 4-bpp patches
- **DO NOT** use `bitbank2/FastEPD@1.3.0` from PlatformIO registry (has bugs)
- **DO NOT** delete vendored library directory
- See `lib/FastEPD/PATCHES.md` for patch details

**PNGdec 1.1.6**: PNG image decoder (bitbank2/PNGdec)
- Standard library from PlatformIO registry
- No patches needed

**ArduinoJson 6.21.3**: JSON parsing for API responses
- Standard library from PlatformIO registry
- No patches needed

## File References

**Documentation**:
- PROJECT_STATUS.md - Complete phase-by-phase progress
- KNOWN_ISSUES.md - Known bugs and limitations
- FLASH_INSTRUCTIONS.md - User flashing guide
- DIAGNOSTIC_TESTING.md - Display diagnostic procedures
- README.md - User installation guide

**Build Artifacts**:
- `.pio/build/T5_E_PAPER_S3_V7/firmware.bin` - Main firmware
- `.pio/build/T5_E_PAPER_S3_V7/bootloader.bin` - ESP32 bootloader
- `.pio/build/T5_E_PAPER_S3_V7/partitions.bin` - Partition table
