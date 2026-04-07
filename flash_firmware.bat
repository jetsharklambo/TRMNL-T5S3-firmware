@echo off
REM Flash TRMNL T5S3 FastEPD Firmware (Windows)
REM This script flashes all three components to the correct addresses

if "%1"=="" (
    echo Usage: flash_firmware.bat ^<port^>
    echo Example: flash_firmware.bat COM3
    exit /b 1
)

set PORT=%1

echo ====================================
echo TRMNL T5S3 FastEPD Firmware Flasher
echo ====================================
echo Port: %PORT%
echo.

REM Check if files exist
if not exist ".pio\build\T5_E_PAPER_S3_V7\bootloader.bin" (
    echo Error: bootloader.bin not found. Run 'platformio run' first.
    exit /b 1
)

if not exist ".pio\build\T5_E_PAPER_S3_V7\partitions.bin" (
    echo Error: partitions.bin not found. Run 'platformio run' first.
    exit /b 1
)

if not exist ".pio\build\T5_E_PAPER_S3_V7\firmware.bin" (
    echo Error: firmware.bin not found. Run 'platformio run' first.
    exit /b 1
)

echo Flashing firmware...
echo.

esptool.py --chip esp32s3 ^
  --port %PORT% ^
  --baud 460800 ^
  --before default_reset ^
  --after hard_reset ^
  write_flash -z ^
  --flash_mode dio ^
  --flash_freq 80m ^
  --flash_size 16MB ^
  0x0 .pio\build\T5_E_PAPER_S3_V7\bootloader.bin ^
  0x8000 .pio\build\T5_E_PAPER_S3_V7\partitions.bin ^
  0x10000 .pio\build\T5_E_PAPER_S3_V7\firmware.bin

if %errorlevel% equ 0 (
    echo.
    echo ====================================
    echo Success! Flash successful!
    echo ====================================
    echo.
    echo Next steps:
    echo 1. Press reset button on the device
    echo 2. Open serial monitor (115200 baud^) to view logs
    echo 3. Look for [DIAGNOSTIC] messages to see display dimensions
    echo.
) else (
    echo.
    echo ====================================
    echo Error! Flash failed!
    echo ====================================
    echo.
    echo Troubleshooting:
    echo 1. Check USB cable connection
    echo 2. Try holding BOOT button while flashing
    echo 3. Check port name is correct
    echo.
    exit /b 1
)
