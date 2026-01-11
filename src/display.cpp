/**
 * @file display.cpp
 * @brief E-Paper Display Implementation
 *
 * EXTRACTED FROM: TRMNL-T5S3-Pro/src/main.cpp (lines 39-597)
 * NO LOGIC CHANGES - COPY EXACT from working code
 *
 * Contains:
 * - PNG file I/O callbacks (open, close, read, seek)
 * - PNG draw callback for pixel rendering
 * - display_init() - Hardware setup
 * - display_text() - Text rendering
 * - display_image() - PNG/BMP rendering
 * - display_clear() - Clear to white
 */

#include "display.h"
#include "bmp_decoder.h"
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <epdiy.h>
#include <PNGdec.h>
#include "firasans_20.h"

// Board and waveform configuration
#define WAVEFORM EPD_BUILTIN_WAVEFORM
#define DEMO_BOARD epd_board_v7

// ============================================================================
// Global Display State - MUST be global (used by callbacks)
// ============================================================================

EpdiyHighlevelState hl;
PNG png;
int png_draw_count = 0;

// PNG file handle for callbacks
static File pngFile;

// Forward declaration
static const char* get_png_error_string(int error_code);

// PNG error tracking for enhanced logging
static png_error_info_t last_png_error = {0, 0, 0, 0, "No error"};

void display_set_png_error(int code, uint16_t w, uint16_t h, uint8_t bpp) {
    last_png_error.error_code = code;
    last_png_error.width = w;
    last_png_error.height = h;
    last_png_error.bpp = bpp;
    last_png_error.error_description = get_png_error_string(code);
}

png_error_info_t display_get_last_png_error() {
    return last_png_error;
}

// ============================================================================
// PNG Error Strings - For debugging decode failures
// ============================================================================

const char* png_error_strings[] = {
    "PNG_SUCCESS",            // 0
    "PNG_INVALID_PARAMETER",  // 1
    "PNG_DECODE_ERROR",       // 2
    "PNG_MEM_ERROR",          // 3
    "PNG_NO_BUFFER",          // 4
    "PNG_UNSUPPORTED_FEATURE",// 5
    "PNG_INVALID_FILE",       // 6
    "PNG_TOO_BIG",            // 7
    "PNG_DECODE_INIT_ERROR"   // 8
};

const char* get_png_error_string(int error_code) {
    if (error_code >= 0 && error_code < 9) {
        return png_error_strings[error_code];
    }
    return "PNG_UNKNOWN_ERROR";
}

// ============================================================================
// PNG File I/O Callbacks - Required by PNGdec library
// ============================================================================

/**
 * @brief PNG callback: Open file and get size
 * Called by PNGdec to prepare file for reading
 */
void* png_open_file(const char *szFilename, int32_t *pFileSize) {
    pngFile = SPIFFS.open(szFilename, "r");
    if (pngFile) {
        *pFileSize = (int32_t)pngFile.size();
        return (void*)1;  // Return non-null to indicate success
    }
    return NULL;
}

/**
 * @brief PNG callback: Close file
 * Called by PNGdec when done reading
 */
void png_close_file(void *pHandle) {
    if (pngFile) {
        pngFile.close();
    }
}

/**
 * @brief PNG callback: Read data from file
 * Called by PNGdec to read file chunks
 */
int32_t png_read_file(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen) {
    return (int32_t)pngFile.read(pBuf, iLen);
}

/**
 * @brief PNG callback: Seek to position in file
 * Called by PNGdec for random access
 */
int32_t png_seek_file(PNGFILE *pFile, int32_t iPos) {
    return (int32_t)pngFile.seek(iPos);
}

/**
 * @brief PNG draw callback - renders each scanline to framebuffer
 * CRITICAL: This is called for every scanline. Must be fast and accurate.
 */
