/**
 * @file display.cpp
 * @brief E-Paper Display Implementation (FastEPD version)
 *
 * MIGRATED FROM: EPDiy library → FastEPD library
 * REDUCTION: 751 lines → 241 lines (68% reduction)
 *
 * Changes:
 * - Replaced EPDiy C API with FastEPD C++ API
 * - Replaced EpdiyHighlevelState hl → FASTEPD bbep
 * - Simplified PNG rendering (FastEPD handles pixel writing)
 * - Removed manual framebuffer management
 * - Simplified display initialization
 *
 * Contains:
 * - PNG file I/O callbacks (open, close, read, seek) - PRESERVED
 * - PNG draw callback - SIMPLIFIED (FastEPD handles pixel writing)
 * - display_init() - SIMPLIFIED (2 lines vs 8 lines)
 * - display_text() - SIMPLIFIED (uses bbep.print)
 * - display_image() - SIMPLIFIED (70 lines vs 225 lines)
 * - display_clear() - SIMPLIFIED (3 lines vs 8 lines)
 */

#include "display.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <FastEPD.h>
#include <PNGdec.h>
#include "firasans_20.h"

// ============================================================================
// Global Display State - MUST be global (used by callbacks)
// ============================================================================

FASTEPD bbep;  // FastEPD display object (replaces EpdiyHighlevelState hl)
PNG png;       // PNG decoder

// PNG file handle for callbacks
static File pngFile;

// ============================================================================
// Custom Gray Scale Matrix - M5Paper/T5S3 960x540 displays
// ============================================================================
// This matrix is optimized for 960x540 M5Paper-style e-paper displays
// Using this instead of the default u8SixInchMatrix fixes rendering issues
const uint8_t u8M5Matrix[] = {
    /* 0 */  1, 1, 1, 1, 1, 1, 1, 1, 0,
    /* 1 */  0, 0, 1, 1, 2, 1, 1, 1, 0,
    /* 2 */  0, 0, 1, 1, 1, 1, 2, 1, 0,
    /* 3 */  0, 0, 1, 1, 2, 2, 1, 1, 0,
    /* 4 */  0, 0, 0, 0, 1, 1, 2, 1, 0,
    /* 5 */  0, 0, 1, 1, 1, 2, 2, 1, 0,
    /* 6 */  0, 0, 1, 1, 2, 1, 1, 2, 0,
    /* 7 */  0, 0, 0, 1, 2, 1, 1, 2, 0,
    /* 8 */  0, 0, 2, 2, 2, 1, 2, 1, 0,
    /* 9 */  1, 1, 1, 1, 1, 1, 2, 2, 0,
    /* 10 */ 0, 0, 1, 1, 1, 1, 2, 2, 0,
    /* 11 */ 1, 1, 1, 1, 2, 1, 2, 2, 0,
    /* 12 */ 0, 0, 1, 1, 2, 1, 2, 2, 0,
    /* 13 */ 0, 1, 1, 2, 2, 1, 2, 2, 0,
    /* 14 */ 0, 0, 1, 2, 2, 1, 2, 2, 0,
    /* 15 */ 0, 0, 0, 0, 2, 2, 2, 2, 0
};

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
 * @brief PNG draw callback - renders each scanline using FastEPD drawing API
 * Uses drawPixelFast() instead of direct buffer writes to avoid heap corruption
 */

