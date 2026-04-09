/**
 * @file display.h
 * @brief E-Paper Display Functions for TRMNL T5S3 Pro
 *
 * CRITICAL MODULE: These functions contain delicate hardware initialization
 * and rendering logic that has been extensively tested.
 * Do NOT modify implementations without extensive validation.
 *
 * Functions:
 * - display_init(): Hardware initialization (I2C, display controller, VCOM)
 * - display_text(): Display text message on screen (centered)
 * - display_image(): Load and render PNG image from SPIFFS
 * - display_clear(): Clear display to white
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <FastEPD.h>
#include <PNGdec.h>

/**
 * Global display state - shared across display functions
 * These MUST be global as they're used by PNGdec callbacks
 * and are expensive to reinitialize
 */
extern FASTEPD bbep;
extern PNG png;

/**
 * @brief Initialize display hardware
 *
 * Sets up:
 * - FastEPD display driver (BB_PANEL_LILYGO_T5PRO)
 * - Display panel size (960x540 pixels)
 * - Rotation (180° for landscape orientation)
 *
 * CRITICAL: This must be called before any display operations.
 * If parameters are wrong, display will not initialize.
 *
 * Time: ~100ms
 * Power: Minimal (FastEPD initialization)
 */
void display_init();

/**
 * @brief Display text message centered on screen
 *
 * Clears framebuffer, renders text, updates display.
 * Used for status messages and errors.
 *
 * Examples:
 * - display_text("WiFi Failed");
 * - display_text("Download Failed");
 * - display_text("No Image");
 *
 * @param text Message to display (max ~30 characters)
 *
 * Time: ~2 seconds (includes display update)
 * Power: ~150mA (display active)
 */
void display_text(const char* text);

/**
 * @brief Load and render image from SPIFFS
 *
 * Simplified with FastEPD - handles PNG rendering automatically:
 * - File format detection (PNG vs BMP via magic bytes)
 * - PNG decoding (FastEPD handles pixel writing internally)
 * - Automatic framebuffer management
 * - Full display refresh
 *
 * Currently supports:
 * - PNG: Full 1-bit (black/white) support
 * - BMP: Detected but not rendered (shows "BMP Format")
 * - Other: Shows "Unknown Format"
 *
 * @param path SPIFFS file path (e.g., "/spiffs/display.bmp")
 *
 * Expected images:
 * - Size: 960x540 pixels (landscape)
 * - Format: 1-bit PNG or BMP
 * - File size: 48-100KB
 *
 * Time: ~2-3 seconds (decode + display refresh)
 * Power: 150-200mA during refresh
 */
void display_image(const char* path);

/**
 * @brief Clear display to white (full refresh)
 *
 * Uses FastEPD fillScreen + fullUpdate for proper display clearing.
 *
 * Time: ~1500ms (full display refresh cycle)
 * Power: 150-200mA during refresh
 */
void display_clear();

/**
 * @brief Display AP mode setup screen (Phase 2c)
 *
 * Shows setup instructions when device is in AP/captive portal mode.
 * Displays:
 * - "Setup Mode"
 * - Device's AP SSID (e.g., "TRMNL-XXXX")
 * - Instructions to connect and configure via WiFi
 *
 * This function displays a multi-line message and keeps display on.
 * Device will loop waiting for user input (Phase 3+).
 */
void display_ap_mode();

/**
 * @brief Display error message with retry count
 *
 * Shows error message and retry count on display
 * Used for: WiFi failures, download failures, API errors
 *
 * Example:
 * - display_error("WiFi Failed", 1, 3);  // "WiFi Failed (1/3)"
 * - display_error("Download Failed", 2, 3);
 *
 * @param message Error message to display
 * @param attempts Current attempt number (1-based)
 * @param max_attempts Total number of attempts allowed
 *
 * Time: ~2 seconds (includes display update)
 * Power: ~150mA (display active)
 */
void display_error(const char* message, int attempts, int max_attempts);

/**
 * @brief Display status/info message
 *
 * Shows status message on display (simpler than display_text)
 * Used for: battery status, WiFi status, operation status
 *
 * Example:
 * - display_status("Battery: 95%");
 * - display_status("WiFi: -65 dBm");
 *
 * @param message Status message to display
 *
 * Time: ~2 seconds (includes display update)
 * Power: ~150mA (display active)
 */
void display_status(const char* message);

/**
 * @brief Display battery warning
 *
 * Shows low battery warning on display with battery percentage
 *
 * @param battery_percent Battery percentage (0-100)
 *
 * Time: ~2 seconds (includes display update)
 * Power: ~150mA (display active)
 */
void display_battery_warning(uint8_t battery_percent);

/**
 * @brief Display WiFi error icon in lower right corner
 *
 * Draws a WiFi-off icon (48x48) overlaid on current display content
 * without clearing. Used for WiFi connection failures.
 *
 * Position: Lower right (900, 480)
 * Visual: WiFi waves with diagonal slash
 *
 * Time: ~500ms (CLEAR_NONE update)
 * Power: Minimal (no full clear)
 */
void display_wifi_error_icon();

/**
 * @brief Display decode error icon in lower right corner
 *
 * Draws a camera-video-off icon (48x48) overlaid on current display content
 * without clearing. Used for PNG decode failures.
 *
 * Position: Lower right (900, 480)
 * Visual: Camera symbol with diagonal slash
 *
 * Time: ~500ms (CLEAR_NONE update)
 * Power: Minimal (no full clear)
 */
void display_decode_error_icon();

#endif // DISPLAY_H
