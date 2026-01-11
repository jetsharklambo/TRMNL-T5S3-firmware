/**
 * @file bmp_decoder.cpp
 * @brief BMP Image Decoder Implementation
 *
 * Decodes 1-bit monochrome BMP files to EPD framebuffer.
 * Handles BMP quirks: bottom-to-top row order, row padding.
 */

#include "bmp_decoder.h"

/**
 * @brief Validate BMP file headers
 */
bool validate_bmp_file(const char* path) {
    File f = SPIFFS.open(path, "r");
    if (!f) {
        Serial.println("[BMP] ERROR: Cannot open file");
        return false;
    }

    // Read file header
    BMPFileHeader file_header;
    if (f.read((uint8_t*)&file_header, sizeof(file_header)) != sizeof(file_header)) {
        Serial.println("[BMP] ERROR: Cannot read file header");
        f.close();
        return false;
    }

    // Validate magic bytes ("BM" = 0x4D42 in little-endian)
    if (file_header.magic != 0x4D42) {
        Serial.printf("[BMP] ERROR: Invalid magic bytes: 0x%04X (expected 0x4D42)\n", file_header.magic);
        f.close();
        return false;
    }

    // Read DIB header
    BMPInfoHeader info_header;
    if (f.read((uint8_t*)&info_header, sizeof(info_header)) != sizeof(info_header)) {
        Serial.println("[BMP] ERROR: Cannot read info header");
        f.close();
        return false;
    }

    // Validate header size (40 = BITMAPINFOHEADER)
    if (info_header.header_size != 40) {
        Serial.printf("[BMP] ERROR: Unsupported header size: %u (expected 40)\n", info_header.header_size);
        f.close();
        return false;
    }

    // Validate bit depth (must be 1-bit)
    if (info_header.bit_depth != 1) {
        Serial.printf("[BMP] ERROR: Unsupported bit depth: %u (expected 1)\n", info_header.bit_depth);
        f.close();
        return false;
    }

    // Validate compression (must be BI_RGB = 0)
    if (info_header.compression != BMP_BI_RGB) {
        Serial.printf("[BMP] ERROR: Compression not supported: %u (expected 0)\n", info_header.compression);
        f.close();
        return false;
    }

    // Validate dimensions - allow smaller images (will be centered)
    int expected_width = epd_rotated_display_width();
    int expected_height = epd_rotated_display_height();

    // Only reject images LARGER than display
    if (info_header.width > expected_width || abs(info_header.height) > expected_height) {
        Serial.printf("[BMP] ERROR: Image too large: %dx%d > %dx%d\n",
                      info_header.width, abs(info_header.height), expected_width, expected_height);
        f.close();
        return false;
    }

    // Log size mismatch but continue with centering
    if (info_header.width != expected_width || abs(info_header.height) != expected_height) {
        Serial.println("[BMP] Image size mismatch - will center on display");
        Serial.printf("[BMP]   Image: %dx%d, Display: %dx%d\n",
                      info_header.width, abs(info_header.height), expected_width, expected_height);
    }

    f.close();

    Serial.println("[BMP] Validation passed");
    Serial.printf("[BMP] Dimensions: %dx%d\n", info_header.width, abs(info_header.height));
    Serial.printf("[BMP] Bit depth: %u\n", info_header.bit_depth);
    Serial.printf("[BMP] Compression: %u (BI_RGB)\n", info_header.compression);

    return true;
}

/**
 * @brief Decode 1-bit BMP file to EPD framebuffer
 */
