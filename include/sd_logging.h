/**
 * @file sd_logging.h
 * @brief SD Card Daily Log Export (Phase 6 Extension)
 *
 * Optional SD card export for device logs:
 * - Creates human-readable daily log files
 * - Exports from SPIFFS to SD card
 * - Graceful degradation if SD card unavailable
 * - Device works normally without SD card
 */

#ifndef SD_LOGGING_H
#define SD_LOGGING_H

#include "log_entry.h"
#include <Arduino.h>

// ============================================================================
// SD Card Pin Definitions (LilyGo T5S3 Pro)
// ============================================================================

#define BOARD_SD_MISO   21
#define BOARD_SD_MOSI   13
#define BOARD_SD_SCLK   14
#define BOARD_SD_CS     12

// ============================================================================
// SD Card Logging Configuration
// ============================================================================

// Enable/disable SD card logging (set to 0 to disable completely)
#define SD_LOGGING_ENABLE 1

// Log directory on SD card
#define SD_LOG_DIR "/logs"

// ============================================================================
// Public API
// ============================================================================

/**
 * @brief Initialize SD card for logging
 *
 * Attempts to initialize SD card. If SD card is not present or fails
 * to initialize, device continues normally without SD logging.
 *
 * Performs:
 * - SPI initialization with T5S3 Pro pins
 * - SD card detection and type check
 * - Creates /logs directory if needed
 * - Logs SD card info to serial
 *
 * @return true if SD card initialized successfully
 * @return false if SD card unavailable (device continues normally)
 */
bool sd_logging_init();

/**
 * @brief Check if SD card is available
 *
 * @return true if SD card is initialized and available
 * @return false if SD card not available
 */
bool sd_logging_is_available();

/**
 * @brief Export a single log entry to SD card
 *
 * Formats log entry as human-readable text and appends to daily log file.
 * Filename is generated from current date: YYYY-MM-DD.log
 *
 * If SD card is unavailable, this function returns silently (no errors).
 *
 * Format:
 * [2025-12-17 23:53:34] ERROR: WiFi connection timeout
 *   Battery: 4.20V (99%, 25°C), Heap: 7.4MB/14.8MB, WiFi: -25dBm
 *   Uptime: 10s, Image: plugin-cc0215
 *   Last Error: Connection refused
 *
 * @param entry Log entry to export
 * @return true if successfully written to SD card
 * @return false if SD card unavailable or write failed
 */
bool sd_logging_export_entry(const log_entry_t* entry);

/**
 * @brief Export all SPIFFS logs to SD card (bulk export)
 *
 * Reads all log entries from SPIFFS and exports them to appropriate
 * daily log files on SD card. Each entry is written to the file
 * corresponding to its timestamp.
 *
 * Useful for:
 * - Initial SD card population
 * - Backup after SD card insertion
 * - Periodic bulk sync
 *
 * @return true if export successful
 * @return false if SD card unavailable or no logs to export
 */
bool sd_logging_export_all();

/**
 * @brief Get current daily log filename
 *
 * Generates filename based on current date: /logs/YYYY-MM-DD.log
 *
 * @return String containing full path to daily log file
 *         Empty string if time not set
 */
String sd_logging_get_daily_filename();

/**
 * @brief Format log entry as human-readable text
 *
 * Converts binary log entry to formatted text string suitable
 * for writing to SD card log files.
 *
 * @param entry Log entry to format
 * @return String containing formatted log entry text
 */
String sd_logging_format_entry(const log_entry_t* entry);

/**
 * @brief List all log files on SD card
 *
 * Prints all .log files in /logs directory to Serial
 * Useful for debugging and inspection
 */
void sd_logging_list_files();

/**
 * @brief Get SD card storage info
 *
 * Prints SD card size, used space, and free space to Serial
 */
void sd_logging_print_info();

#endif // SD_LOGGING_H