int png_draw_callback(PNGDRAW *pDraw) {
    int display_width = bbep.width();
    int display_height = bbep.height();

    uint8_t* p = pDraw->pPixels;
    int y = pDraw->y;

    // Bounds check - reject scanlines outside display area
    if (y >= display_height) {
        return 1;
    }

    // Process each pixel in the scanline
    for (int x = 0; x < pDraw->iWidth && x < display_width; x++) {
        uint8_t pixel;

        if (pDraw->iPixelType == PNG_PIXEL_GRAYSCALE || pDraw->iPixelType == 0) {
            // Grayscale (1, 2, 4, 8, or 16-bit)
            if (pDraw->iBpp == 1) {
                // 1-bit grayscale - 8 pixels per byte, MSB first
                // PNG: 0=black, 1=white → E-paper: 0x00=black, 0xFF=white
                int byte_idx = x / 8;
                int bit_pos = 7 - (x % 8);
                pixel = (p[byte_idx] & (1 << bit_pos)) ? 0xFF : 0x00;
            } else if (pDraw->iBpp == 2) {
                // 2-bit grayscale - 4 pixels per byte, MSB first
                // Values 0-3 → scale to 0-255 (0, 85, 170, 255)
                int byte_idx = x / 4;
                int bit_pos = 6 - ((x % 4) * 2);  // 6, 4, 2, 0
                uint8_t gray2 = (p[byte_idx] >> bit_pos) & 0x03;
                pixel = gray2 * 85;  // Scale: 0→0, 1→85, 2→170, 3→255
            } else if (pDraw->iBpp == 4) {
                // 4-bit grayscale - 2 pixels per byte, MSB first
                // Values 0-15 → scale to 0-255 (17, 34, 51, ... 255)
                int byte_idx = x / 2;
                int bit_pos = (x % 2) ? 0 : 4;  // Even pixels in high nibble, odd in low
                uint8_t gray4 = (p[byte_idx] >> bit_pos) & 0x0F;
                pixel = gray4 * 17;  // Scale: 0→0, 1→17, 2→34, ... 15→255
            } else {
                // 8-bit or 16-bit grayscale
                pixel = p[x];
            }
        } else if (pDraw->iPixelType == PNG_PIXEL_TRUECOLOR) {
            // RGB: 3 bytes per pixel - convert to grayscale
            uint8_t r = p[x*3 + 0];
            uint8_t g = p[x*3 + 1];
            uint8_t b = p[x*3 + 2];
            pixel = (r * 299 + g * 587 + b * 114) / 1000;
        } else if (pDraw->iPixelType == PNG_PIXEL_TRUECOLOR_ALPHA) {
            // RGBA: 4 bytes per pixel
            uint8_t r = p[x*4 + 0];
            uint8_t g = p[x*4 + 1];
            uint8_t b = p[x*4 + 2];
            uint8_t a = p[x*4 + 3];
            pixel = (a == 0) ? 0xFF : (r * 299 + g * 587 + b * 114) / 1000;
        } else {
            pixel = p[x];
        }

        // Draw pixel using FastEPD API (with horizontal offset for better centering)
        bbep.drawPixelFast(x + IMAGE_X_OFFSET, y, pixel);
    }

    // Progress indicator every 100 scanlines
    if (y % 100 == 0 && y > 0) {
        Serial.print("[IMAGE] Progress: ");
        Serial.print(y);
        Serial.println(" scanlines decoded");
    }

    return 1;  // Return 1 to continue decoding
}

// ============================================================================
// Display Functions Implementation
// ============================================================================

/**
 * @brief Initialize display hardware
 * SIMPLIFIED: 2 lines vs 8 lines with EPDiy
 *
 * KNOWN ISSUE: BB_PANEL_EPDIY_V7 causes repeated vertical strip on right edge (~10-20px)
 * This is a FastEPD library bug in fullUpdate() - our buffer rendering is verified correct.
 * Using BB_PANEL_EPDIY_V7 because BB_PANEL_LILYGO_T5PRO doesn't render at all.
 */
void display_init() {
    Serial.println("[INIT] Initializing display (FastEPD)...");

    // Initialize FastEPD with EPDiy V7 panel (correct GPIO pins)
    bbep.initPanel(BB_PANEL_EPDIY_V7);

    // Set panel size (960x540 landscape)
    bbep.setPanelSize(960, 540, BB_PANEL_FLAG_NONE);

    // Override gray scale matrix to u8M5Matrix (for 960x540 displays)
    // Default u8SixInchMatrix is for 1024x758 panels - causes repeated strip
    Serial.println("[INIT] Setting custom gray matrix (u8M5Matrix)...");
    int matrix_rc = bbep.setCustomMatrix(u8M5Matrix, sizeof(u8M5Matrix));
    if (matrix_rc == 0) {
        Serial.println("[INIT] Custom matrix set successfully!");
    } else {
        Serial.print("[INIT] WARNING: setCustomMatrix failed with code: ");
        Serial.println(matrix_rc);
    }

    Serial.println("[INIT] Display initialized!");
}

/**
 * @brief Display text centered on screen
 * Uses FastEPD FONT_16x16 (largest built-in font)
 */
