# TRMNL-T5S3 Epaper Pro Firmware

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


**Build Info (v1.0.0):**
- Flash: 65.1% (1,366,233 / 2,097,152 bytes)
- RAM: 28.5% (93,320 / 327,680 bytes)
- **OTA Rollback Protection**: Active
- **API Log Submission**: Enabled


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

| Module | Purpose | Lines |
|--------|---------|-------|
| **display.cpp** | E-paper rendering, PNG/BMP decode | 600+ |
| **wifi.cpp** | WiFi connection, polling | 50+ | 
| **download.cpp** | HTTP streaming to SPIFFS | 90 | 
| **sleep.cpp** | Deep sleep, button/timer wakeup | 40 | 
| **nvram_config.cpp** | NVS persistence (WiFi, API keys) | 360 | 
| **ap_server.cpp** | AP mode, DNS, HTTP server | 400+ | 
| **trmnl_api_client.h** | TRMNL /api/setup, /api/display | 200+ | 
| **logging.cpp** | Central logging API | 120 | 
| **device_status.cpp** | Device telemetry collection | 90 | 
| **sd_logging.cpp** | SD card daily export | 370 | 
| **power.cpp** | BQ27220 fuel gauge, battery monitoring | 210 | 
| **main.cpp** | Boot flow orchestrator | 520 | 

## ⚙️ Configuration

Edit `include/config.h` or `src/wifi_config.h`:

```cpp
// Deep sleep interval (seconds)
#define TRMNL_DEEP_SLEEP_SECONDS 10    // Fallback if you dont get if from the server

// Shutdown timer (how long device stays awake)
#define TRMNL_SHUTDOWN_TIMER_SECONDS 5  // Extra time to connect your device for testing. Keep at 0 for regular usage

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


## 🐛 Known Issues

GPIO Buttons do not wake from sleep.

## 📞 Support & Issues

For issues, questions, or contributions:
1. Check existing GitHub issues
2. Review relevant documentation file
3. Open new issue with:
   - Device model and firmware version
   - Reproduction steps
   - Serial output (enable verbose logging)
   - Relevant configuration

## 📊 Build & Flash Info

- **Platform**: ESP32-S3 (LilyGo T5 4.7")
- **Framework**: Arduino
- **Build System**: PlatformIO
- **Flash Size**: 2 MB (firmware ~1.35 MB, 64.5% utilization)
- **RAM**: 320 KB available (28.4% used after init)
- **Heap**: 8.3MB total (PSRAM + internal), 7.1-8.2MB free
- **Compile Time**: ~7-8 seconds
- **Flash Time**: ~10 seconds


---

## 📝 License

This firmware is provided as-is for personal and commercial use.
Based on ESP32 Arduino framework and epdiy library.

Original TRMNL firmware: https://github.com/usetrmnl/firmware