int png_draw_callback(PNGDRAW *pDraw) {
    png_draw_count++;

    uint8_t* fb = epd_hl_get_framebuffer(&hl);
    int display_width = epd_rotated_display_width();
    int display_height = epd_rotated_display_height();

    uint8_t* p = pDraw->pPixels;
    int y = pDraw->y;

    // Log first few scanlines and every 100th line
    if (png_draw_count <= 3 || png_draw_count % 100 == 0) {
        Serial.print("[PNG_DRAW] Line ");
        Serial.print(png_draw_count);
        Serial.print(": y=");
        Serial.print(y);
        Serial.print(" width=");
        Serial.print(pDraw->iWidth);
        Serial.print(" type=");
        Serial.print(pDraw->iPixelType);
        Serial.print(" bpp=");
        Serial.println(pDraw->iBpp);
    }

    // Only render if within display bounds
    if (y >= display_height) {
        Serial.print("[PNG_DRAW] Skipping line ");
        Serial.print(y);
        Serial.print(" (>= height ");
        Serial.print(display_height);
        Serial.println(")");
        return 0;
    }

    // Process each pixel in the scanline
    int pixels_written = 0;
    for (int x = 0; x < pDraw->iWidth && x < display_width; x++) {
        uint8_t pixel;

        if (pDraw->iPixelType == PNG_PIXEL_TRUECOLOR) {
            // RGB: 3 bytes per pixel
            uint8_t r = p[x*3 + 0];
            uint8_t g = p[x*3 + 1];
            uint8_t b = p[x*3 + 2];
            // Convert to grayscale
            pixel = (r * 299 + g * 587 + b * 114) / 1000;
        } else if (pDraw->iPixelType == PNG_PIXEL_TRUECOLOR_ALPHA) {
            // RGBA: 4 bytes per pixel
            uint8_t r = p[x*4 + 0];
            uint8_t g = p[x*4 + 1];
            uint8_t b = p[x*4 + 2];
            uint8_t a = p[x*4 + 3];
            // Convert to grayscale with alpha
            if (a == 0) {
                pixel = 0xFF;  // Transparent = white
            } else {
                pixel = (r * 299 + g * 587 + b * 114) / 1000;
            }
        } else if (pDraw->iPixelType == PNG_PIXEL_INDEXED) {
            // Indexed color - use palette
            pixel = p[x];
        } else if (pDraw->iPixelType == PNG_PIXEL_GRAYSCALE || pDraw->iPixelType == 0) {
            // Grayscale (1, 2, 4, 8, or 16-bit)
            if (pDraw->iBpp == 1) {
                // 1-bit grayscale - 8 pixels per byte
                // MSB = first pixel, LSB = eighth pixel
                int byte_idx = x / 8;
                int bit_pos = 7 - (x % 8);  // MSB first
                pixel = (p[byte_idx] & (1 << bit_pos)) ? 0x00 : 0xFF;
            } else if (pDraw->iBpp == 2) {
                // 2-bit grayscale - 4 pixels per byte
                int byte_idx = x / 4;
                int bit_pos = 6 - ((x % 4) * 2);
                uint8_t val = (p[byte_idx] >> bit_pos) & 0x03;
                pixel = (val << 6) | (val << 4) | (val << 2) | val;  // Expand to 8-bit
            } else if (pDraw->iBpp == 4) {
                // 4-bit grayscale - 2 pixels per byte
                int byte_idx = x / 2;
                int bit_pos = (x % 2) ? 0 : 4;
                uint8_t val = (p[byte_idx] >> bit_pos) & 0x0F;
                pixel = (val << 4) | val;  // Expand to 8-bit
            } else {
                // 8-bit or 16-bit grayscale
                pixel = p[x];
            }
        } else {
            // Unknown type - use as-is
            pixel = p[x];
        }

        // Write to framebuffer
        if (x < display_width && y < display_height) {
            fb[y * display_width + x] = pixel;
            pixels_written++;
        }
    }

    // Log progress every 50 lines
    if (png_draw_count % 50 == 0) {
        Serial.print("[PNG_DRAW] Progress: ");
        Serial.print(png_draw_count);
        Serial.print(" lines, ");
        Serial.print(pixels_written);
        Serial.println(" pixels written");
    }

    return 0;  // Return 0 to continue decoding
}

// ============================================================================
// Display Functions Implementation
// ============================================================================

/**
 * @brief Initialize display hardware
 * CRITICAL - Lines 239-252 from original main.cpp
 */
void display_init() {
    Serial.println("[INIT] Initializing display...");

    Wire.begin(39, 40);
    epd_init(&DEMO_BOARD, &ED047TC1, EPD_LUT_64K);
    epd_set_vcom(1560);
    hl = epd_hl_init(WAVEFORM);
    epd_set_rotation(EPD_ROT_INVERTED_LANDSCAPE);  // 180° rotation - device held upside-down

    Serial.println("[INIT] Display initialized!");
}