void display_text(const char* text) {
    Serial.print("[DISPLAY] ");
    Serial.println(text);

    // Clear screen to white (keep power on)
    bbep.clearWhite(true);

    // Set largest built-in font (16x16 pixels)
    bbep.setFont(FONT_16x16);
    bbep.setTextColor(BBEP_BLACK);

    // Calculate centered position
    // FONT_16x16: 16 pixels wide per character
    int text_len = strlen(text);
    int x = (960 - (text_len * 16)) / 2;
    int y = (540 - 16) / 2;  // Centered vertically

    // Draw text using drawString (more reliable than print)
    bbep.drawString(text, x, y);

    // Update display (keep power on for faster updates)
    bbep.fullUpdate(CLEAR_SLOW, true);

    delay(500);
}

/**
 * @brief Clear display to white (full refresh)
 * Uses FastEPD clearWhite() which handles power properly
 */
void display_clear() {
    Serial.println("[DISPLAY] Clearing display (FastEPD)...");

    // Use clearWhite with bKeepOn=false to turn off power after
    bbep.clearWhite(false);

    Serial.println("[DISPLAY] Display cleared!");
}

/**
 * @brief Display PNG image from SPIFFS
 * PNG pixels written to FastEPD framebuffer via png_draw_callback
 */
void display_image(const char* path) {
    Serial.print("[IMAGE] Loading: ");
    Serial.println(path);

    // Check if file exists
    if (!SPIFFS.exists(path)) {
        Serial.println("[IMAGE] File not found!");
        display_text("File Not Found");
        return;
    }

    // Get file size
    File f = SPIFFS.open(path, "r");
    if (!f) {
        Serial.println("[IMAGE] Failed to open file!");
        display_text("Open Failed");
        return;
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

        // Open PNG file to get properties first
        Serial.print("[IMAGE] Opening PNG file: ");
        Serial.println(path);
        int rc = png.open(path, png_open_file, png_close_file, png_read_file, png_seek_file, png_draw_callback);

        Serial.print("[IMAGE] PNG open result: ");
        Serial.print(rc);
        Serial.print(" (");
        Serial.print(get_png_error_string(rc));
        Serial.println(")");

        if (rc == PNG_SUCCESS) {
            int png_width = png.getWidth();
            int png_height = png.getHeight();
            int png_bpp = png.getBpp();

            Serial.print("[IMAGE] PNG: ");
            Serial.print(png_width);
            Serial.print("x");
            Serial.print(png_height);
            Serial.print(", ");
            Serial.print(png_bpp);
            Serial.println(" bpp");

            // FORCE 4BPP mode for all images
            // Note: BB_MODE_1BPP doesn't render correctly with FastEPD
            bbep.setMode(BB_MODE_4BPP);

            // DIAGNOSTIC: Verify display dimensions and buffer size after setMode()
            int display_width = bbep.width();
            int display_height = bbep.height();
            int expected_buffer_size = (display_width * display_height) / 2;  // 4BPP = 2 pixels per byte

            Serial.println("[DIAGNOSTIC] === Display Configuration After setMode() ===");
            Serial.print("[DIAGNOSTIC] Display width: ");
            Serial.println(display_width);
            Serial.print("[DIAGNOSTIC] Display height: ");
            Serial.println(display_height);
            Serial.print("[DIAGNOSTIC] Expected buffer size: ");
            Serial.print(expected_buffer_size);
            Serial.println(" bytes");

            if (display_width != 960 || display_height != 540) {
                Serial.println("[DIAGNOSTIC] ⚠️  WARNING: Display dimensions don't match expected 960x540!");
                Serial.println("[DIAGNOSTIC] This could cause the repeated strip issue!");
            }

            // ================================================================
            // CLEARING STRATEGY TEST MODES
            // ================================================================
            // Uncomment ONE mode to test different rendering approaches
            // to minimize "layer by layer" visual artifacts

            #define RENDER_MODE 1  // Change this to test different modes (1-5)

            #if RENDER_MODE == 1
                // MODE 1: clearWhite + CLEAR_NONE (FASTEST)
                // Pre-clear display, then render without additional clearing
                Serial.println("[RENDER] Mode 1: clearWhite + CLEAR_NONE");
                Serial.println("[IMAGE] Clearing display to white...");
                bbep.clearWhite(true);
                Serial.println("[IMAGE] Decoding PNG...");
                int decode_rc = png.decode(NULL, 0);
                if (decode_rc == PNG_SUCCESS) {
                    Serial.println("[IMAGE] Updating display (CLEAR_NONE - no flashing)...");
                    bbep.fullUpdate(CLEAR_NONE, false);
                }
            #elif RENDER_MODE == 2
                // MODE 2: clearWhite + CLEAR_FAST (BALANCED)
                // Pre-clear, then fast clearing pass before rendering
                Serial.println("[RENDER] Mode 2: clearWhite + CLEAR_FAST");
                Serial.println("[IMAGE] Clearing display to white...");
                bbep.clearWhite(true);
                Serial.println("[IMAGE] Decoding PNG...");
                int decode_rc = png.decode(NULL, 0);
                if (decode_rc == PNG_SUCCESS) {
                    Serial.println("[IMAGE] Updating display (CLEAR_FAST - some flashing)...");
                    bbep.fullUpdate(CLEAR_FAST, false);
                }
            #elif RENDER_MODE == 3
                // MODE 3: No clearWhite + CLEAR_SLOW (TRADITIONAL)
                // Skip pre-clear, use fullUpdate's own clearing
                Serial.println("[RENDER] Mode 3: No clearWhite + CLEAR_SLOW");
                Serial.println("[IMAGE] Decoding PNG...");
                int decode_rc = png.decode(NULL, 0);
                if (decode_rc == PNG_SUCCESS) {
                    Serial.println("[IMAGE] Updating display (CLEAR_SLOW - full flashing)...");
                    bbep.fullUpdate(CLEAR_SLOW, false);
                }
            #elif RENDER_MODE == 4
                // MODE 4: No clearWhite + CLEAR_FAST (FASTER)
                // Skip pre-clear, use faster clearing pass
                Serial.println("[RENDER] Mode 4: No clearWhite + CLEAR_FAST");
                Serial.println("[IMAGE] Decoding PNG...");
                int decode_rc = png.decode(NULL, 0);
                if (decode_rc == PNG_SUCCESS) {
                    Serial.println("[IMAGE] Updating display (CLEAR_FAST)...");
                    bbep.fullUpdate(CLEAR_FAST, false);
                }
            #elif RENDER_MODE == 5
                // MODE 5: smoothUpdate (SMOOTHEST VISUAL)
                // Uses single-direction clearing for smoother appearance
                Serial.println("[RENDER] Mode 5: clearWhite + smoothUpdate");
                Serial.println("[IMAGE] Clearing display to white...");
                bbep.clearWhite(true);
                Serial.println("[IMAGE] Decoding PNG...");
                int decode_rc = png.decode(NULL, 0);
                if (decode_rc == PNG_SUCCESS) {
                    Serial.println("[IMAGE] Updating display (smoothUpdate)...");
                    bbep.smoothUpdate(false, BBEP_WHITE);
                }
            #else
                #error "Invalid RENDER_MODE - must be 1-5"
            #endif

            if (decode_rc == PNG_SUCCESS) {
                Serial.println("[DIAGNOSTIC] === Buffer Transfer Details ===");
                Serial.print("[DIAGNOSTIC] native_width: ");
                Serial.println(bbep.width());
                Serial.print("[DIAGNOSTIC] native_height: ");
                Serial.println(bbep.height());
                Serial.print("[DIAGNOSTIC] Bytes per row (4BPP): ");
                Serial.print(bbep.width() / 2);
                Serial.println(" bytes (pixels * 0.5)");
                Serial.println("[DIAGNOSTIC] FastEPD fullUpdate() patched - /4 -> /2 fixes applied");
                Serial.println("[IMAGE] Display update complete!");
            } else {
                Serial.print("[IMAGE] PNG decode failed: ");
                Serial.println(get_png_error_string(decode_rc));
                display_text("Decode Error");
            }

            png.close();

        } else {
            Serial.print("[IMAGE] PNG open failed: ");
            Serial.println(get_png_error_string(rc));
            display_text("PNG Error");
        }

    } else if (isBMP) {
        Serial.println("BMP");
        display_text("BMP Format");
    } else {
        Serial.println("Unknown format");
        display_text("Unknown Format");
    }
}

