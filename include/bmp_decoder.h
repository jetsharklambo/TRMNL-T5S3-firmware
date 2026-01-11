/**
 * @file bmp_decoder.h
 * @brief BMP Image Decoder for 1-bit Monochrome Bitmaps
 *
 * Lightweight BMP decoder for TRMNL e-paper display.
 * Supports 1-bit (monochrome) BMP files without compression.
 *
 * Target format:
 * - 960x540 pixels (landscape)
 * - 1-bit per pixel (black/white)
 * - No compression (BI_RGB)
 * - Standard BMP structure (BITMAPINFOHEADER)
 */

#ifndef BMP_DECODER_H
#define BMP_DECODER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <epdiy.h>

/**
 * @brief BMP file header structure (14 bytes)
 */
struct __attribute__((packed)) BMPFileHeader {
    uint16_t magic;           // 0x00: "BM" = 0x4D42 (little-endian)
    uint32_t file_size;       // 0x02: Total file size in bytes
    uint16_t reserved1;       // 0x06: Reserved (0)
    uint16_t reserved2;       // 0x08: Reserved (0)
    uint32_t data_offset;     // 0x0A: Offset to pixel data
};

/**
 * @brief BMP DIB header (BITMAPINFOHEADER, 40 bytes)
 */
struct __attribute__((packed)) BMPInfoHeader {
    uint32_t header_size;     // 0x0E: Header size (40)
    int32_t width;            // 0x12: Image width in pixels
    int32_t height;           // 0x16: Image height in pixels (positive = bottom-up)
    uint16_t planes;          // 0x1A: Color planes (must be 1)
    uint16_t bit_depth;       // 0x1C: Bits per pixel (1, 4, 8, 24, 32)
    uint32_t compression;     // 0x1E: Compression type (0 = BI_RGB)
    uint32_t image_size;      // 0x22: Image size (can be 0 for BI_RGB)
    int32_t x_pixels_per_m;   // 0x26: Horizontal resolution
    int32_t y_pixels_per_m;   // 0x2A: Vertical resolution
    uint32_t colors_used;     // 0x2E: Colors in palette (0 = all)
    uint32_t colors_important;// 0x32: Important colors (0 = all)
};

/**
 * @brief BMP compression types
 */
#define BMP_BI_RGB        0  // No compression
#define BMP_BI_RLE8       1  // 8-bit RLE
#define BMP_BI_RLE4       2  // 4-bit RLE
#define BMP_BI_BITFIELDS  3  // Bitfield encoding

/**
 * @brief Decode 1-bit BMP file to EPD framebuffer
 *
 * Reads BMP file from SPIFFS and decodes pixel data directly
 * to the e-paper display framebuffer. Supports only 1-bit
 * monochrome BMPs without compression.
 *
 * BMP pixel format:
 * - 8 pixels per byte, MSB = leftmost pixel
 * - Rows stored bottom-to-top (last row first in file)
 * - Rows padded to 4-byte boundary
 * - Color palette: entry 0 = black, entry 1 = white
 *
 * @param path SPIFFS path to BMP file (e.g., "/spiffs/image.bmp")
 * @param framebuffer EPD framebuffer to write pixels to
 * @return true if decode successful, false on error
 *
 * Validation failures:
 * - Invalid magic bytes (not "BM")
 * - Wrong bit depth (not 1-bit)
 * - Compression enabled (not BI_RGB)
 * - Dimensions don't match display (not 960x540)
 * - File read errors
 *
 * Time: ~1-2 seconds for 960x540 image
 * Memory: No additional allocation (streams to framebuffer)
 */
bool decode_bmp_to_framebuffer(const char* path, uint8_t* framebuffer);

/**
 * @brief Validate BMP file headers
 *
 * Checks if BMP file is valid and supported:
 * - Magic bytes = "BM"
 * - Bit depth = 1
 * - Compression = BI_RGB (none)
 * - Dimensions = 960x540
 *
 * @param path SPIFFS path to BMP file
 * @return true if valid and supported, false otherwise
 *
 * Use this for quick validation before attempting decode.
 */
bool validate_bmp_file(const char* path);

#endif // BMP_DECODER_H
