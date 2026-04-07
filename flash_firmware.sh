#!/bin/bash
# Flash TRMNL T5S3 FastEPD Firmware
# This script flashes all three components (bootloader, partitions, firmware) to the correct addresses

# Detect serial port
if [ -z "$1" ]; then
    echo "Usage: ./flash_firmware.sh <port>"
    echo "Example (macOS): ./flash_firmware.sh /dev/cu.usbmodem14101"
    echo "Example (Linux): ./flash_firmware.sh /dev/ttyACM0"
    echo "Example (Windows): ./flash_firmware.sh COM3"
    exit 1
fi

PORT=$1

echo "===================================="
echo "TRMNL T5S3 FastEPD Firmware Flasher"
echo "===================================="
echo "Port: $PORT"
echo ""

# Check if files exist
if [ ! -f ".pio/build/T5_E_PAPER_S3_V7/bootloader.bin" ]; then
    echo "Error: bootloader.bin not found. Run 'platformio run' first."
    exit 1
fi

if [ ! -f ".pio/build/T5_E_PAPER_S3_V7/partitions.bin" ]; then
    echo "Error: partitions.bin not found. Run 'platformio run' first."
    exit 1
fi

if [ ! -f ".pio/build/T5_E_PAPER_S3_V7/firmware.bin" ]; then
    echo "Error: firmware.bin not found. Run 'platformio run' first."
    exit 1
fi

echo "Flashing firmware..."
echo ""

esptool.py --chip esp32s3 \
  --port "$PORT" \
  --baud 460800 \
  --before default_reset \
  --after hard_reset \
  write_flash -z \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size 16MB \
  0x0 .pio/build/T5_E_PAPER_S3_V7/bootloader.bin \
  0x8000 .pio/build/T5_E_PAPER_S3_V7/partitions.bin \
  0x10000 .pio/build/T5_E_PAPER_S3_V7/firmware.bin

if [ $? -eq 0 ]; then
    echo ""
    echo "===================================="
    echo "✓ Flash successful!"
    echo "===================================="
    echo ""
    echo "Next steps:"
    echo "1. Press reset button on the device"
    echo "2. Open serial monitor (115200 baud) to view logs"
    echo "3. Look for [DIAGNOSTIC] messages to see display dimensions"
    echo ""
else
    echo ""
    echo "===================================="
    echo "✗ Flash failed!"
    echo "===================================="
    echo ""
    echo "Troubleshooting:"
    echo "1. Check USB cable connection"
    echo "2. Try holding BOOT button while flashing"
    echo "3. Check port name is correct"
    echo ""
    exit 1
fi
