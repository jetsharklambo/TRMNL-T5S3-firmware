# TRMNL-T5S3 Pro Modular Firmware

Production-ready firmware for the LilyGo T5 4.7" E-Paper Display (ESP32-S3) with WiFi provisioning, auto-registration, and TRMNL cloud integration.

**Status**: ✅ **1.0 RELEASE READY** = 100% Core Features Complete
**Last Updated**: December 17, 2025
**Firmware Version**: 1.0.0

### Project Progress
- ✅ Phase 1: Core Infrastructure (100%)
- ✅ Phase 2: Device Registration with Error Handling (100%)
- ✅ Phase 3: WiFi Configuration (85% - escalating retry delays deferred to 1.1)
- ✅ Phase 4: Enhanced API Client (100% - Telemetry, OTA, dynamic refresh, log submission)
- ⏳ Phase 5: Button & Power Management (50% - future enhancements)
- ✅ Phase 6: Display & Logging Infrastructure (100%)
- ✅ Phase 7: Firmware OTA Updates (100%)
- ✅ Phase 8: Business Logic (100%)
- ✅ Phase 9: Production Ready Features (100%)

## 🚀 Quick Start

### For Users (Fresh Device)
1. **Flash** the firmware to your T5 device (see Installation section below)
2. **First Boot**: Device enters AP mode automatically (no credentials)
3. **Connect** to WiFi network named `TRMNL-XXXX` (using last 2 MAC address bytes)
4. **Open** web browser (auto-redirects to setup portal)
5. **Enter** your WiFi SSID and password
6. **Device** reboots and:
   - ✅ Connects to your WiFi
   - ✅ Auto-registers with TRMNL backend (MAC-based)
   - ✅ Gets API key from server
   - ✅ Downloads and displays your content
   - ✅ Enters deep sleep (configurable interval)

### For Developers
```bash
# Clone and build
git clone <repo>
cd TRMNL-T5S3
pio run

# Flash device
pio run -t upload

# Monitor serial output
pio device monitor
```

## 📥 Installation & Flashing

### Prerequisites

Before flashing the firmware, you'll need:

#### Hardware
- **LilyGo T5 4.7" E-Paper Display** (ESP32-S3)
- **USB-C cable** (for data transfer, not just charging)
- **Computer** (macOS, Linux, or Windows)

#### Software

**Option 1: Pre-compiled Binary (Easiest)**
- **esptool.py**: Install via pip
  ```bash
  pip install esptool
  ```