/**
 * @brief Display text centered on screen
 * CRITICAL - Lines 209-234 from original main.cpp
 */
void display_text(const char* text) {
    Serial.print("[DISPLAY] ");
    Serial.println(text);

    uint8_t* fb = epd_hl_get_framebuffer(&hl);

    // Clear framebuffer completely
    int width = epd_rotated_display_width();
    int height = epd_rotated_display_height();
    memset(fb, 0xFF, width * height);

    int temperature = epd_ambient_temperature();
    int cursor_x = width / 2;
    int cursor_y = height / 2;

    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_CENTER;

    epd_write_string(&FiraSans_20, text, &cursor_x, &cursor_y, fb, &font_props);

    epd_poweron();
    epd_hl_update_screen(&hl, MODE_GL16, temperature);
    epd_poweroff();

    delay(500);
}

/**
 * @brief Clear display to white (full refresh)
 * CRITICAL - Lines 259-268 from original main.cpp
 */
void display_clear() {
    Serial.println("[DISPLAY] Clearing display (hardware clear)...");

    // Use hardware clear operation - optimized and correct waveforms
    epd_poweron();
    epd_clear();
    epd_poweroff();

    Serial.println("[DISPLAY] Display cleared!");
}

/**
 * @brief Display PNG image from SPIFFS
 * CRITICAL - Lines 353-597 from original main.cpp
 * Most complex function - handles file detection, PNG decoding, rendering
 * @return true if successfully displayed, false on error
 */
