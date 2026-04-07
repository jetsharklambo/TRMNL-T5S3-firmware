# Contributing to TRMNL T5S3 FastEPD Firmware

Thank you for your interest in contributing to the TRMNL firmware! This guide will help you understand the codebase, development workflow, and best practices.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Development Environment](#development-environment)
3. [Critical: FastEPD Library Patches](#critical-fastepd-library-patches)
4. [Code Architecture](#code-architecture)
5. [Development Workflow](#development-workflow)
6. [Testing Guidelines](#testing-guidelines)
7. [Pull Request Process](#pull-request-process)
8. [Coding Standards](#coding-standards)

---

## Getting Started

### Prerequisites

- **Hardware:** LilyGo T5 4.7" E-Paper Display (ESP32-S3)
- **Software:**
  - PlatformIO Core or PlatformIO IDE (VS Code extension)
  - Python 3.7+ (for PlatformIO)
  - Serial terminal (PlatformIO Monitor, screen, or minicom)
  - macOS, Linux, or Windows

### First Time Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/usetrmnl/trmnl-firmware.git
   cd trmnl-firmware
   ```

2. **Install PlatformIO dependencies:**
   ```bash
   ~/.platformio/penv/bin/platformio run
   ```

3. **⚠️ CRITICAL: Apply FastEPD patches** (see section below)

4. **Flash firmware to device:**
   ```bash
   ~/.platformio/penv/bin/platformio run -t upload
   ```

5. **Monitor serial output:**
   ```bash
   ~/.platformio/penv/bin/platformio device monitor
   ```

---

## Development Environment

### Required Tools

**PlatformIO:**
- Manages ESP32 toolchain, libraries, and build process
- Install via: `pip install platformio` or VS Code extension

**Serial Terminal:**
- Monitor device logs at 115200 baud
- Useful for debugging WiFi, API calls, display operations

**Optional:**
- `esptool.py` for low-level flashing
- `gh` CLI for GitHub pull requests
- Git for version control

### Build Commands

```bash
# Build firmware (compile only)
~/.platformio/penv/bin/platformio run

# Build and flash to device
~/.platformio/penv/bin/platformio run -t upload

# Monitor serial output
~/.platformio/penv/bin/platformio device monitor

# Clean build artifacts
~/.platformio/penv/bin/platformio run -t clean

# Build and flash, then monitor
~/.platformio/penv/bin/platformio run -t upload && ~/.platformio/penv/bin/platformio device monitor
```

---

## Critical: FastEPD Library Patches

### ⚠️ MANDATORY READING

This firmware **requires local patches** to FastEPD v1.3.0. **You MUST apply these patches** or the firmware will have:
- Rendering artifacts (repeated vertical strip, missing pixels)
- Heap corruption crash after image render
- Inability to enter deep sleep

### Why Patches Are Needed

FastEPD library v1.3.0 has buffer calculation bugs in 4BPP (4-bit-per-pixel) mode. The library uses `/4` divisor (for 2BPP) instead of `/2` divisor (for 4BPP) in 15 locations.

### How to Apply Patches

**After first build** (when PlatformIO downloads libraries):

1. **Locate the file:**
   ```
   .pio/libdeps/T5_E_PAPER_S3_V7/FastEPD/src/FastEPD.inl
   ```

2. **Apply 15 patches:**
   - Change **13 instances** of `/4` to `/2` at specific line numbers
   - Change **line 1295** DMA buffer allocation from `(pState->width / 2)` to `(pState->width)`

3. **Complete patch details:** See CLAUDE.md section "FastEPD Library Patches"

### Automation (Recommended)

Create `scripts/patch_fastepd.py` (example):

```python
Import('env')
import os

def patch_fastepd(source, target, env):
    fastepd_path = ".pio/libdeps/T5_E_PAPER_S3_V7/FastEPD/src/FastEPD.inl"

    if not os.path.exists(fastepd_path):
        print("FastEPD.inl not found - patches not applied!")
        return

    with open(fastepd_path, 'r') as f:
        content = f.read()

    # Apply patches (example - full implementation needed)
    # Line 1288: pState->pPrevious = &pState->pCurrent[(width/4) * height];
    # Change to: pState->pPrevious = &pState->pCurrent[(width/2) * height];

    # ... (apply all 15 patches)

    with open(fastepd_path, 'w') as f:
        f.write(content)

    print("FastEPD patches applied successfully!")

env.AddPreAction("buildprog", patch_fastepd)
```

Add to `platformio.ini`:
```ini
[env:T5_E_PAPER_S3_V7]
extra_scripts = post:scripts/patch_fastepd.py
```

### Verifying Patches

After applying patches, build and test:

```bash
# Build and flash
~/.platformio/penv/bin/platformio run -t upload

# Monitor serial output
~/.platformio/penv/bin/platformio device monitor

# Verify:
# 1. Image renders without artifacts on edges
# 2. Device enters deep sleep successfully (no crash)
# 3. Device wakes from sleep without crash
```

---

## Code Architecture

### Modular Design

The firmware uses a **modular architecture** where each major feature is separated:

**Core Modules:**
- `display.cpp` - E-paper rendering (FastEPD library)
- `wifi.cpp` - WiFi connectivity
- `ap_server.cpp` - Captive portal for WiFi setup
- `download.cpp` - HTTPS image downloading
- `sleep.cpp` - Deep sleep power management
- `nvram_config.cpp` - Non-volatile storage (credentials)

**Hardware Modules:**
- `button.cpp` - PCA9555 I/O expander button detection
- `power.cpp` - BQ27220 fuel gauge battery monitoring

**API & Cloud:**
- `trmnl_api_client.h` - TRMNL API integration
- `ota.cpp` - Over-the-air firmware updates
- `api_log.cpp` - Error log submission

**Logging & Telemetry:**
- `logging.cpp` - Centralized logging API
- `device_status.cpp` - Device telemetry collection
- `sd_logging.cpp` - SD card log export

**Main Flow:**
`main.cpp` orchestrates: boot → check credentials → connect WiFi → register device → fetch image → display → sleep

### Key Files

**Configuration:**
- `include/config.h` - System configuration (version, sleep intervals, OTA settings)
- `src/wifi_config.h` - WiFi and API settings (DO NOT commit credentials)
- `platformio.ini` - Build configuration
- `partitions.csv` - Custom partition table

**Documentation:**
- `CLAUDE.md` - Complete developer guide ⭐ **START HERE**
- `CHANGELOG.md` - Version history
- `KNOWN_ISSUES.md` - Known bugs and limitations
- `CONTRIBUTING.md` - This file

---

## Development Workflow

### Making Changes

1. **Create a feature branch:**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make changes in appropriate module:**
   - Display changes → `src/display.cpp`
   - WiFi changes → `src/wifi.cpp`
   - API changes → `include/trmnl_api_client.h`
   - Configuration → `include/config.h`

3. **Test on hardware:**
   - Build and flash to device
   - Monitor serial output for errors
   - Verify functionality end-to-end

4. **Update documentation:**
   - Update CLAUDE.md if architecture changes
   - Update CHANGELOG.md for user-facing changes
   - Add comments to complex code sections

5. **Commit changes:**
   ```bash
   git add .
   git commit -m "feat: Add descriptive commit message"
   ```

### Commit Message Format

Use conventional commits:

```
feat: Add new feature
fix: Fix bug in display rendering
docs: Update CLAUDE.md
refactor: Simplify WiFi connection logic
test: Add battery monitoring tests
chore: Update dependencies
```

---

## Testing Guidelines

### Manual Testing Checklist

Before submitting pull request, test on hardware:

- [ ] **Build:** Firmware compiles without errors
- [ ] **Flash:** Device accepts firmware upload
- [ ] **Boot:** Device boots without crash
- [ ] **WiFi:** Connects to saved network (or enters AP mode if no credentials)
- [ ] **Display:** Image renders correctly (no artifacts on edges)
- [ ] **Sleep:** Device enters deep sleep successfully
- [ ] **Wake:** Device wakes from sleep without crash
- [ ] **Battery:** Battery telemetry reports correctly
- [ ] **OTA:** OTA update process works (if applicable)
- [ ] **Logs:** Serial logs show expected behavior

### Test Scenarios

**First Boot (No Credentials):**
1. Flash firmware to clean device
2. Device enters AP mode (SSID: TRMNL-XXXX)
3. MAC address displayed on screen
4. Connect to AP, enter WiFi credentials
5. Device saves credentials and reboots
6. Device connects to WiFi and registers

**Normal Operation:**
1. Device boots and connects to WiFi
2. Calls `/api/display` and downloads image
3. Renders image on e-paper display
4. Enters deep sleep
5. Wakes after interval and repeats

**Button Actions:**
- Short press: Wake from sleep, refresh content
- 5-second hold: Soft reset (reboot)
- 15-second hold: Hard reset (clear credentials, enter AP mode)

**Recovery:**
- 3 consecutive WiFi failures → auto-clear credentials, enter AP mode

### Display Testing

**Rendering Modes:** Test all 5 modes in `display.cpp` (change RENDER_MODE):

- Mode 1: clearWhite + CLEAR_NONE (default, fastest)
- Mode 2: clearWhite + CLEAR_FAST
- Mode 3: No clearWhite + CLEAR_SLOW
- Mode 4: No clearWhite + CLEAR_FAST
- Mode 5: clearWhite + smoothUpdate

**Visual Inspection:**
- No artifacts on right edge
- No missing pixels on left edge
- Image centered with bezel spacing (IMAGE_X_OFFSET = 5px)
- Text readable and sharp

---

## Pull Request Process

### Before Submitting

1. **Test thoroughly** on hardware (see Testing Guidelines)
2. **Update documentation** (CLAUDE.md, CHANGELOG.md)
3. **Verify FastEPD patches** are documented if modified
4. **Run clean build** to ensure no build errors
5. **Check for credentials** in code (DO NOT commit API keys, WiFi passwords)

### Submitting PR

1. **Push branch to GitHub:**
   ```bash
   git push origin feature/your-feature-name
   ```

2. **Create pull request:**
   - Use GitHub web interface or `gh pr create`
   - Provide clear description of changes
   - Reference any related issues
   - Include testing notes

3. **PR Template:**
   ```markdown
   ## Summary
   Brief description of changes

   ## Changes Made
   - Added/fixed/changed X
   - Updated Y

   ## Testing
   - [ ] Tested on hardware
   - [ ] All test scenarios pass
   - [ ] Documentation updated

   ## Breaking Changes
   None / List any breaking changes
   ```

4. **Address review feedback:**
   - Make requested changes
   - Push updates to same branch
   - PR will auto-update

---

## Coding Standards

### Code Style

**C/C++ Conventions:**
- Use 4 spaces for indentation (no tabs)
- Opening braces on same line
- Clear variable names (no single letters except loop counters)
- Comment complex logic
- Use `const` for constants
- Prefer `nullptr` over `NULL`

**Example:**
```cpp
// Good
void connectToWiFi(const char* ssid, const char* password) {
    Serial.println("[WIFI] Connecting to WiFi...");

    WiFi.begin(ssid, password);
    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WIFI] Connected!");
    }
}
```

### Logging Conventions

Use prefixed `Serial.println()` for consistent logs:

```cpp
Serial.println("[INIT] Initializing system...");
Serial.println("[WIFI] Connecting to WiFi...");
Serial.println("[API] Calling /api/display...");
Serial.println("[IMAGE] Downloading image...");
Serial.println("[DISPLAY] Rendering to e-paper...");
Serial.println("[SLEEP] Entering deep sleep...");
Serial.println("[ERROR] Something went wrong!");
```

**Common prefixes:**
- `[INIT]` - Initialization
- `[WIFI]` - WiFi operations
- `[API]` - API calls
- `[IMAGE]` - Image download/display
- `[BUTTON]` - Button events
- `[POWER]` - Battery/power
- `[SLEEP]` - Deep sleep
- `[OTA]` - Firmware updates
- `[ERROR]` - Errors
- `[DIAGNOSTIC]` - Debug diagnostics

### Configuration

**DO NOT hardcode values** - use `include/config.h`:

```cpp
// Bad
#define SLEEP_SECONDS 300

// Good (in config.h)
#define TRMNL_DEEP_SLEEP_SECONDS 300
```

**DO NOT commit credentials** - use `src/wifi_config.h` with `.gitignore`:

```cpp
// wifi_config.h (gitignored)
#define FALLBACK_WIFI_SSID "your-network"
#define FALLBACK_WIFI_PASSWORD "your-password"
#define TRMNL_API_BASE_URL "https://usetrmnl.com"
```

---

## Common Development Tasks

### Changing Display Rendering

1. **Adjust horizontal offset:**
   - Edit `IMAGE_X_OFFSET` in `include/config.h`
   - Rebuild and test on hardware

2. **Change rendering mode:**
   - Edit `RENDER_MODE` (1-5) in `src/display.cpp`
   - Rebuild and test visual appearance

3. **Modify pixel rendering:**
   - Edit PNG callback in `display.cpp` around line 190
   - Always apply `IMAGE_X_OFFSET` to x-coordinate
   - Use `bbep.drawPixelFast()` for performance

### Adjusting Deep Sleep

1. **Change sleep interval:**
   - Edit `TRMNL_DEEP_SLEEP_SECONDS` in `include/config.h`
   - Default: 300 seconds (5 minutes)
   - For testing: 10 seconds

2. **Change shutdown timer:**
   - Edit `TRMNL_SHUTDOWN_TIMER_SECONDS` in `include/config.h`
   - Default: 0 (immediate sleep)

### Adding API Endpoints

1. **Define endpoint in `trmnl_api_client.h`:**
   ```cpp
   String callNewEndpoint() {
       HTTPClient http;
       http.begin(String(TRMNL_API_BASE_URL) + "/api/new-endpoint");
       http.addHeader("X-API-Key", api_key);
       // ... (add telemetry headers)
       int httpCode = http.GET();
       // ... (handle response)
   }
   ```

2. **Add headers for telemetry:**
   - `X-API-Key` - Device API key
   - `Battery-Voltage` - mV (e.g., 4137)
   - `RSSI` - WiFi signal strength (e.g., -67)
   - `FW-Version` - Firmware version (e.g., "1.1.0")

---

## Getting Help

### Resources

- **CLAUDE.md** - Complete technical documentation ⭐
- **KNOWN_ISSUES.md** - Known bugs and workarounds
- **DIAGNOSTIC_TESTING.md** - Display diagnostic procedures
- **GitHub Issues** - Report bugs or request features

### Common Issues

**"IMAGE_X_OFFSET was not declared":**
- Missing `#include "config.h"` in source file

**Rendering artifacts on edges:**
- FastEPD patches not applied correctly
- Verify all 15 patches in FastEPD.inl

**Heap corruption crash after render:**
- DMA buffer patch (line 1295) not applied
- Should allocate `(pState->width)` not `(pState->width / 2)`

**Device won't enter sleep:**
- Check serial logs for crash before sleep
- Likely heap corruption (see above)

---

## License

This firmware is part of the TRMNL project. Check the main repository for license information.

---

## Thank You!

Your contributions help make TRMNL better for everyone. We appreciate your time and effort! 🎉

---

**Last Updated:** 2026-04-03
**Firmware Version:** 1.1.0
