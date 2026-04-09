# Building TRMNL Firmware from Source

Complete guide to building and customizing the TRMNL firmware using PlatformIO in Visual Studio Code.

---

## Prerequisites

### Software

**Required**:
- **Visual Studio Code** - [Download](https://code.visualstudio.com/)
- **PlatformIO Extension** - Install from VSCode Extensions Marketplace

**Optional**:
- **Git** - For cloning repository (or download ZIP from GitHub)

### Hardware

- **LilyGo T5 4.7" E-Paper Display (ESP32-S3)** - The target device
- **USB-C Cable** - Must support data transfer (not charge-only)
- **Computer** - Windows, macOS, or Linux

---

## Setup

### 1. Install Visual Studio Code

Download and install VSCode from [https://code.visualstudio.com/](https://code.visualstudio.com/)

### 2. Install PlatformIO Extension

1. Open VSCode
2. Click Extensions icon in sidebar (or press `Ctrl+Shift+X` / `Cmd+Shift+X`)
3. Search for "PlatformIO IDE"
4. Click "Install" on the official PlatformIO extension
5. Wait for installation to complete
6. Reload VSCode if prompted

### 3. Clone or Download Repository

**Option A: Using Git**
```bash
git clone https://github.com/jetsharklambo/TRMNL-T5S3-firmware.git
cd TRMNL-T5S3-firmware
```

**Option B: Download ZIP**
1. Visit [GitHub repository](https://github.com/jetsharklambo/TRMNL-T5S3-firmware)
2. Click "Code" → "Download ZIP"
3. Extract the ZIP file
4. Note the extracted folder location

### 4. Open Project in VSCode

1. Open VSCode
2. File → Open Folder
3. Navigate to the repository folder
4. Click "Open"

PlatformIO will automatically:
- Detect the project (via `platformio.ini`)
- Download the ESP32 platform
- Install required libraries
- Set up build tools

**Note**: First-time setup downloads ~2GB of tools and libraries. Be patient!

---

## Configuration

### System Configuration (`include/config.h`)

Edit this file to customize firmware behavior:

```cpp
// Example: Change deep sleep interval
#define TRMNL_DEEP_SLEEP_SECONDS 300  // 5 minutes

// Example: Enable display rotation
#define DISPLAY_ROTATION 180  // Upside down

// Example: Disable OTA updates
#define OTA_AUTO_UPDATE 0  // Manual only
```

**All available settings** are documented in [README.md Configuration section](../README.md#configuration).

### WiFi & API Configuration (`src/wifi_config.h`)

⚠️ **IMPORTANT**: This file contains your WiFi credentials and should NOT be committed to version control.

Create or edit this file:

```cpp
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// WiFi credentials (for development/testing only)
// In production, device uses AP mode for user-entered credentials
#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PASSWORD "YourPassword"

// TRMNL API configuration (optional - device auto-registers)
#define TRMNL_API_URL "https://usetrmnl.com"
#define DEVICE_API_KEY ""  // Leave empty - device auto-registers

#endif
```

**For end users**: Leave WiFi credentials empty. The device will enter AP mode and prompt for WiFi setup on first boot.

**For developers**: You can hardcode credentials for faster testing iterations.

---

## Building

### Using VSCode UI (Recommended)

1. **Open PlatformIO Panel**
   - Click the PlatformIO icon in the left sidebar
   - Or View → Command Palette → "PlatformIO: Home"

2. **Build the Firmware**
   - Under "PROJECT TASKS"
   - Expand your project name
   - Expand "T5_E_PAPER_S3_V7"
   - Click "Build"
   - Or click the checkmark (✓) icon in the bottom toolbar

3. **Watch the Build**
   - Terminal opens showing compilation progress
   - Build takes ~30-60 seconds (first build longer)
   - Success message: `SUCCESS`
   - Firmware location: `.pio/build/T5_E_PAPER_S3_V7/firmware.bin`

### Using Command Line

```bash
# Build firmware
~/.platformio/penv/bin/platformio run

# Or use the PlatformIO CLI shortcut
pio run
```

**Build Output**:
```
Processing T5_E_PAPER_S3_V7 (platform: espressif32@6.5.0; board: T5-ePaper-S3)
--------------------------------------------------------------------------------
RAM:   [===       ]  29.7% (used 97268 bytes from 327680 bytes)
Flash: [=====     ]  54.8% (used 1149681 bytes from 2097152 bytes)

========================= [SUCCESS] Took 21.72 seconds =========================
```

### Build Targets

| Target | Command | Description |
|--------|---------|-------------|
| Build | `pio run` | Compile firmware only |
| Upload | `pio run -t upload` | Build and flash to device |
| Clean | `pio run -t clean` | Delete build artifacts |
| Monitor | `pio device monitor` | Open serial monitor (115200 baud) |

---

## Flashing

### Prepare Device

**Enter Bootloader Mode** (Choose one method):

**Hold BOOT while plugging in**
1. Press and hold the **BOOT** button
2. While holding BOOT, plug in the USB-C cable
3. Release the **BOOT** button
4. Device is now in bootloader mode


### Flash Using VSCode

1. **Connect Device**
   - Plug in device via USB-C (or enter bootloader mode if not already)

2. **Upload Firmware**
   - PlatformIO Panel → PROJECT TASKS → Upload
   - Or click the arrow (→) icon in the bottom toolbar

3. **Watch Upload Progress**
   - Terminal shows upload progress
   - Takes 30-90 seconds
   - Success message: `SUCCESS`

4. **Reset Device**
   - Press the **RESET** button on device
   - Or unplug and replug USB cable
   - Device boots into new firmware

### Flash Using Command Line

```bash
# Auto-detect port and upload
pio run -t upload

# Specify port manually (macOS example)
pio run -t upload --upload-port /dev/cu.usbmodem14101

# Windows example
pio run -t upload --upload-port COM3
```

### Flash Using Scripts

The repository includes flash scripts that flash bootloader, partitions, and firmware separately:

**macOS/Linux**:
```bash
# Make executable (first time only)
chmod +x flash_firmware.sh

# Flash firmware (replace port with your serial port)
./flash_firmware.sh /dev/cu.usbmodem14101
```

**Windows**:
```bash
flash_firmware.bat COM3
```

**Find Your Serial Port**:

```bash
# macOS
ls /dev/cu.usbmodem* /dev/cu.usbserial*

# Linux
ls /dev/ttyACM* /dev/ttyUSB*

# Windows PowerShell
Get-WmiObject Win32_SerialPort | Select-Object DeviceID,Description
```

---

## Monitoring & Debugging

### Serial Monitor

**Using VSCode**:
1. PlatformIO Panel → PROJECT TASKS → Monitor
2. Or click the power plug icon in the bottom toolbar
3. Baud rate: 115200 (configured in `platformio.ini`)

**Using Command Line**:
```bash
pio device monitor
```

**Keyboard Shortcuts**:
- `Ctrl+C` - Exit monitor
- `Ctrl+T Ctrl+H` - Show help

### Log Output

The firmware produces verbose logging prefixed by category:

```
[INIT] Initializing display (FastEPD)...
[WIFI] Starting connection...
[API] Calling /api/setup
[IMAGE] Loading: /spiffs/display.bmp
[DISPLAY] Drawing WiFi error icon (BB_RECT update)
[SLEEP] Entering deep sleep for 300 seconds
```

### Debug Commands

**Factory Reset** (via serial monitor):
1. Connect serial monitor (115200 baud)
2. Press RESET button on device
3. Within 5 seconds of boot, type: `clear`
4. Device erases credentials and reboots to AP mode

---

## Customization Examples

### Example 1: Change Sleep Interval to 1 Hour

**Edit `include/config.h`**:
```cpp
// Change from 10 seconds (testing) to 3600 seconds (1 hour)
#define TRMNL_DEEP_SLEEP_SECONDS 3600
```

**Rebuild and flash**:
```bash
pio run -t upload
```

### Example 2: Flip Display Upside Down

**Edit `include/config.h`**:
```cpp
// Change from 0 (normal) to 180 (inverted)
#define DISPLAY_ROTATION 180
```

**Rebuild and flash**.

### Example 3: Disable Auto-Updates

**Edit `include/config.h`**:
```cpp
// Change from 1 (enabled) to 0 (disabled)
#define OTA_AUTO_UPDATE 0
```

Now updates must be triggered manually via API.

### Example 4: Adjust Image Position

**Edit `include/config.h`**:
```cpp
// Change horizontal offset (positive = shift right)
#define IMAGE_X_OFFSET 5  // Was 3, now 5 pixels
```

Useful if images are too close to the bezel.

---

## Troubleshooting

### "Build Failed" or Compilation Errors

**Check**:
- PlatformIO extension is installed and active
- Internet connection (for downloading libraries)
- No syntax errors in `config.h` or `wifi_config.h`

**Solution**:
```bash
# Clean and rebuild
pio run -t clean
pio run
```

### "Upload Failed" or "Port Not Found"

**Check**:
- Device is connected via USB
- Device is in bootloader mode (hold BOOT button)
- USB cable supports data transfer (not charge-only)
- No other program has serial port open (Arduino IDE, screen, etc.)

**macOS/Linux Permissions**:
```bash
# Add your user to dialout group (Linux)
sudo usermod -a -G dialout $USER
# Logout and login for changes to take effect

# Grant permission to port (macOS, if needed)
sudo chmod 666 /dev/cu.usbmodem*
```

**Windows Drivers**:
- Install [CP210x USB drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

### "Out of Memory" During Build

**Solution**:
- Close other applications
- Increase system virtual memory
- Use command line build instead of VSCode

### Library Dependency Issues

**Force reinstall dependencies**:
```bash
# Remove library cache
rm -rf .pio/libdeps

# Rebuild (downloads fresh)
pio run
```

### PlatformIO Not Detecting Project

**Check**:
- `platformio.ini` exists in project root
- Folder opened in VSCode is the project root (not a parent/child folder)

**Solution**:
```bash
# Reinitialize project
pio init --ide vscode
```

---

## File Structure

```
TRMNL-T5S3-firmware/
├── include/
│   ├── config.h              # System configuration (EDIT THIS)
│   └── *.h                   # Header files
├── src/
│   ├── main.cpp              # Main firmware logic
│   ├── wifi_config.h         # WiFi/API credentials (EDIT THIS)
│   ├── display.cpp           # Display driver
│   ├── wifi.cpp              # WiFi management
│   ├── download.cpp          # Image downloading
│   ├── ota.cpp               # OTA updates
│   └── *.cpp                 # Other modules
├── lib/                      # Local libraries (unused - using PlatformIO registry)
├── docs/                     # Documentation
│   ├── BUILD_FIRMWARE.md     # This file
│   └── ...
├── .pio/                     # PlatformIO build artifacts (auto-generated)
│   └── build/T5_E_PAPER_S3_V7/
│       ├── firmware.bin      # Compiled firmware
│       ├── bootloader.bin    # ESP32 bootloader
│       └── partitions.bin    # Partition table
├── platformio.ini            # PlatformIO configuration
├── partitions.csv            # Custom partition table
├── flash_firmware.sh         # Flash script (macOS/Linux)
├── flash_firmware.bat        # Flash script (Windows)
└── README.md                 # User documentation
```

---

## Advanced Topics

### Custom Partition Table

The project uses a custom partition table (`partitions.csv`) for larger SPIFFS storage:

```csv
nvs,      data, nvs,     0x9000,  0x6000,
phy_init, data, phy,     0xf000,  0x1000,
factory,  app,  factory, 0x10000, 0x140000,
ota_0,    app,  ota_0,   0x150000,0x140000,
ota_1,    app,  ota_1,   0x290000,0x140000,
spiffs,   data, spiffs,  0x3D0000,0xC30000,
```

**OTA partitions**: Two app partitions (ota_0, ota_1) for safe firmware updates
**SPIFFS**: Large storage for images and logs (12MB)

### Adding Custom Libraries

**Edit `platformio.ini`**:
```ini
lib_deps =
    https://github.com/bitbank2/FastEPD.git#3e47f1e
    bitbank2/PNGdec@1.1.6
    bblanchon/ArduinoJson@6.21.3
    your-library/YourLib@1.0.0  # Add here
```

### Changing Board or Platform

**Edit `platformio.ini`**:
```ini
[env:T5_E_PAPER_S3_V7]
platform = espressif32@6.5.0    # Change ESP32 platform version
board = T5-ePaper-S3            # Change board type
framework = arduino
```

**Note**: Different boards may require GPIO pin changes in source code.

---

## Contributing

See [CONTRIBUTING.md](developer/CONTRIBUTING.md) for guidelines on:
- Code style
- Commit messages
- Pull request process
- Testing requirements

---

## Resources

**Official Documentation**:
- [PlatformIO Docs](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/)
- [FastEPD Library](https://github.com/bitbank2/FastEPD)

**Hardware**:
- [LilyGo T5S3 Hardware Repo](https://github.com/Xinyuan-LilyGO/T5S3-4.7-e-paper-PRO)
- [T5S3 Schematics](https://github.com/Xinyuan-LilyGO/T5S3-4.7-e-paper-PRO/tree/main/schematic)

**TRMNL Platform**:
- [TRMNL Website](https://usetrmnl.com)
- [TRMNL Documentation](https://docs.trmnl.com)

---

## Support

**Issues**: [GitHub Issues](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues)

**When reporting build issues, include**:
- Operating system and version
- PlatformIO version (`pio --version`)
- Full error message from build output
- Contents of `platformio.ini`
