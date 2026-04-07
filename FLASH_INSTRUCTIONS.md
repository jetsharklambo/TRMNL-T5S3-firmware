# Flashing Instructions - TRMNL T5S3 FastEPD Firmware

## Current Build: Diagnostic Firmware

**Purpose:** Testing double-rendering theory to diagnose the repeated strip issue.

**New Diagnostic Features:**
- Reports display width/height after setMode()
- Detects dimension mismatches (should be 960x540)
- Options to skip clearWhite() or test different refresh modes
- Look for `[DIAGNOSTIC]` messages in serial output

## Recommended: Flash Script (Works Every Time!)

The provided flash scripts flash all components (bootloader, partitions, firmware) to the correct addresses.

### macOS/Linux

```bash
./flash_firmware.sh /dev/cu.usbmodem14101
```

Replace `/dev/cu.usbmodem14101` with your serial port:
- macOS: `/dev/cu.usbmodem*`
- Linux: `/dev/ttyACM0` or `/dev/ttyUSB0`

### Windows

```cmd
flash_firmware.bat COM3
```

Replace `COM3` with your serial port (check Device Manager).

---

## Alternative: Web Flasher (Doesn't Always Work)

### Option 1: ESP Web Flasher

1. Visit: **https://espressif.github.io/esptool-js/**
2. Connect your LilyGo T5S3 Pro via USB-C
3. Click **"Connect"** and select the USB serial port
4. Click **"Choose File"** and select `firmware-fastepd-production.bin`
5. Set **Flash Address:** `0x0`
6. Click **"Program"**
7. Wait for flash to complete (~30 seconds)

### Option 2: Adafruit ESP Web Flasher

1. Visit: **https://adafruit.github.io/Adafruit_WebSerial_ESPTool/**
2. Connect device via USB-C
3. Click **"Connect"**
4. Select `firmware-fastepd-production.bin` at offset `0x0`
5. Click **"Program"**

### Option 3: Command Line (esptool)

```bash
esptool --chip esp32s3 --port /dev/ttyACM0 write_flash 0x0 firmware-fastepd-production.bin
```

Replace `/dev/ttyACM0` with your serial port (macOS: `/dev/cu.usbmodem*`, Windows: `COM*`)

---

## Firmware Details

**Board:** LilyGo T5S3 Pro (ESP32-S3)
**Display:** 960x540 e-paper
**Library:** FastEPD 1.3.0
**Flash Size:** 16 MB
**Flash Mode:** QIO
**Flash Frequency:** 80 MHz

**Memory Usage:**
- RAM: 29.7% (97,252 bytes / 327,680 bytes)
- Flash: 54.5% (1,143,233 bytes / 2,097,152 bytes)

---

## After Flashing

1. **Reset the device** (press reset button or power cycle)
2. **Connect to serial monitor** (115200 baud) to see debug output
3. **Test display functions:**
   - Display should initialize with FastEPD
   - Check for `[INIT] Initializing display (FastEPD)...` in logs

---

## Troubleshooting

**Device not detected:**
- Make sure USB-C cable supports data (not just power)
- Install CH340/CP2102 drivers if needed
- Try different USB port

**Flash fails:**
- Hold BOOT button while connecting USB
- Press RESET button, then try flashing again

**Display issues:**
- Check serial output for errors
- Verify display is properly seated
- Check for `[DISPLAY]` and `[IMAGE]` debug logs

---

## Migration from EPDiy

This firmware migrates from EPDiy library to FastEPD library:

**Code reduction:**
- `display.cpp`: 751 lines → 456 lines (39% reduction)
- `display_init()`: 8 lines → 2 lines
- `display_image()`: 225 lines → ~70 lines

**Benefits:**
- Simpler API (bbep object methods)
- Automatic PNG pixel handling
- Better upstream compatibility
- Easier maintenance

---

## Next Steps

1. Flash firmware
2. Test all display functions
3. Report any issues with display rotation or rendering
4. Proceed with upstream integration once validated

For issues, see: https://github.com/usetrmnl/trmnl-firmware/issues
