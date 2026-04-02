# TRMNL Firmware for LilyGo T5S3 Pro

Custom firmware for the LilyGo T5 4.7" E-Paper Display (ESP32-S3) with WiFi provisioning, auto-registration, and TRMNL cloud integration.

**Firmware Version**: 1.1.0
**Last Updated**: April 1, 2026

## Features

- **Zero-Configuration Setup**: Connect via WiFi AP on first boot, enter your WiFi credentials, and you're done
- **Auto-Registration**: Device automatically registers with TRMNL backend using its MAC address
- **Remote Content Updates**: Downloads and displays content from your TRMNL dashboard
- **Deep Sleep Power Management**: Optimized for long battery life (~1 week with 30-minute refresh intervals)
- **Over-the-Air Updates**: Firmware updates delivered automatically from TRMNL
- **4.7" E-Paper Display**: Beautiful 960x540 grayscale display with excellent outdoor visibility
- **Battery Monitoring**: Real-time battery status with intelligent power management

## What You Need

### Hardware
- **LilyGo T5 4.7" E-Paper Display** (ESP32-S3) - [Purchase Link](https://www.lilygo.cc/products/t5-4-7-inch-e-paper-v2-3)
- **USB-C cable** (must support data transfer, not just charging)
- **Computer** (Windows, macOS, or Linux)

### Software
- **Chrome, Edge, or Opera browser** (for web-based installation)
- OR **esptool** (if using Firefox or prefer command-line)

## Installation

Choose one of the two methods below:

---

### Method 1: Web Flasher (Recommended)

**Easiest method - no software installation required!**

1. **Download the firmware**
   - Download the latest release from the [releases page](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/releases)
   - Get the file: `trmnl-t5s3-factory.bin`

