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

### Display System (FastEPD)

This firmware uses the **official FastEPD library** from GitHub with upstream fix for 4-bpp mode.

**FastEPD Fix** (commit 3e47f1e - 2026-04-07):
- Fixed buffer corruption in 4-bpp mode
- Added guard in `bbepBackupPlane()` to prevent misuse in 4-bpp
- Single-line fix: `if (pState->mode == BB_MODE_4BPP) return;`
- See: https://github.com/bitbank2/FastEPD/commit/3e47f1e

**Migration History**:
- Original: EPDiy library (751 lines in display.cpp)
- Migrated to: FastEPD v1.3.0 (456 lines, 39% reduction)
- v1.1.0: Vendored FastEPD with 15 patches (temporary workaround)
- v1.1.1: Official FastEPD with upstream fix (current)

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

## FastEPD 4-bpp Fix (Upstream - April 2026)

This firmware uses the **official FastEPD library** with upstream fix for 4-bpp buffer corruption.

### The Issue (Fixed April 7, 2026)

FastEPD v1.3.0 had a bug where `clearWhite()`/`clearBlack()` would call `bbepBackupPlane()` even in 4-bpp mode, causing buffer corruption because:
- 4-bpp mode doesn't use the "previous buffer" (only for differential updates in 1/2-bpp)
- But `bbepBackupPlane()` tried to copy the buffer anyway
- Result: Duplicate ~5-10px strip on right edge + heap corruption warnings

### The Fix

**Upstream commit**: https://github.com/bitbank2/FastEPD/commit/3e47f1e3ab6d5e00a979e1c32557bb3bc89e8653

**Changed**: Single line added to `bbepBackupPlane()` in FastEPD.inl:
```cpp
void bbepBackupPlane(FASTEPDSTATE *pState) {
    if (pState->mode == BB_MODE_4BPP) return; // <-- THE FIX
    // ... rest of function
}
```

### Results

**Before Fix** (FastEPD v1.3.0):
- ❌ Duplicate ~5-10px strip on right edge
- ❌ Heap corruption warnings during WiFi shutdown
- ❌ Occasional deep sleep failures

**After Fix** (commit 3e47f1e):
- ✅ Clean rendering across full 960×540 display
- ✅ No heap corruption
- ✅ Stable deep sleep operation
- ✅ Official upstream solution (no vendored code)

### Historical Note

v1.1.0 used a vendored FastEPD with 15 patches as a temporary workaround. The maintainer's single-line fix is cleaner and now integrated upstream.

See `docs/archive/` for historical patch documentation.

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

**FastEPD** (commit 3e47f1e): E-paper display driver
- Official upstream library from GitHub
- Uses fixed version with 4-bpp guard in bbepBackupPlane()
- Specified in platformio.ini: `https://github.com/bitbank2/FastEPD.git#3e47f1e`
- No vendored code, no patches needed

**PNGdec 1.1.6**: PNG image decoder
- Standard library from PlatformIO registry (bitbank2/PNGdec)
- No patches needed

**ArduinoJson 6.21.3**: JSON parsing for API responses
- Standard library from PlatformIO registry (bblanchon/ArduinoJson)
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
