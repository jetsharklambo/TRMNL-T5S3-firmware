#!/usr/bin/env python3
"""
Build Factory Image Script for TRMNL T5S3 Firmware

This script merges the bootloader, partition table, and firmware into a single
factory image that can be flashed to address 0x0.

Usage:
    python3 build_factory_image.py

Output:
    trmnl-t5s3-v1.1.0-patched.bin - Complete factory image

Flash command:
    esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \
        write_flash 0x0 trmnl-t5s3-v1.1.0-patched.bin
"""

import os
import sys

def build_factory_image():
    """Build a complete factory image from PlatformIO build artifacts."""

    build_dir = '.pio/build/T5_E_PAPER_S3_V7'

    # Check if build artifacts exist
    required_files = [
        f'{build_dir}/bootloader.bin',
        f'{build_dir}/partitions.bin',
        f'{build_dir}/firmware.bin'
    ]

    for filepath in required_files:
        if not os.path.exists(filepath):
            print(f'ERROR: Missing required file: {filepath}')
            print('Run "~/.platformio/penv/bin/platformio run" first to build firmware')
            sys.exit(1)

    # Calculate required size
    firmware_size = os.path.getsize(f'{build_dir}/firmware.bin')
    required_size = 0x10000 + firmware_size

    # Round up to nearest 64KB boundary
    image_size = ((required_size + 0xFFFF) // 0x10000) * 0x10000

    print('=' * 70)
    print('Building TRMNL T5S3 Factory Image')
    print('=' * 70)
    print(f'Firmware size: {firmware_size:,} bytes')
    print(f'Required size: {required_size:,} bytes')
    print(f'Image size:    {image_size:,} bytes ({image_size / 1024 / 1024:.2f} MB)')
    print()

    # Create blank image (0xFF = erased flash)
    factory_image = bytearray([0xFF] * image_size)

    # Merge binaries at correct offsets
    # Note: Bootloader at 0x0 (not 0x1000) for factory image format
    binaries = [
        (0x0,     f'{build_dir}/bootloader.bin', 'Bootloader'),
        (0x8000,  f'{build_dir}/partitions.bin', 'Partition Table'),
        (0x10000, f'{build_dir}/firmware.bin',   'Application Firmware')
    ]

    for offset, filepath, description in binaries:
        with open(filepath, 'rb') as f:
            data = f.read()
            factory_image[offset:offset + len(data)] = data
            print(f'✓ {description:20s} at 0x{offset:05X} ({len(data):,} bytes)')

    # Write factory image
    output_path = 'trmnl-t5s3-v1.1.0-patched.bin'
    with open(output_path, 'wb') as f:
        f.write(factory_image)

    print()
    print('=' * 70)
    print(f'✓ Factory image created: {output_path}')
    print(f'  Size: {len(factory_image):,} bytes ({len(factory_image) / 1024 / 1024:.2f} MB)')
    print()
    print('Flash command:')
    print(f'  esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 \\')
    print(f'      write_flash 0x0 {output_path}')
    print('=' * 70)

if __name__ == '__main__':
    try:
        build_factory_image()
    except Exception as e:
        print(f'ERROR: {e}')
        sys.exit(1)
