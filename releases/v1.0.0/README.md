# TRMNL T5S3 Firmware v1.1.0 - Installation Guide

## Quick Install (Easiest Method - No Software Required!)

Flash your TRMNL firmware directly from your web browser in 3 simple steps:

### Prerequisites
- **Chrome, Edge, or Opera browser** (Web Serial API required - Firefox not supported)
- **USB-C cable** (must support data transfer, not just charging)
- **LilyGo T5 4.7" E-Paper Display** (ESP32-S3)

### Installation Steps

1. **Download the firmware**
   - Download `trmnl-t5s3-factory.bin` from this release

2. **Open the web flasher**
   - Visit: **[esptool.spacehuhn.com](https://esptool.spacehuhn.com/)**

3. **Connect your device**
   - Connect your T5 device via USB-C cable
   - Click **"Connect"** button in the web tool
   - Select your device's serial port from the popup
     - macOS: Usually shows as `/dev/cu.usbserial-XXXX` or `/dev/cu.usbmodem-XXXX`
     - Windows: Usually shows as `COM3`, `COM4`, etc.
     - Linux: Usually shows as `/dev/ttyUSB0` or `/dev/ttyACM0`

4. **Flash the firmware**
   - In the web tool interface:
     - **Address**: `0x0` (or just `0`)
     - **File**: Click "Choose File" and select `trmnl-t5s3-factory.bin`
   - Click **"Program"** or **"Flash"** button
   - Wait for the upload to complete (usually 30-60 seconds)

5. **Done!**
   - Unplug and replug your device (or press the reset button)
   - Your TRMNL device will boot up and enter AP mode on first boot
   - Connect to WiFi network `TRMNL-XXXX` to configure

### Troubleshooting

**No serial port appears when clicking "Connect":**
- Check your USB cable - many cables are charge-only and don't support data
- Try a different USB port
- On Windows, you may need [CP210x drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

**Upload fails or times out:**
- Lower the baud rate in settings to 115200 (default is usually 921600)
- Try pressing and holding the BOOT button while connecting
- Ensure no other program (like Arduino IDE or PlatformIO) has the serial port open

**Device doesn't boot after flashing:**
- Unplug and replug the device
- Press the RESET button on the device
- Re-flash the firmware at address `0x0`

### Alternative Installation Methods

If you can't use the web flasher (e.g., using Firefox), see the [main README](../../README.md) for alternative installation methods using command-line tools.

---

## What's Included

This single factory binary contains:
- **Bootloader** (ESP32-S3)
- **Partition Table**
- **TRMNL Firmware v1.1.0**

All pre-configured and ready to flash at address `0x0`.

---

## Next Steps After Installation

1. **First Boot**: Device enters AP mode automatically
2. **Connect**: Join WiFi network `TRMNL-XXXX` (password not required)
3. **Configure**: Browser will auto-redirect to setup portal (or visit `192.168.4.1`)
4. **Enter WiFi**: Provide your WiFi credentials
5. **Auto-Register**: Device connects to TRMNL backend and registers automatically
6. **Enjoy**: Your content will download and display!

For detailed firmware features and troubleshooting, see the [main documentation](../../README.md).