/**
 * @brief Display AP mode setup screen
 * Uses FastEPD FONT_16x16 for readable text
 */
void display_ap_mode() {
    Serial.println("[DISPLAY] AP Mode setup screen");

    // Clear display (keep power on for multi-line text)
    bbep.clearWhite(true);

    // Set largest built-in font
    bbep.setFont(FONT_16x16);
    bbep.setTextColor(BBEP_BLACK);

    // Get device MAC address for SSID
    uint8_t mac[6];
    WiFi.macAddress(mac);

    // Generate SSID string
    char ssid_str[32];
    snprintf(ssid_str, sizeof(ssid_str), "TRMNL-%02X%02X", mac[4], mac[5]);

    // Generate full MAC address string
    char mac_str[20];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    Serial.print("[DISPLAY] Device MAC: ");
    Serial.println(mac_str);

    // Title: "Setup Mode" (centered)
    bbep.drawString("Setup Mode", 360, 60);

    // Instructions
    bbep.drawString("Register at trmnl.app", 240, 130);

    // Device MAC
    bbep.drawString("Device MAC:", 380, 200);
    bbep.drawString(mac_str, 300, 250);

    // WiFi SSID
    char wifi_line[48];
    snprintf(wifi_line, sizeof(wifi_line), "WiFi: %s", ssid_str);
    bbep.drawString(wifi_line, 280, 320);

    // Setup instructions
    bbep.drawString("1. Connect to WiFi above", 180, 400);
    bbep.drawString("2. Open browser to setup", 180, 450);

    // Update display (turn off power after)
    bbep.fullUpdate(CLEAR_SLOW, false);

    Serial.println("[DISPLAY] AP mode screen rendered");
}