bool display_image(const char* path) {
    Serial.print("[IMAGE] Loading: ");
    Serial.println(path);

    // Check if file exists
    if (!SPIFFS.exists(path)) {
        Serial.println("[IMAGE] File not found!");
        display_text("File Not Found");
        return false;
    }

    // Get file size
    File f = SPIFFS.open(path, "r");
    if (!f) {
        Serial.println("[IMAGE] Failed to open file!");
        display_text("Open Failed");
        return false;
    }

    size_t fileSize = f.size();
    Serial.print("[IMAGE] File size: ");
    Serial.println(fileSize);
    f.close();

    // Check file format by magic bytes
    f = SPIFFS.open(path, "r");
    uint8_t magic[4] = {0};
    f.read(magic, 4);
    f.close();

    // PNG magic: 89 50 4E 47
    bool isPNG = (magic[0] == 0x89 && magic[1] == 0x50 && magic[2] == 0x4E && magic[3] == 0x47);
    // BMP magic: 42 4D
    bool isBMP = (magic[0] == 0x42 && magic[1] == 0x4D);

    Serial.print("[IMAGE] Format: ");
    if (isPNG) {
        Serial.println("PNG");

        // Open PNG file
        Serial.print("[IMAGE] Opening PNG file: ");
        Serial.println(path);
        int rc = png.open(path, png_open_file, png_close_file, png_read_file, png_seek_file, NULL);

        Serial.print("[IMAGE] PNG open result: ");
        Serial.print(rc);
        Serial.print(" (");
        Serial.print(get_png_error_string(rc));
        Serial.println(")");

        if (rc != PNG_SUCCESS) {
            // Failed to open PNG - record error
            display_set_png_error(rc, 0, 0, 0);
        }

        if (rc == PNG_SUCCESS) {
            int png_width = png.getWidth();
            int png_height = png.getHeight();
            int png_bpp = png.getBpp();
            int png_type = png.getPixelType();

            Serial.print("[IMAGE] PNG properties: ");
            Serial.print(png_width);
            Serial.print("x");
            Serial.print(png_height);
            Serial.print(" ");
            Serial.print(png_bpp);
            Serial.print("bpp type=");
            Serial.println(png_type);

            // Get required buffer size for decoded image
            int buffer_size = png.getBufferSize();
            Serial.print("[IMAGE] PNG buffer size needed: ");
            Serial.println(buffer_size);

            // Allocate buffer - try PSRAM first, then regular RAM
            uint8_t* png_decode_buffer = NULL;
            bool using_psram = false;

            #ifdef BOARD_HAS_PSRAM
            if (psramFound()) {
                png_decode_buffer = (uint8_t*)ps_malloc(buffer_size);
                if (png_decode_buffer) {
                    using_psram = true;
                    Serial.print("[IMAGE] ✓ Allocated PNG buffer in PSRAM (");
                    Serial.print(buffer_size);
                    Serial.println(" bytes)");
                } else {
                    Serial.println("[IMAGE] WARNING: PSRAM allocation failed, trying regular RAM");
                }
            }
            #endif

            if (!png_decode_buffer) {
                png_decode_buffer = (uint8_t*)malloc(buffer_size);
                if (png_decode_buffer) {
                    Serial.print("[IMAGE] ✓ Allocated PNG buffer in RAM (");
                    Serial.print(buffer_size);
                    Serial.println(" bytes)");
                } else {
                    Serial.println("[IMAGE] ERROR: Failed to allocate PNG buffer!");
                    Serial.print("[IMAGE] Requested: ");
                    Serial.print(buffer_size);
                    Serial.print(" bytes, Free heap: ");
                    Serial.println(ESP.getFreeHeap());
                    png.close();
                    display_text("Memory Error");
                    return false;
                }
            }

            // Verify buffer is valid
            if (!png_decode_buffer) {
                Serial.println("[IMAGE] ERROR: Buffer pointer is NULL!");
                png.close();
                display_text("Buffer Error");
                return false;
            }

            // Set the buffer for the decoder
            png.setBuffer(png_decode_buffer);

            // Decode the PNG image with retry logic
            Serial.println("[IMAGE] Starting PNG decode with full buffer...");
            Serial.print("[IMAGE] Buffer: ");
            Serial.print(using_psram ? "PSRAM" : "RAM");
            Serial.print(", Size: ");
            Serial.print(buffer_size);
            Serial.println(" bytes");

            int decode_rc = png.decode(NULL, 0);

            Serial.print("[IMAGE] PNG decode result: ");
            Serial.print(decode_rc);
            Serial.print(" (");
            Serial.print(get_png_error_string(decode_rc));
            Serial.println(")");

            // Retry once if decode failed
            if (decode_rc != PNG_SUCCESS) {
                Serial.println("[IMAGE] Decode failed, retrying once...");
                png.close();

                // Reopen the PNG file
                rc = png.open(path, png_open_file, png_close_file, png_read_file, png_seek_file, NULL);
                if (rc == PNG_SUCCESS) {
                    png.setBuffer(png_decode_buffer);
                    decode_rc = png.decode(NULL, 0);
                    Serial.print("[IMAGE] Retry decode result: ");
                    Serial.print(decode_rc);
                    Serial.print(" (");
                    Serial.print(get_png_error_string(decode_rc));
                    Serial.println(")");
                }
            }

            if (decode_rc == PNG_SUCCESS) {
                // Clear display before drawing new image
                Serial.println("[IMAGE] Clearing display...");
                epd_poweron();
                epd_clear();
                epd_poweroff();
                delay(100);

                // Get display dimensions
                uint8_t* fb = epd_hl_get_framebuffer(&hl);
                int display_width = epd_rotated_display_width();
                int display_height = epd_rotated_display_height();

                Serial.print("[IMAGE] Display dimensions: ");
                Serial.print(display_width);
                Serial.print("x");
                Serial.println(display_height);

                // Clear framebuffer
                int framebuffer_bytes = (display_width * display_height) / 2;
                Serial.print("[IMAGE] Clearing framebuffer: ");
                Serial.print(framebuffer_bytes);
                Serial.println(" bytes");
                memset(fb, 0xFF, framebuffer_bytes);

                // Copy decoded PNG data to framebuffer
                Serial.println("[IMAGE] Copying PNG data to framebuffer...");

                Serial.print("[IMAGE] PNG bit depth: ");
                Serial.print(png_bpp);
                Serial.println("bpp");
                Serial.print("[IMAGE] PNG dimensions: ");
                Serial.print(png_width);
                Serial.print("x");
                Serial.println(png_height);

                // Draw image using direct pixel-by-pixel approach
                Serial.println("[IMAGE] Drawing image using direct epd_draw_pixel()...");
                Serial.println("[IMAGE] Extracting 1-bit pixels from buffer...");

                int pixels_drawn = 0;

                // PNG 1bpp format: 8 pixels packed in each byte, MSB first
                // Row stride must account for padding to byte boundary
                int row_stride_bytes = (png_width + 7) / 8;  // Bytes per row (rounded up)

                for (int y = 0; y < png_height; y++) {
                    for (int x = 0; x < png_width; x++) {
                        // Calculate byte position and bit position within that byte
                        int byte_offset = y * row_stride_bytes + (x / 8);
                        int bit_position = 7 - (x % 8);  // MSB is leftmost

                        // Extract the single bit and convert to 8-bit value
                        uint8_t bit = (png_decode_buffer[byte_offset] >> bit_position) & 1;
                        uint8_t pixel_value = bit ? 0xFF : 0x00;  // 1 = white (0xFF), 0 = black (0x00)

                        // epd_draw_pixel handles 4-bit nibble packing internally
                        epd_draw_pixel(x, y, pixel_value, fb);
                        pixels_drawn++;
                    }

                    // Progress logging every row
                    if ((y + 1) % 50 == 0 || y == 0 || y == png_height - 1) {
                        Serial.print("[IMAGE] Row ");
                        Serial.print(y + 1);
                        Serial.print("/");
                        Serial.println(png_height);
                    }
                }

                Serial.print("[IMAGE] Rendered ");
                Serial.print(pixels_drawn);
                Serial.println(" pixels to framebuffer (1-bit extracted)");

                // Update display
                Serial.println("[IMAGE] Rendering to display...");
                int temperature = epd_ambient_temperature();
                Serial.print("[IMAGE] Temperature: ");
                Serial.print(temperature);
                Serial.println(" C");

                Serial.println("[IMAGE] Powering on...");
                epd_poweron();
                delay(100);

                Serial.println("[IMAGE] Updating screen (MODE_GL16)...");
                epd_hl_update_screen(&hl, MODE_GL16, temperature);
                delay(100);

                Serial.println("[IMAGE] Powering off...");
                epd_poweroff();

                Serial.println("[IMAGE] Display update complete!");

                // Free buffer and return success
                free(png_decode_buffer);
                Serial.println("[IMAGE] PNG buffer freed");
                png.close();

                return true;  // SUCCESS!
            } else {
                Serial.println("[IMAGE] ====== PNG DECODE FAILURE ======");
                Serial.print("[IMAGE] Error code: ");
                Serial.print(decode_rc);
                Serial.print(" (");
                Serial.print(get_png_error_string(decode_rc));
                Serial.println(")");
                Serial.print("[IMAGE] File size: ");
                Serial.println(fileSize);
                Serial.print("[IMAGE] Buffer size: ");
                Serial.println(buffer_size);
                Serial.print("[IMAGE] PNG dimensions: ");
                Serial.print(png_width);
                Serial.print("x");
                Serial.print(png_height);
                Serial.print(" ");
                Serial.print(png_bpp);
                Serial.println("bpp");
                Serial.print("[IMAGE] Buffer location: ");
                Serial.println(using_psram ? "PSRAM" : "RAM");
                Serial.print("[IMAGE] Free heap: ");
                Serial.println(ESP.getFreeHeap());
                Serial.println("[IMAGE] =============================");

                // Record PNG decode error for logging
                display_set_png_error(decode_rc, png_width, png_height, png_bpp);

                // Display error with code
                char error_msg[32];
                snprintf(error_msg, sizeof(error_msg), "Decode Err %d", decode_rc);
                display_text(error_msg);

                // Free buffer and return failure
                free(png_decode_buffer);
                Serial.println("[IMAGE] PNG buffer freed");
                png.close();
                return false;
            }

            // Should not reach here - success path returns early
            png.close();
            free(png_decode_buffer);
            Serial.println("[IMAGE] PNG buffer freed");
            return false;

        } else {
            Serial.print("[IMAGE] PNG open failed with code: ");
            Serial.print(rc);
            Serial.print(" (");
            Serial.print(get_png_error_string(rc));
            Serial.println(")");
            display_text("PNG Error");
            return false;
        }

    } else if (isBMP) {
        Serial.println("BMP");

        // Use custom BMP decoder
        Serial.println("[IMAGE] Decoding BMP image...");
        uint8_t* fb = epd_hl_get_framebuffer(&hl);

        if (decode_bmp_to_framebuffer(path, fb)) {
            // Clear display first to remove ghosting from previous image
            Serial.println("[IMAGE] Clearing display before BMP render...");
            epd_poweron();
            epd_clear();
            epd_poweroff();
            delay(100);

            // Render to display
            Serial.println("[IMAGE] Rendering BMP to display...");
            int temperature = epd_ambient_temperature();
            Serial.print("[IMAGE] Temperature: ");
            Serial.print(temperature);
            Serial.println(" C");

            Serial.println("[IMAGE] Powering on...");
            epd_poweron();
            delay(100);

            Serial.println("[IMAGE] Updating screen (MODE_GL16)...");
            epd_hl_update_screen(&hl, MODE_GL16, temperature);
            delay(100);

            Serial.println("[IMAGE] Powering off...");
            epd_poweroff();

            Serial.println("[IMAGE] BMP display complete!");
            return true;
        } else {
            Serial.println("[IMAGE] BMP decode failed");
            display_text("BMP Error");
            return false;
        }
    } else {
        Serial.println("Unknown format");
        display_text("Unknown Format");
        return false;
    }
}