**Option 2: Build from Source**
- **Python 3.7+**: [Download](https://www.python.org/downloads/)
- **PlatformIO**: Install via pip
  ```bash
  pip install platformio
  ```

#### USB Drivers

**macOS/Linux**: Usually automatic, no drivers needed

**Windows**: Install [CP210x USB to UART Bridge drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

---

### Method 1: Automated Flash Script (Recommended)

The easiest way to flash the firmware:

1. **Download the v1.0.0 release assets** from the [releases page](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/releases/tag/v1.0.0):
   - `firmware.bin`
   - `bootloader.bin`
   - `partitions.bin`
   - `flash.sh`

2. **Install esptool.py**:
   ```bash
   pip install esptool
   ```

3. **Connect your device** via USB-C cable

4. **Run the flash script**:
   ```bash
   cd releases/v1.0.0
   chmod +x flash.sh
   ./flash.sh
   ```

   The script will:
   - Auto-detect your USB port
   - Flash bootloader, partition table, and firmware
   - Show progress for each step
   - Provide next steps after completion

5. **Manual port specification** (if auto-detect fails):
   ```bash
   # macOS
   ./flash.sh /dev/cu.usbserial-XXXX

   # Linux
   ./flash.sh /dev/ttyUSB0

   # Windows (Git Bash)
   ./flash.sh COM3
   ```

---

### Method 2: Manual Flash with esptool.py

For more control over the flashing process:

1. **Download firmware files** (same as Method 1)

2. **Install esptool.py**:
   ```bash
   pip install esptool
   ```

3. **Find your USB port**:
   ```bash
   # macOS
   ls /dev/cu.usbserial-* /dev/cu.usbmodem*

   # Linux
   ls /dev/ttyUSB* /dev/ttyACM*

   # Windows (PowerShell)
   Get-WmiObject Win32_SerialPort | Select-Object DeviceID,Description
   ```

4. **Flash the firmware** (replace `/dev/cu.usbserial-XXXX` with your port):

   ```bash
   # Flash bootloader at address 0x0
   esptool.py --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
       write_flash 0x0 bootloader.bin

   # Flash partition table at address 0x8000
   esptool.py --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
       write_flash 0x8000 partitions.bin

   # Flash firmware at address 0x10000
   esptool.py --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
       write_flash 0x10000 firmware.bin
   ```

5. **Verify flash success**:
   ```bash
   esptool.py --chip esp32s3 --port /dev/cu.usbserial-XXXX verify_flash \
       0x10000 firmware.bin
   ```

---

### Method 3: Build and Flash from Source

For developers who want to compile the firmware:

1. **Clone the repository**:
   ```bash
   git clone https://github.com/jetsharklambo/TRMNL-T5S3-firmware/TRMNL-T5S3.git
   cd TRMNL-T5S3
   ```

2. **Install PlatformIO**:
   ```bash
   pip install platformio
   ```

3. **Build the firmware**:
   ```bash
   pio run
   ```

   Expected output:
   ```
   RAM:   [===       ]  28.5% (used 93320 bytes from 327680 bytes)
   Flash: [=======   ]  65.1% (used 1366233 bytes from 2097152 bytes)
   ========================= [SUCCESS] Took 8.21 seconds =========================
   ```

4. **Flash to device**:
   ```bash
   pio run -t upload
   ```

   PlatformIO will auto-detect the port and flash all components.

5. **Monitor serial output**:
   ```bash
   pio device monitor
   ```

   Press `Ctrl+C` to exit the monitor.

---

### Method 4: PlatformIO IDE (Visual Studio Code)

1. **Install VS Code** and the **PlatformIO IDE extension**

2. **Open the project folder** in VS Code

3. **Click the PlatformIO icon** in the sidebar

4. **Build**: Click "Build" in the Project Tasks

5. **Upload**: Click "Upload" in the Project Tasks

6. **Monitor**: Click "Monitor" to view serial output

---

### Post-Flash: First Boot Setup

After successfully flashing:

1. **Disconnect and reconnect** the USB cable (or press the reset button)

2. **Device boots** and enters AP mode (no WiFi credentials stored)

3. **Serial output** shows:
   ```
   [BOOT] TRMNL-T5S3 Firmware v1.0.0
   [NVRAM] No WiFi credentials found
   [AP] Starting WiFi AP mode: TRMNL-XXXX
   [AP] AP IP address: 192.168.4.1
   [AP] Waiting for WiFi credentials...
   ```

4. **Connect to WiFi AP**:
   - Network name: `TRMNL-XXXX` (where XXXX are last 2 bytes of MAC)
   - No password required
   - Captive portal should auto-open

5. **Enter WiFi credentials**:
   - SSID: Your WiFi network name
   - Password: Your WiFi password
   - Click "Connect"

6. **Device reboots** and connects to WiFi:
   ```
   [WIFI] Connecting to YOUR_SSID...
   [WIFI] Connected! IP: 192.168.1.XXX
   [TRMNL] Calling /api/setup for device registration...
   [TRMNL] API Key: xxxxxxxx
   [TRMNL] Friendly ID: XXXXXX
   [TRMNL] Calling /api/display for image update...
   [DOWNLOAD] Downloading image...
   [DISPLAY] Rendering image...
   [SLEEP] Entering deep sleep for 300 seconds...
   ```

---

### Troubleshooting

#### Flash Fails with "No serial data received"
- **Press and hold BOOT button** while connecting USB cable
- Try a **different USB cable** (must support data transfer)
- Try a **lower baud rate**: Use `460800` or `115200` instead of `921600`
- **Check USB drivers** (especially on Windows)

#### Device Not Entering AP Mode
- **Serial command**: Type `clear` within 5 seconds of boot to erase credentials
- **Hard reset**: Hold button for 15 seconds to factory reset
- **Verify flash**: Use `esptool.py verify_flash` to confirm firmware integrity

#### Can't Find USB Port
- **Unplug and replug** the USB cable
- **Check Device Manager** (Windows) or `ls /dev` (macOS/Linux)
- **Install drivers** if needed (see USB Drivers section above)
- **Try different USB port** (avoid USB hubs if possible)

#### Compilation Errors (Building from Source)
- **Update PlatformIO**: `pio upgrade`
- **Clean build**: `pio run -t clean` then `pio run`
- **Check Python version**: Must be 3.7 or higher
- **Install dependencies**: `pio lib install`

#### Display Not Updating After Flash
- **Check battery**: Must be >20% for operation
- **Check WiFi**: Device will re-enter AP mode if WiFi fails
- **Monitor serial**: `screen /dev/cu.usbserial-XXXX 115200` (Ctrl+A then K to exit)
- **Check logs**: Review SD card logs if present

---

### Serial Monitoring

To view real-time device output:

**Using screen (macOS/Linux)**:
```bash
screen /dev/cu.usbserial-XXXX 115200
```
Press `Ctrl+A` then `K` to exit.

**Using PlatformIO**:
```bash
pio device monitor
```
Press `Ctrl+C` to exit.

**Using Arduino IDE Serial Monitor**:
- Tools → Serial Monitor
- Set baud rate to 115200

**Example serial output**:
```
[BOOT] TRMNL-T5S3 Firmware v1.0.0
[OTA] Confirmed valid boot - rollback protection active
[POWER] Battery: 4.20V (96%, full, CHARGING, 25.5°C)
[WIFI] Connecting to MyNetwork...
[WIFI] Connected! IP: 192.168.1.100, RSSI: -45dBm
[TRMNL] Calling /api/display for image update...
[TRMNL] Refresh Rate: 304 seconds
[DOWNLOAD] Downloading image (17540 bytes)...
[DISPLAY] Rendering 960x540 PNG...
[DISPLAY] Display update complete (4.2s)
[SLEEP] Entering deep sleep for 304 seconds...
```

---

### Factory Reset

To completely erase the device and start fresh:

**Method 1: Button Hold (Hardware)**
- Press and hold the button for **15 seconds**
- Device will erase all credentials and reboot into AP mode

**Method 2: Serial Command (Software)**
- Connect via serial monitor
- Within 5 seconds of boot, type: `clear`
- Device will erase credentials and reboot into AP mode

**Method 3: Erase Flash (Complete Wipe)**
```bash
esptool.py --chip esp32s3 --port /dev/cu.usbserial-XXXX erase_flash
```
Then re-flash the firmware using one of the methods above.

## 📋 Completed Features

### Phase 1: Modularization ✅
- Extracted 750-line monolithic `main.cpp` into focused modules
- **display.cpp**: E-paper rendering, PNG/BMP support, text overlay
- **wifi.cpp**: WiFi connectivity, connection polling
- **download.cpp**: HTTP image downloads, chunked streaming
- **sleep.cpp**: Deep sleep, button/timer wakeup
- ✅ 100% functionality preserved, zero changes to core logic

### Phase 2: NVS Persistence ✅
- **nvram_config.cpp**: Non-volatile storage for credentials
- Saves: WiFi SSID, WiFi password, API key, friendly device ID
- Automatic fallback to hardcoded values
- `nvram_clear_all()` for factory reset via serial command "clear"

### Phase 2c: AP Mode & Serial Control ✅
- WiFi AP broadcasting (`softAP`) with DNS spoofing
- Captive portal: Auto-redirect all domains to 192.168.4.1
- Beautiful HTML form with client-side validation
- Stores credentials to NVS, auto-reboots
- Serial command "clear" erases all credentials during boot window

### Phase 3: Captive Portal Implementation ✅
- **ap_server.cpp**: HTTP server + DNS server
- Responsive web UI (gradient background, password toggle)
- Credential validation (SSID max 32 chars, password max 64 chars)
- POST endpoint saves to NVS and triggers reboot

### Phase 2: Device Registration Flow (COMPLETE) ✅
**Recent Enhancements:**
- ✅ **Real Telemetry**: Battery voltage and WiFi RSSI sent to server (visible on dashboard!)
- ✅ **AP Mode MAC Display**: Shows device MAC address during setup for pre-registration
- ✅ **WiFi Failure Recovery**: Auto-recovery after 3 failed attempts
  - Displays "WiFi Failed (X/3)" feedback to user
  - Automatically clears bad credentials
  - Returns to AP mode for credential re-entry
- ✅ **Failure Counter Persistence**: Tracks failures across reboots in NVRAM
- **Features**:
  - `/api/setup` endpoint: MAC-based auto-registration (no hardcoded keys!)
  - Gets API key + friendly ID from TRMNL server
  - `/api/display` endpoint: Fetches current image from user's account
  - Signed S3 URLs for image downloads
  - Real battery voltage in `Battery-Voltage` header
  - Real WiFi signal strength in `RSSI` header (dBm)
  - Proper header support: `ID` (MAC), `Access-Token` (API key)

### Phase 3: WiFi & Configuration (85% COMPLETE) ⚠️
- ✅ WiFi credential persistence to NVRAM
- ✅ Captive portal with DNS spoofing
- ✅ Beautiful HTML form with validation
- ✅ Auto-recovery on repeated failures
- ✅ Visual failure feedback to user
- ⏳ Escalating retry delays (planned)

### Phase 4: Enhanced API Client (100% COMPLETE) ✅
**Implemented:**
- ✅ Battery voltage telemetry (real values from BQ27220)
- ✅ WiFi signal strength reporting (real RSSI in dBm)
- ✅ Firmware version reporting (v1.0.0 sent to server)
- ✅ Telemetry visible on TRMNL dashboard
- ✅ Dynamic refresh rate (server-controlled sleep intervals)
- ✅ Response parsing for refresh_rate, update_firmware, reset_firmware
- ✅ Firmware update handling (OTA updates working)
- ✅ Device reset handling (soft reboot)
- ✅ **API Log Submission** (errors sent to server for diagnostics)
  - WiFi connection failures
  - API call failures
  - Image download failures
  - OTA update failures

### Phase 6: Display & Logging Infrastructure (100% COMPLETE) ✅
**Implemented:**
- ✅ **SPIFFS Log Storage** (100 entries circular buffer)
  - Binary log format in `/spiffs/logs.bin`
  - Automatic log rotation (keeps last 100 entries)
  - Persistent across reboots
- ✅ **SD Card Daily Export** (human-readable text files)
  - Daily log files: `/logs/YYYY-MM-DD.log`
  - Graceful degradation (device works without SD card)
  - Device MAC address in file header
- ✅ **Comprehensive Event Logging**
  - Wake source detection (button vs timer)
  - WiFi connection events (success/failure)
  - API calls (setup, display, errors)
  - Image download events
  - Battery warnings (SOC < 20%)
  - Sleep transitions
- ✅ **Device Status Telemetry**
  - Battery: voltage, SOC, temperature (BQ27220 fuel gauge)
  - Memory: free/total heap (accurate calculations)
  - WiFi: RSSI signal strength
  - System: uptime, current image filename
  - Error tracking: last error message
- ✅ **Accuracy Fixes**
  - NTP time synchronization (accurate timestamps)
  - Fixed heap calculation (8.3MB vs buggy 14-16MB)
  - Fixed initialization order (battery data before first log)
  - Image filename tracking (actual plugin IDs)

**Log File Format:**
```
========================================
TRMNL Device Log - 2025-12-18
Device MAC: D8:3B:DA:70:CF:50
========================================

[2025-12-18 01:19:55] INFO: API /display call successful
  Battery: 4.13V (99%, 25°C), Heap: 7.1MB/8.3MB, WiFi: -67dBm
  Uptime: 13s, Image: plugin-421bb9-1766013565
  Last Error: none
```

**Device Health Insights:**
- Battery: 4.13-4.20V (99% SOC, 24-26°C - excellent health)
- WiFi: -14 to -74 dBm (variable signal, strong when close)
- Memory: 7.1-8.3MB free (stable, no leaks detected)
- Power Efficiency: 17-22s active, ~5min sleep (95%+ sleep time)
- Update Success: 85.7% success rate over extended operation

### Phase 7: Over-The-Air Firmware Updates (100% COMPLETE) ✅
**Implemented:**
- ✅ **HTTPS Firmware Download**
  - Streaming download (no full buffering)
  - Direct flash to inactive OTA partition
  - Timeout protection (60 seconds)
  - SSL/TLS certificate validation
- ✅ **Triple Firmware Source Priority**
  - Priority 1: Custom URL (config.h override for testing)
  - Priority 2: API URL (TRMNL server managed)
  - Priority 3: Official URL (GitHub releases fallback)
- ✅ **Safety Mechanisms**
  - Battery level check (30% minimum)
  - Partition validation before write
  - Automatic rollback on failure
  - Boot partition switching (OTA_0 ↔ OTA_1)
- ✅ **Update Triggers**
  - Server-initiated via `update_firmware` flag
  - Automatic when API sends command
  - Configurable auto-update enable/disable
- ✅ **Error Handling**
  - Network failure detection
  - Flash write validation
  - Size mismatch detection
  - User-friendly error messages on display

**Configuration (config.h):**
```cpp
#define OTA_ENABLE 1                    // Enable/disable OTA
#define OTA_OFFICIAL_FIRMWARE_URL "..." // GitHub releases
#define OTA_CUSTOM_FIRMWARE_URL ""      // Custom server override
#define OTA_MIN_BATTERY_PERCENT 30      // Safety threshold
#define OTA_TIMEOUT_MS 60000            // Download timeout
#define OTA_AUTO_UPDATE 1               // Auto-update on server command
```

**Update Flow:**
1. Server sends `update_firmware: true` in `/api/display` response
2. Device checks battery level (≥30%)
3. Device determines firmware URL (custom → API → official)
4. Downloads firmware via HTTPS streaming
5. Validates and flashes to inactive partition
6. Reboots into new firmware
7. On failure: logs error, displays message, continues operation

**Build Info (v1.0.0):**
- Flash: 65.1% (1,366,233 / 2,097,152 bytes)
- RAM: 28.5% (93,320 / 327,680 bytes)
- **OTA Rollback Protection**: Active
- **API Log Submission**: Enabled

## 📚 Document Overview

### 0. **PROJECT_STATUS.md** (NEW) - Current Project Status
**The Most Important Document** - Start here for current progress!

**Contains**:
- Current completion status by phase
- What's working and what's planned
- Build and flash instructions
- File organization
- Testing checklist
- Known limitations
- Recommended next steps with time estimates
- Quick reference for developers

**When to Read**: First thing! Get the current project status.

---

### 1. **FIRMWARE_EVOLUTION_PLAN.md** (87 KB)
The comprehensive master plan for firmware evolution.

**Contains**:
- Executive summary of current vs target state
- Official TRMNL firmware architecture analysis
- API endpoints specification (/api/setup, /api/display, /api/log)
- 9 implementation phases with detailed tasks
- Project structure and organization
- Power consumption targets
- Testing checklist
- 33-43 hour timeline estimate
- Success criteria

**When to Read**: Start here for overall vision and scope.

---

### 2. **CORE_FUNCTIONALITY_PRESERVATION.md** (62 KB)
**⚠️ CRITICAL** - How to refactor WITHOUT breaking working features.

**Contains**:
- 10 critical code sections that must not be modified
- Functions that can be safely wrapped/extended
- Architectural integration points
- Testing checklist before any refactoring
- Git strategy for safe development
- Critical variables and data structures
- Memory considerations
- Known fragile areas
- Recommended modularization structure
- Final safety checklist

**When to Read**: BEFORE starting any refactoring work. This is your safety guide.

**Key Rule**: Extract don't rewrite. Wrap don't modify.

---

### 3. **HOW_CORE_SYSTEMS_WORK.md** (55 KB)
Deep technical explanation of internal mechanics.

**Contains**:
- **Part 1**: WiFi Connection System
  - Initialization sequence
  - Connection polling mechanism
  - Status codes explanation
  - Disconnection and power management

- **Part 2**: Image Download System
  - HTTP client setup and timeouts
  - Chunked download loop explanation
  - Buffer management
  - Stream handling
  - Download verification

- **Part 3**: Image Display and Rendering
  - Display hardware setup
  - I2C communication
  - VCOM calibration
  - File format detection
  - PNG decoding process
  - Pixel extraction (bit-level details)
  - Framebuffer management
  - Display update sequence
  - Power sequencing

- **Part 4**: Deep Sleep and Wakeup
  - GPIO button wakeup
  - Timer wakeup configuration
  - Sleep vs light sleep
  - Wakeup cause detection

- **Part 5**: Critical dependencies and integration points

- **Part 6**: What can break and how

**When to Read**: When you need to understand WHY something is the way it is before modifying it.

---

### 4. **SAFE_REFACTORING_PATTERNS.md** (48 KB)
Concrete before/after examples of safe refactoring.

**Contains**:
- **Pattern 1**: Extract functions without changing implementation
- **Pattern 2**: Add wrapper functions for enhanced behavior
- **Pattern 3**: Move configuration to persistent storage (safely)
- **Pattern 4**: Add state machine without breaking core
- **Pattern 5**: Add API integration without breaking core
- **Pattern 6**: Safe preference storage addition
- **Pattern 7**: Safe error recovery addition

Each pattern shows:
- ❌ **WRONG** way (with explanation of why it breaks)
- ✅ **RIGHT** way (with explanation of why it's safe)
- Code examples
- Reasoning

Also includes:
- Safety checklist
- Git workflow example

**When to Read**: When you're about to make a change and want to know the safe way to do it.

---

## 🎯 Quick Start: Refactoring Safely

### If You're About to Make Changes:

1. **Read**: `CORE_FUNCTIONALITY_PRESERVATION.md` (Critical!)
2. **Read**: `HOW_CORE_SYSTEMS_WORK.md` (Understand the system)
3. **Follow**: `SAFE_REFACTORING_PATTERNS.md` (Pattern to use)

### The Golden Rules:

1. ✅ **Extract** functions to separate modules (no logic changes)
2. ✅ **Wrap** existing functions with new behavior
3. ✅ **Call** original functions from new code
4. ✅ **Test** original functionality still works
5. ✅ **Commit** atomically after each phase

---

1. ❌ **Rewrite** critical functions
2. ❌ **Inline** logic into other functions
3. ❌ **Modify** function signatures without updating all callers
4. ❌ **Skip** testing original behavior
5. ❌ **Combine** multiple features in one commit

---

## 🔴 Critical Sections (DO NOT MODIFY)

These code sections are working but fragile. Extract, don't rewrite:

| Code | Location | Risk | Line Count |
|------|----------|------|-----------|
| Display Init | main.cpp:239-252 | 🔴 HIGH | 14 lines |
| PNG Render Loop | main.cpp:520-546 | 🔴 HIGH | 27 lines |
| Display Update Sequence | main.cpp:549-564 | 🔴 HIGH | 16 lines |
| Chunked Download | main.cpp:322-339 | 🟡 MEDIUM | 18 lines |
| WiFi Poll Loop | main.cpp:643-648 | 🟡 MEDIUM | 6 lines |
| Deep Sleep Setup | main.cpp:723-728 | 🟡 MEDIUM | 6 lines |

**Total Critical Code**: ~87 lines out of 750

---

## 🟢 Safe to Modify

These can be changed freely (but still test):

| Configuration | Purpose | Impact |
|---------------|---------|--------|
| WIFI_SSID | WiFi network name | Connection |
| WIFI_PASSWORD | WiFi password | Connection |
| TRMNL_API_KEY | API credentials | API calls |
| TRMNL_TEST_IMAGE_URL | Fallback image | Display |
| TRMNL_DEEP_SLEEP_SECONDS | Sleep duration | Battery life |
| Timeouts | Connection timeouts | Robustness |

---

## 📋 Testing Checklist

**Must Pass Before Any Change**:

- [ ] Device boots and shows startup message
- [ ] Display initializes correctly
- [ ] WiFi connects (or shows error after 15s)
- [ ] Image downloads successfully
- [ ] Image renders on display
- [ ] No black corners or artifacts
- [ ] Deep sleep triggers
- [ ] Device wakes on timer
- [ ] Device wakes on button press
- [ ] Battery life is acceptable

**Must Pass After Any Change**:

All of the above, plus:
- [ ] New features work as intended
- [ ] Original behavior unchanged
- [ ] No memory leaks
- [ ] No crashes or watchdog resets
- [ ] Serial output clean (no errors)
- [ ] Git diff shows only intended changes

---

## 📂 Project Structure

```
TRMNL-T5S3/
├── include/                          # Header files
│   ├── config.h                      # Global config (deep sleep, timeouts)
│   ├── display.h                     # E-paper display functions
│   ├── wifi_module.h                 # WiFi connectivity
│   ├── download.h                    # Image download
│   ├── sleep.h                       # Deep sleep & wakeup
│   ├── nvram_config.h                # NVS persistence layer
│   ├── wifi_config.h                 # WiFi/API configuration
│   ├── ap_server.h                   # AP mode & captive portal
│   ├── trmnl_api_client.h            # TRMNL API integration
│   └── firasans_20.h                 # Font for display
│
├── src/                              # Implementation files
│   ├── main.cpp                      # Simplified orchestrator (130 lines)
│   ├── display.cpp                   # E-paper rendering (600+ lines)
│   ├── wifi.cpp                      # WiFi functions
│   ├── download.cpp                  # Image download stream
│   ├── sleep.cpp                     # Sleep management
│   ├── nvram_config.cpp              # NVS implementation
│   ├── ap_server.cpp                 # HTTP + DNS servers
│   └── trmnl_api_client.h            # API client (inline header)
│
├── lib/
│   └── epdiy/                        # E-paper display library
│
├── platformio.ini                    # Build configuration
├── README.md                         # This file (updated!)
├── FIRMWARE_EVOLUTION_PLAN.md        # Original comprehensive plan
├── CORE_FUNCTIONALITY_PRESERVATION.md
├── HOW_CORE_SYSTEMS_WORK.md
└── SAFE_REFACTORING_PATTERNS.md
```

## 🔧 Key Modules

| Module | Purpose | Lines | Status |
|--------|---------|-------|--------|
| **display.cpp** | E-paper rendering, PNG/BMP decode | 600+ | ✅ Extracted, preserved |
| **wifi.cpp** | WiFi connection, polling | 50+ | ✅ Modular |
| **download.cpp** | HTTP streaming to SPIFFS | 90 | ✅ Modular |
| **sleep.cpp** | Deep sleep, button/timer wakeup | 40 | ✅ Modular |
| **nvram_config.cpp** | NVS persistence (WiFi, API keys) | 360 | ✅ Complete |
| **ap_server.cpp** | AP mode, DNS, HTTP server | 400+ | ✅ Phase 3 complete |
| **trmnl_api_client.h** | TRMNL /api/setup, /api/display | 200+ | ✅ Phase 4 complete |
| **logging.cpp** | Central logging API | 120 | ✅ Phase 6 complete |
| **device_status.cpp** | Device telemetry collection | 90 | ✅ Phase 6 complete |
| **sd_logging.cpp** | SD card daily export | 370 | ✅ Phase 6 complete |
| **power.cpp** | BQ27220 fuel gauge, battery monitoring | 210 | ✅ Phase 5/6 complete |
| **main.cpp** | Boot flow orchestrator | 520 | ✅ Simplified |

## ⚙️ Configuration

Edit `include/config.h` or `src/wifi_config.h`:

```cpp
// Deep sleep interval (seconds)
#define TRMNL_DEEP_SLEEP_SECONDS 10    // 10s for testing, 300+ for production

// Shutdown timer (how long device stays awake)
#define TRMNL_SHUTDOWN_TIMER_SECONDS 60

// WiFi connection timeout
#define WIFI_CONNECT_TIMEOUT 15

// Enable persistent storage
#define NVRAM_ENABLE 1

// Hardcoded fallback (for offline testing)
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define TRMNL_API_KEY ""
```

---

## 🚀 Implementation Roadmap

### Phase 1: Safe Extraction (2-3 hours)
- Extract functions to separate modules
- Keep all logic exactly the same
- Verify tests still pass

### Phase 2: Persistence Layer (2-3 hours)
- Add preferences storage
- Save WiFi credentials
- Save API key and friendly ID

### Phase 3: State Machine (3-4 hours)
- Create main business logic state machine
- Wrap existing functions
- No changes to original implementations

### Phase 4: API Integration (4-5 hours)
- Implement /api/setup and /api/display
- Add to state machine as optional enhancement
- Works with and without API

### Phase 5: Enhanced Features (3-4 hours)
- Add retries with exponential backoff
- Add error logging
- Add battery monitoring

### Phases 6-9: Remaining features
- See FIRMWARE_EVOLUTION_PLAN.md for details

---

## 🔗 Key Insights

### Why the Original Code Breaks Rendering

**The PNG rendering took significant time to get right**:
1. Different image formats (PNG vs BMP)
2. Different color depths (1-bit vs grayscale)
3. Bit-packing in PNG files
4. Proper conversion to framebuffer format
5. Display refresh sequence
6. Temperature compensation

**Even small changes can break it**:
- Off-by-one error in pixel calculation → diagonal lines
- Wrong byte offset → image shifts
- Missing power-on → display blank
- Timing issues → artifacts

**This is why we extract, not rewrite**.

---

### Why We Avoid State Machines Initially

**The pattern in official firmware** (main loop in `bl.c`):
- Processes events in state machine
- Each state calls sub-modules
- Works great for complex devices

**The risk for us**:
- Adding state machine forces refactoring of working code
- If we refactor display rendering → it might break
- We can't afford to break image rendering

**The solution**:
- Extract working code first (safe)
- Add state machine around original functions (wraps it)
- Refactor state machine all we want (doesn't affect core)

---

## 📞 When to Seek Help

### Ask for Code Review When:
- Making changes to critical sections
- Modifying display or WiFi functions
- Changing global variables or structures
- Unsure if refactoring is safe

### Automatic Checks:
- Compile without warnings
- All tests pass
- Serial output clean
- Compare diff: should add code, not remove/modify critical code

### Red Flags:
- 🚨 Modified `display_init()` or PNG rendering loop
- 🚨 Changed WiFi connection logic
- 🚨 Removed error handling or timeouts
- 🚨 Multiple features in one commit
- 🚨 Tests fail after changes

---

## 📖 Related Documentation

- **Official TRMNL Firmware**: https://github.com/usetrmnl/firmware
- **ESP32 Sleep Modes**: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/sleep_modes.html
- **PNGdec Library**: https://github.com/bitbank2/PNGdec
- **EPDiy Display Library**: https://github.com/jonathanmeier/epdiy
- **LilyGo T5 S3 Pro**: Hardware documentation in repo

---

## 📝 Document History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | Dec 15, 2024 | Initial creation with 4 comprehensive docs |

---

## ⚖️ License

These documentation files are part of the TRMNL firmware project.
Code examples shown are educational for safe refactoring patterns.

---

## 🎓 Key Takeaway

**This wasn't just a plan document.**

We learned:
1. **Why** the current code works (mechanics)
2. **What** breaks when you change it (fragile areas)
3. **How** to add features safely (patterns)
4. **When** to extract vs modify (rules)

**With this understanding, you can evolve the firmware from 750 lines in main.cpp into a modular, maintainable production system—without breaking image rendering.**

---

## 🔍 Testing Checklist

Before deployment:

- [x] Firmware compiles without errors or warnings
- [x] Fresh device boots and enters AP mode
- [x] WiFi AP `TRMNL-XXXX` broadcasts correctly
- [x] Captive portal redirects all domains to 192.168.4.1
- [x] Credential form submits and saves to NVS
- [x] Device reboots after credential entry
- [x] Second boot uses saved WiFi credentials
- [x] Auto-registration calls `/api/setup` with MAC
- [x] API key and friendly ID saved to NVS
- [x] Image fetch calls `/api/display` with API key
- [x] PNG image downloads and renders correctly
- [x] Deep sleep enters after display update
- [x] Button press (GPIO48) wakes device
- [x] Timer wakeup works as configured
- [x] Serial "clear" command erases credentials
- [x] AP mode re-enters on next boot after clear

## 🐛 Known Issues

None currently. Firmware is production-ready.

## 📞 Support & Issues

For issues, questions, or contributions:
1. Check existing GitHub issues
2. Review relevant documentation file
3. Open new issue with:
   - Device model and firmware version
   - Reproduction steps
   - Serial output (enable verbose logging)
   - Relevant configuration

## 📖 Documentation Files

- **README.md** (this file) - Overview and quick start
- **FIRMWARE_EVOLUTION_PLAN.md** - Original comprehensive plan (87 KB)
- **CORE_FUNCTIONALITY_PRESERVATION.md** - Safety guidelines (62 KB)
- **HOW_CORE_SYSTEMS_WORK.md** - Technical deep-dive (55 KB)
- **SAFE_REFACTORING_PATTERNS.md** - Refactoring examples (48 KB)

## 📊 Build & Flash Info

- **Platform**: ESP32-S3 (LilyGo T5 4.7")
- **Framework**: Arduino
- **Build System**: PlatformIO
- **Flash Size**: 2 MB (firmware ~1.35 MB, 64.5% utilization)
- **RAM**: 320 KB available (28.4% used after init)
- **Heap**: 8.3MB total (PSRAM + internal), 7.1-8.2MB free
- **Compile Time**: ~7-8 seconds
- **Flash Time**: ~10 seconds

## 🎯 Next Phases (Optional)

Future enhancements not yet implemented:

- **Phase 7**: OTA firmware updates via TRMNL
- **Phase 9**: State machine refactoring

See `FIRMWARE_EVOLUTION_PLAN.md` for detailed specifications.

---

**Status**: 🟢 Production Ready
**Phases Complete**: 1, 2, 3, 4 (partial), 5 (partial), 6
**Last Updated**: December 18, 2025
**Author**: Firmware Development and Testing
**Confidence Level**: 🟢 HIGH - Tested on live hardware with extended operation

---

## 📝 License

This firmware is provided as-is for personal and commercial use.
Based on ESP32 Arduino framework and epdiy library.

Original TRMNL firmware: https://github.com/usetrmnl/firmware
