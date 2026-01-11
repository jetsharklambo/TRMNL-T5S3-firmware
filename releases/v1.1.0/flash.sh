#!/bin/bash

# TRMNL-T5S3 v1.1.0 Flash Script
# Automated firmware flashing for LilyGo T5 4.7" E-Paper Display (ESP32-S3)

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FIRMWARE_BIN="$SCRIPT_DIR/firmware.bin"
BOOTLOADER_BIN="$SCRIPT_DIR/bootloader.bin"
PARTITIONS_BIN="$SCRIPT_DIR/partitions.bin"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "  TRMNL-T5S3 Firmware Flash Tool v1.1.0"
echo "=========================================="
echo ""

# Check if esptool.py is installed
if ! command -v esptool.py &> /dev/null; then
    echo -e "${RED}ERROR: esptool.py not found!${NC}"
    echo ""
    echo "Please install esptool.py:"
    echo "  pip install esptool"
    echo ""
    exit 1
fi

# Check if firmware files exist
if [ ! -f "$FIRMWARE_BIN" ]; then
    echo -e "${RED}ERROR: firmware.bin not found in $SCRIPT_DIR${NC}"
    exit 1
fi

if [ ! -f "$BOOTLOADER_BIN" ]; then
    echo -e "${RED}ERROR: bootloader.bin not found in $SCRIPT_DIR${NC}"
    exit 1
fi

if [ ! -f "$PARTITIONS_BIN" ]; then
    echo -e "${RED}ERROR: partitions.bin not found in $SCRIPT_DIR${NC}"
    exit 1
fi

# Detect USB port
echo "Detecting USB port..."
if [ -z "$1" ]; then
    # Auto-detect on macOS/Linux
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        PORT=$(ls /dev/cu.usbserial-* /dev/cu.usbmodem* 2>/dev/null | head -n 1)
    else
        # Linux
        PORT=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -n 1)
    fi

    if [ -z "$PORT" ]; then
        echo -e "${RED}ERROR: Could not auto-detect USB port${NC}"
        echo ""
        echo "Please specify the port manually:"
        echo "  $0 /dev/cu.usbserial-XXXX    (macOS)"
        echo "  $0 /dev/ttyUSB0              (Linux)"
        echo "  $0 COM3                      (Windows Git Bash)"
        echo ""
        exit 1
    fi
else
    PORT="$1"
fi

echo -e "${GREEN}Using port: $PORT${NC}"
echo ""

# Confirm before flashing
echo -e "${YELLOW}WARNING: This will erase all data on the device!${NC}"
echo "Press ENTER to continue or Ctrl+C to cancel..."
read

echo ""
echo "Flashing firmware..."
echo "=========================================="

# Flash bootloader at 0x0
echo -e "${GREEN}[1/3] Flashing bootloader...${NC}"
esptool.py --chip esp32s3 --port "$PORT" --baud 921600 \
    write_flash 0x0 "$BOOTLOADER_BIN"

# Flash partition table at 0x8000
echo -e "${GREEN}[2/3] Flashing partition table...${NC}"
esptool.py --chip esp32s3 --port "$PORT" --baud 921600 \
    write_flash 0x8000 "$PARTITIONS_BIN"

# Flash firmware at 0x10000
echo -e "${GREEN}[3/3] Flashing firmware...${NC}"
esptool.py --chip esp32s3 --port "$PORT" --baud 921600 \
    write_flash 0x10000 "$FIRMWARE_BIN"

echo ""
echo "=========================================="
echo -e "${GREEN}✅ Flash complete!${NC}"
echo "=========================================="
echo ""
echo "Next steps:"
echo "  1. Unplug and replug the device"
echo "  2. Device will boot into AP mode (TRMNL-XXXX)"
echo "  3. Connect to WiFi AP and configure credentials"
echo "  4. Device will auto-register and display content"
echo ""
echo "To monitor serial output:"
echo "  screen $PORT 115200"
echo "  (Press Ctrl+A then K to exit screen)"
echo ""
