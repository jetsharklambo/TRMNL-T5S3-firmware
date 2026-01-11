/**
 * @file logging.h
 * @brief Central logging API for TRMNL device (Phase 6)
 *
 * Provides centralized logging for all subsystems with:
 * - Local storage in SPIFFS
 * - Multiple log levels (DEBUG, INFO, WARNING, ERROR)
 * - Device context (battery, heap, WiFi signal)
 * - Server submission via /api/log
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "log_entry.h"

// ============================================================================
// Public Logging API
// ============================================================================

/**
 * @brief Initialize logging system
 *
 * Creates log file in SPIFFS if it doesn't exist
 * Must be called once at startup
 *
 * @return true if initialization successful
 * @return false if SPIFFS error
 */
bool logging_init();

/**
 * @brief Write a log entry
 *
 * Appends log entry to local storage with device context
 * Handles log rotation automatically (keeps last 100 entries)
 *
 * @param level Log level (DEBUG, INFO, WARNING, ERROR)
 * @param message Log message (max 255 characters)
 *
 * Example:
 * - logging_write(LOG_ERROR, "WiFi connection timeout");
 * - logging_write(LOG_INFO, "Image downloaded successfully");
 * - logging_write(LOG_WARNING, "Low battery detected");
 */
void logging_write(log_level_t level, const char* message);

/**
 * @brief Write a log entry with detailed error information
 *
 * Appends log entry with additional error diagnostic information
 * for HTTP errors, PNG decode errors, etc.
 *
 * @param level Log level (typically LOG_ERROR)
 * @param message Log message
 * @param error Pointer to error_detail_t struct with diagnostic info
 *
 * Example:
 * ```
 * error_detail_t err;
 * error_detail_init(&err);
 * err.http_status_code = 503;
 * err.retry_attempt = 2;
 * err.operation_time_ms = 15234;
 * strcpy(err.failure_reason, "Service unavailable");
 * logging_write_error(LOG_ERROR, "API /display call failed", &err);
 * ```
 */
void logging_write_error(log_level_t level, const char* message, const error_detail_t* error);

// Use typedef in logging.h to ensure consistent typing
typedef log_entry_t log_entry_type;

/**
 * @brief Read all stored logs
 *
 * Retrieves all logs from SPIFFS storage
 * Caller is responsible for freeing returned pointer
 *
 * @param out_count Pointer to store number of log entries read
 * @return Pointer to array of log_entry_t, NULL if error
 *
 * Usage:
 * ```
 * uint32_t count = 0;
 * log_entry_t* logs = logging_read_all(&count);
 * if (logs) {
 *     for (uint32_t i = 0; i < count; i++) {
 *         // Process logs[i]
 *     }
 *     free(logs);
 * }
 * ```
 */
log_entry_t* logging_read_all(uint32_t* out_count);

/**
 * @brief Clear all stored logs
 *
 * Deletes log file from SPIFFS and reinitializes
 * Use with caution - data will be lost
 *
 * @return true if successful
 * @return false if error
 */
bool logging_clear();

/**
 * @brief Get number of stored log entries
 *
 * @return Number of entries currently in storage
 */
uint32_t logging_get_count();

/**
 * @brief Get size of log storage in bytes
 *
 * @return Total bytes used by log file
 */
uint32_t logging_get_size();

/**
 * @brief Submit logs to server
 *
 * Sends all stored logs to /api/log endpoint
 * Requires WiFi connection
 *
 * This is typically called periodically or on-demand
 * Does NOT clear local logs after successful submission
 *
 * @param api_key TRMNL API key for authentication
 * @return true if submission successful
 * @return false if network error or server error
 */
bool logging_submit_to_server(const char* api_key);

/**
 * @brief Convenience macro for logging errors
 *
 * Usage: LOG_ERROR("WiFi failed: %s", error_message);
 * Note: This is a simple version, full sprintf not supported
 */
#define LOG_ERROR(msg) logging_write(LOG_ERROR, msg)
#define LOG_WARNING(msg) logging_write(LOG_WARNING, msg)
#define LOG_INFO(msg) logging_write(LOG_INFO, msg)
#define LOG_DEBUG(msg) logging_write(LOG_DEBUG, msg)

#endif // LOGGING_H