2. **Open the web flasher**
   - Visit: **[esptool.spacehuhn.com](https://esptool.spacehuhn.com/)**
   - **Note**: Must use Chrome, Edge, or Opera (Firefox not supported)

3. **Connect your device**
   - Plug in your T5 device via USB-C cable
   - Click **"Connect"** in the web tool
   - Select your device's serial port from the list:
     - **macOS**: `/dev/cu.usbserial-XXXX` or `/dev/cu.usbmodem-XXXX`
     - **Windows**: `COM3`, `COM4`, etc.
     - **Linux**: `/dev/ttyUSB0` or `/dev/ttyACM0`

4. **Flash the firmware**
   - **Address**: Enter `0` (or `0x0`)
   - **File**: Click "Choose File" and select `trmnl-t5s3-factory.bin`
   - Click **"Program"** or **"Flash"**
   - Wait 30-60 seconds for upload to complete

5. **Done!**
   - Unplug and replug the device (or press reset button)
   - Continue to "First Boot Setup" below

---

### Method 2: Command Line

**For Firefox users or those who prefer the command line.**

#### Prerequisites

Install esptool via Python:
```bash
pip install esptool
```

**Windows users**: You may need [CP210x drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

#### Installation Steps

1. **Download the firmware**
   - Get `trmnl-t5s3-factory.bin` from the [releases page](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/releases)

2. **Find your USB port**
   ```bash
   # macOS
   ls /dev/cu.usbserial-* /dev/cu.usbmodem*

   # Linux
   ls /dev/ttyUSB* /dev/ttyACM*

   # Windows (PowerShell)
   Get-WmiObject Win32_SerialPort | Select-Object DeviceID,Description
   ```

3. **Flash the firmware**

   Replace `/dev/cu.usbserial-XXXX` with your actual port:

   ```bash
   esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
     write_flash 0x0 trmnl-t5s3-factory.bin
   ```

   **Windows example**:
   ```bash
   esptool --chip esp32s3 --port COM3 --baud 921600 write_flash 0x0 trmnl-t5s3-factory.bin
   ```

4. **Done!**
   - Unplug and replug the device
   - Continue to "First Boot Setup" below

---

## First Boot Setup

After flashing the firmware, follow these steps to get your device connected:

### 1. Device Boots in AP Mode

- Unplug and replug the device (or press reset button)
- Device boots and enters AP mode (no WiFi credentials stored yet)
- **Important**: Display shows the device's **MAC address** - write this down!
  - Format: `D8:3B:DA:XX:XX:XX`
  - You'll need this in the next step

### 2. Register Device in TRMNL Dashboard

**Before connecting the device to WiFi, add it to your TRMNL account:**

1. Visit the [TRMNL DIY/BYOD Guide](https://docs.trmnl.com/go/diy/byod)
2. Go to your device settings in the TRMNL dashboard
3. Under **Developer Perks** section, add your device's MAC address (from the display)
4. Configure which plugins/content you want to display

**Why register first?** When you register the MAC address before WiFi setup, your device will automatically link to your TRMNL account and start displaying your content!

### 3. Connect to Device WiFi

- Connect to the WiFi network: `TRMNL-XXXX` (where XXXX are the last bytes of the MAC address)
- No password required
- Your phone/computer may say "No Internet" - this is normal

### 4. Open Setup Page

- Browser should auto-redirect to the setup portal
- If not, manually visit: `http://192.168.4.1`

### 5. Enter Your WiFi Credentials

- **SSID**: Your home WiFi network name (must be 2.4GHz - ESP32 doesn't support 5GHz)
- **Password**: Your WiFi password
- Click "Connect"

### 6. Device Auto-Configures

- Device reboots and connects to your WiFi network
- Auto-registers with TRMNL backend using the MAC address you registered
- Downloads and displays your content
- Enters deep sleep mode

### 7. Verify on Dashboard

- Visit [usetrmnl.com](https://usetrmnl.com)
- Your device should now be online and displaying content
- The device will automatically refresh based on your configured interval

## Battery Life

- **Expected battery life**: ~1 week with 30-minute refresh intervals
- Battery life depends on:
  - Refresh frequency (configured in your TRMNL dashboard)
  - WiFi signal strength (weaker signal = more power)
  - Display update frequency
  - Temperature (colder = slightly shorter battery life)

**Power-saving tips**:
- Set longer refresh intervals in your TRMNL dashboard (60+ minutes)
- Place device closer to WiFi router for better signal
- Keep firmware up to date for latest power optimizations

## Features After Setup

Once your device is set up and running:

- **Automatic content updates**: Device wakes up at configured intervals to fetch new content
- **Over-the-air firmware updates**: Firmware automatically updates when new versions are available
- **Battery monitoring**: View battery status in your TRMNL dashboard
- **Button wake**: Press the button to manually trigger a content refresh
- **Deep sleep**: Device sleeps between updates to maximize battery life

## Troubleshooting

### Installation Issues

**"No serial port appears when I click Connect"**
- Check your USB cable - many cables are charge-only and don't support data transfer
- Try a different USB port on your computer
- On Windows, install [CP210x drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- Make sure no other program has the serial port open (Arduino IDE, PlatformIO, etc.)

**"Flash failed" or "Timeout waiting for packet header"**
- Lower the baud rate to `115200` (default is usually `921600`)
- Press and hold the BOOT button on the device while starting the flash
- Try a different USB cable or USB port
- On macOS, try both `/dev/cu.*` and `/dev/tty.*` variants of the port

**"Device doesn't boot after flashing"**
- Unplug and replug the USB cable
- Press the RESET button on the device
- Re-flash the firmware at address `0x0`

### Setup Issues

**"Can't connect to TRMNL-XXXX network"**
- Wait 10-15 seconds after device boots for AP to start
- Forget any previously saved "TRMNL-XXXX" networks on your phone/computer
- Try resetting the device and waiting for it to fully boot
- Check the serial output to verify AP mode started

**"Setup page doesn't load"**
- Manually navigate to `http://192.168.4.1`
- Disable mobile data on your phone (if using phone to configure)
- Try a different device (phone vs laptop)
- Clear browser cache and try again

**"Device won't connect to my WiFi"**
- Verify your WiFi password is correct
- Make sure you're entering the 2.4GHz network name (ESP32 doesn't support 5GHz)
- Check that your WiFi network is broadcasting its SSID (not hidden)
- Move device closer to your WiFi router during setup

### Operation Issues

**"Display not updating"**
- Check battery level - device won't update if battery is too low (<20%)
- Verify device is showing in your TRMNL dashboard
- Check that you have content configured in your dashboard
- Press the button to force a manual refresh
- Check WiFi signal strength (move closer to router if needed)

**"Device keeps entering AP mode"**
- WiFi credentials may be incorrect - reconnect and re-enter
- WiFi network may be unavailable or moved out of range
- After 3 failed WiFi attempts, device auto-clears credentials and re-enters AP mode

### Factory Reset

To completely erase all settings and start fresh:

**Option 1: Serial Command (Easiest)**
1. Connect device via USB
2. Open serial monitor (115200 baud)
3. Press RESET button on device
4. Within 5 seconds of boot, type: `clear`
5. Device will erase credentials and reboot to AP mode

**Option 2: Complete Flash Erase**
```bash
esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX erase_flash
```
Then re-flash the firmware.

## For Developers

If you want to build from source or modify the firmware:

1. Install PlatformIO
2. Clone the repository
3. Build: `~/.platformio/penv/bin/platformio run`
4. Flash: `~/.platformio/penv/bin/platformio run -t upload`
5. Monitor: `~/.platformio/penv/bin/platformio device monitor`

See the included technical documentation for development details.

## Support & Issues

**Need help?**
1. Check the troubleshooting section above
2. Visit the [GitHub issues page](https://github.com/jetsharklambo/TRMNL-T5S3-firmware/issues)
3. Join the TRMNL community for support

**When reporting an issue, please include:**
- Device model (LilyGo T5 4.7" S3 Pro)
- Firmware version (shown on device during boot)
- What you were trying to do
- What happened instead
- Serial output (if possible)

## Credits

- **TRMNL**: [usetrmnl.com](https://usetrmnl.com)
- **LilyGo T5 Hardware**: [github.com/Xinyuan-LilyGO/T5S3-4.7-e-paper-PRO](https://github.com/Xinyuan-LilyGO/T5S3-4.7-e-paper-PRO)
- **EPDiy Display Library**: E-paper display driver
- **ESP32 Arduino Framework**: Espressif Systems

## License

This firmware is provided as-is for personal and commercial use.