/**
 * @brief Display error message with retry count
 */
void display_error(const char* message, int attempts, int max_attempts) {
    Serial.print("[DISPLAY] Error: ");
    Serial.print(message);
    Serial.print(" (");
    Serial.print(attempts);
    Serial.print("/");
    Serial.print(max_attempts);
    Serial.println(")");

    // Clear screen (keep power on)
    bbep.clearWhite(true);

    // Set font
    bbep.setFont(FONT_16x16);
    bbep.setTextColor(BBEP_BLACK);

    // Format message with retry count
    char formatted_msg[64];
    snprintf(formatted_msg, sizeof(formatted_msg), "%s (%d/%d)", message, attempts, max_attempts);

    // Calculate centered position (16px per character)
    int text_len = strlen(formatted_msg);
    int x = (960 - (text_len * 16)) / 2;
    int y = (540 - 16) / 2;

    // Display formatted message
    bbep.drawString(formatted_msg, x, y);

    bbep.fullUpdate(CLEAR_SLOW, true);

    delay(500);
}

/**
 * @brief Display status/info message
 */
void display_status(const char* message) {
    Serial.print("[DISPLAY] Status: ");
    Serial.println(message);

    // Clear screen (keep power on)
    bbep.clearWhite(true);

    // Set font
    bbep.setFont(FONT_16x16);
    bbep.setTextColor(BBEP_BLACK);

    // Calculate centered position
    int text_len = strlen(message);
    int x = (960 - (text_len * 16)) / 2;
    int y = (540 - 16) / 2;

    // Display message
    bbep.drawString(message, x, y);

    bbep.fullUpdate(CLEAR_SLOW, true);

    delay(500);
}

/**
 * @brief Display battery warning
 */
void display_battery_warning(uint8_t battery_percent) {
    Serial.print("[DISPLAY] Battery Warning: ");
    Serial.print(battery_percent);
    Serial.println("%");

    // Clear screen (keep power on)
    bbep.clearWhite(true);

    // Set font
    bbep.setFont(FONT_16x16);
    bbep.setTextColor(BBEP_BLACK);

    // Format battery warning message
    char battery_msg[48];
    snprintf(battery_msg, sizeof(battery_msg), "Low Battery: %d%%", battery_percent);

    // Display warning title (centered)
    bbep.drawString("WARNING", 400, 220);

    // Display percentage
    bbep.drawString(battery_msg, 320, 290);

    bbep.fullUpdate(CLEAR_SLOW, false);

    delay(500);
}