bool decode_bmp_to_framebuffer(const char* path, uint8_t* framebuffer) {
    // Validate first
    if (!validate_bmp_file(path)) {
        return false;
    }

    // Open file
    File f = SPIFFS.open(path, "r");
    if (!f) {
        Serial.println("[BMP] ERROR: Cannot open file");
        return false;
    }

    Serial.println("[BMP] ====== Starting BMP Decode ======");
    Serial.printf("[BMP] File: %s\n", path);
    Serial.printf("[BMP] Size: %u bytes\n", f.size());

    // Read file header
    BMPFileHeader file_header;
    f.read((uint8_t*)&file_header, sizeof(file_header));

    // Read DIB header
    BMPInfoHeader info_header;
    f.read((uint8_t*)&info_header, sizeof(info_header));

    int width = info_header.width;
    int height = abs(info_header.height);  // Height can be negative for top-down BMPs
    bool bottom_up = (info_header.height > 0);  // Positive = bottom-up (standard)

    Serial.printf("[BMP] Image dimensions: %dx%d\n", width, height);
    Serial.printf("[BMP] Row order: %s\n", bottom_up ? "bottom-up" : "top-down");

    // Get display dimensions
    int display_width = epd_rotated_display_width();
    int display_height = epd_rotated_display_height();

    Serial.printf("[BMP] Display dimensions: %dx%d\n", display_width, display_height);

    // Clear framebuffer before decoding (fill with BLACK for bars)
    Serial.println("[BMP] Clearing framebuffer to black...");
    int framebuffer_size = (display_width * display_height) / 2;  // 4-bit nibble format
    memset(framebuffer, 0x00, framebuffer_size);  // 0x00 = BLACK (for letterbox bars)
    Serial.printf("[BMP] Cleared %d bytes to black\n", framebuffer_size);

    // Calculate centering offsets for smaller images
    int offset_x = (display_width - width) / 2;   // e.g., (960 - 800) / 2 = 80
    int offset_y = (display_height - height) / 2; // e.g., (540 - 480) / 2 = 30

    if (offset_x > 0 || offset_y > 0) {
        Serial.printf("[BMP] Centering offsets: X=%d, Y=%d\n", offset_x, offset_y);
    }

    // Calculate row stride (bytes per row, padded to 4-byte boundary)
    int bytes_per_row = (width + 7) / 8;  // 1 bit per pixel
    int row_stride = ((bytes_per_row + 3) / 4) * 4;  // Pad to 4-byte boundary

    Serial.printf("[BMP] Bytes per row: %d\n", bytes_per_row);
    Serial.printf("[BMP] Row stride (padded): %d\n", row_stride);

    // Skip color palette (2 entries for 1-bit = 8 bytes)
    // Color palette is at offset 14 + 40 = 54
    uint8_t palette[8];
    f.read(palette, 8);

    Serial.println("[BMP] Color palette:");
    Serial.printf("[BMP]   Entry 0 (black): RGB(%u,%u,%u)\n", palette[2], palette[1], palette[0]);
    Serial.printf("[BMP]   Entry 1 (white): RGB(%u,%u,%u)\n", palette[6], palette[5], palette[4]);

    // Seek to pixel data
    f.seek(file_header.data_offset);
    Serial.printf("[BMP] Pixel data offset: %u\n", file_header.data_offset);

    // Allocate row buffer
    uint8_t* row_buffer = (uint8_t*)malloc(row_stride);
    if (!row_buffer) {
        Serial.println("[BMP] ERROR: Cannot allocate row buffer");
        f.close();
        return false;
    }

    Serial.println("[BMP] Decoding pixels...");

    int pixels_decoded = 0;

    // Decode row by row
    // BMP stores rows bottom-to-top (last row first)
    for (int bmp_row = 0; bmp_row < height; bmp_row++) {
        // Read row data
        if (f.read(row_buffer, row_stride) != row_stride) {
            Serial.printf("[BMP] ERROR: Failed to read row %d\n", bmp_row);
            free(row_buffer);
            f.close();
            return false;
        }

        // Calculate display Y coordinate with centering offset
        // BMP row 0 = bottom of image
        int display_y;
        if (bottom_up) {
            display_y = ((height - 1) - bmp_row) + offset_y;  // Flip + offset
        } else {
            display_y = bmp_row + offset_y;  // No flip + offset
        }

        // Skip rows outside display bounds
        if (display_y < 0 || display_y >= display_height) {
            continue;
        }

        // Decode pixels in this row
        for (int x = 0; x < width; x++) {
            // Calculate byte and bit position
            int byte_index = x / 8;
            int bit_position = 7 - (x % 8);  // MSB = leftmost pixel

            // Extract bit
            uint8_t bit = (row_buffer[byte_index] >> bit_position) & 1;

            // BMP palette: 0 = black (0x00), 1 = white (0xFF)
            uint8_t pixel_value = bit ? 0xFF : 0x00;

            // Apply horizontal centering offset
            int display_x = x + offset_x;

            // Bounds check
            if (display_x >= 0 && display_x < display_width) {
                // Write to framebuffer
                epd_draw_pixel(display_x, display_y, pixel_value, framebuffer);
                pixels_decoded++;
            }
        }

        // Progress logging every 50 rows
        if ((bmp_row + 1) % 50 == 0 || bmp_row == 0 || bmp_row == height - 1) {
            Serial.printf("[BMP] Decoded row %d/%d (display Y=%d)\n", bmp_row + 1, height, display_y);
        }
    }

    Serial.println("[BMP] ====== Decode Complete ======");
    Serial.printf("[BMP] Pixels decoded: %d\n", pixels_decoded);

    // Cleanup
    free(row_buffer);
    f.close();

    return true;
}