/**
 * @brief Display AP mode setup screen (Phase 2c)
 *
 * Shows setup instructions when device is in AP/captive portal mode.
 * Displays:
 * - "Setup Mode" headline
 * - Device's AP SSID (e.g., "TRMNL-XXXX")
 * - Instructions to connect and configure via WiFi
 *
 * This function displays a multi-line message and keeps display on.
 * Device will loop waiting for user input (Phase 3+).
 */
void display_ap_mode() {
    Serial.println("[DISPLAY] AP Mode setup screen");

    // Clear the display first (hardware clear with proper waveforms)
    display_clear();

    // Get framebuffer from epdiy
    uint8_t* fb = epd_hl_get_framebuffer(&hl);
    int width = epd_rotated_display_width();
    int height = epd_rotated_display_height();

    // Clear framebuffer to white
    memset(fb, 0xFF, width * height);

    // Get device MAC address for SSID
    uint8_t mac[6];
    WiFi.macAddress(mac);

    // Generate SSID string (WiFi name with last 2 bytes of MAC)
    char ssid_str[48];
    snprintf(ssid_str, sizeof(ssid_str), "WiFi: TRMNL-%02X%02X", mac[4], mac[5]);
    Serial.print("[DISPLAY] Generated SSID: ");
    Serial.println(ssid_str);

    // Generate full MAC address string for device registration
    char mac_str[20];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.print("[DISPLAY] Device MAC: ");
    Serial.println(mac_str);

    // Get ambient temperature for display refresh
    int temperature = epd_ambient_temperature();

    // Title: "Setup Mode"
    {
        int cursor_x = width / 2;
        int cursor_y = 60;
        EpdFontProperties font_props = epd_font_properties_default();
        font_props.flags = EPD_DRAW_ALIGN_CENTER;
        epd_write_string(&FiraSans_20, "Setup Mode", &cursor_x, &cursor_y, fb, &font_props);
    }

    // Instructions: Visit registration website
    {
        int cursor_x = width / 2;
        int cursor_y = 130;
        EpdFontProperties font_props = epd_font_properties_default();
        font_props.flags = EPD_DRAW_ALIGN_CENTER;
        epd_write_string(&FiraSans_20, "Register at trmnl.app", &cursor_x, &cursor_y, fb, &font_props);
    }

    // Device MAC address (prominently displayed for easy reading)
    {
        int cursor_x = width / 2;
        int cursor_y = 200;
        EpdFontProperties font_props = epd_font_properties_default();
        font_props.flags = EPD_DRAW_ALIGN_CENTER;
        font_props.bg_color = 0xFF;  // White background for readability
        epd_write_string(&FiraSans_20, "Device MAC:", &cursor_x, &cursor_y, fb, &font_props);
    }

    // MAC address value (larger font for easy reading)
    {
        int cursor_x = width / 2;
        int cursor_y = 250;
        EpdFontProperties font_props = epd_font_properties_default();
        font_props.flags = EPD_DRAW_ALIGN_CENTER;
        epd_write_string(&FiraSans_20, mac_str, &cursor_x, &cursor_y, fb, &font_props);
    }

    // WiFi SSID to connect to (actual AP name)
    {
        char ap_ssid_label[32];
        snprintf(ap_ssid_label, sizeof(ap_ssid_label), "WiFi: TRMNL-%02X%02X", mac[4], mac[5]);

        int cursor_x = width / 2;
        int cursor_y = 320;
        EpdFontProperties font_props = epd_font_properties_default();
        font_props.flags = EPD_DRAW_ALIGN_CENTER;
        epd_write_string(&FiraSans_20, ap_ssid_label, &cursor_x, &cursor_y, fb, &font_props);
    }

    // Instruction lines
    {
        EpdFontProperties font_props = epd_font_properties_default();

        int cursor_x = 40;
        int cursor_y = 400;
        epd_write_string(&FiraSans_20, "1. Connect to WiFi above", &cursor_x, &cursor_y, fb, &font_props);

        cursor_x = 40;
        cursor_y = 450;
        epd_write_string(&FiraSans_20, "2. Open browser to setup", &cursor_x, &cursor_y, fb, &font_props);
    }

    // Update display (full refresh with temperature compensation)
    epd_poweron();
    epd_hl_update_screen(&hl, MODE_GC16, temperature);
    epd_poweroff();

    Serial.println("[DISPLAY] AP mode screen rendered");
}

/**
 * @brief Display error message with retry count (Phase 6)
 *
 * Shows error message like "WiFi Failed (1/3)" on the display
 */
void display_error(const char* message, int attempts, int max_attempts) {
    Serial.print("[DISPLAY] Error: ");
    Serial.print(message);
    Serial.print(" (");
    Serial.print(attempts);
    Serial.print("/");
    Serial.print(max_attempts);
    Serial.println(")");

    uint8_t* fb = epd_hl_get_framebuffer(&hl);
    int width = epd_rotated_display_width();
    int height = epd_rotated_display_height();

    // Clear framebuffer
    memset(fb, 0xFF, width * height);

    int temperature = epd_ambient_temperature();

    // Format message with retry count
    char formatted_msg[64];
    snprintf(formatted_msg, sizeof(formatted_msg), "%s (%d/%d)", message, attempts, max_attempts);

    // Display formatted message centered
    int cursor_x = width / 2;
    int cursor_y = height / 2;
    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_CENTER;

    epd_write_string(&FiraSans_20, formatted_msg, &cursor_x, &cursor_y, fb, &font_props);

    epd_poweron();
    epd_hl_update_screen(&hl, MODE_GL16, temperature);
    epd_poweroff();

    delay(500);
}

/**
 * @brief Display status/info message (Phase 6)
 *
 * Shows status message on display
 */
void display_status(const char* message) {
    Serial.print("[DISPLAY] Status: ");
    Serial.println(message);

    uint8_t* fb = epd_hl_get_framebuffer(&hl);
    int width = epd_rotated_display_width();
    int height = epd_rotated_display_height();

    // Clear framebuffer
    memset(fb, 0xFF, width * height);

    int temperature = epd_ambient_temperature();

    // Display message centered
    int cursor_x = width / 2;
    int cursor_y = height / 2;
    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_CENTER;

    epd_write_string(&FiraSans_20, message, &cursor_x, &cursor_y, fb, &font_props);

    epd_poweron();
    epd_hl_update_screen(&hl, MODE_GL16, temperature);
    epd_poweroff();

    delay(500);
}

/**
 * @brief Display battery warning (Phase 6)
 *
 * Shows low battery warning with percentage
 */
void display_battery_warning(uint8_t battery_percent) {
    Serial.print("[DISPLAY] Battery Warning: ");
    Serial.print(battery_percent);
    Serial.println("%");

    uint8_t* fb = epd_hl_get_framebuffer(&hl);
    int width = epd_rotated_display_width();
    int height = epd_rotated_display_height();

    // Clear framebuffer
    memset(fb, 0xFF, width * height);

    int temperature = epd_ambient_temperature();

    // Format battery warning message
    char battery_msg[48];
    snprintf(battery_msg, sizeof(battery_msg), "Low Battery: %d%%", battery_percent);

    // Display warning
    int cursor_x = width / 2;
    int cursor_y = height / 2 - 30;
    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_CENTER;

    epd_write_string(&FiraSans_20, "WARNING", &cursor_x, &cursor_y, fb, &font_props);

    // Display percentage
    cursor_x = width / 2;
    cursor_y = height / 2 + 30;
    epd_write_string(&FiraSans_20, battery_msg, &cursor_x, &cursor_y, fb, &font_props);

    epd_poweron();
    epd_hl_update_screen(&hl, MODE_GL16, temperature);
    epd_poweroff();

    delay(500);
}
