/**
 * @file stored_logs.h
 * @brief Local log storage in SPIFFS (Phase 6)
 *
 * Implements circular buffer log storage in SPIFFS for device diagnostics
 * Automatically rotates logs (keeps last 100 entries)
 */

#ifndef STORED_LOGS_H
#define STORED_LOGS_H

#include <cstdint>

// Include log entry definition to use the typedef properly
#include "log_entry.h"

// ============================================================================
// Log Storage Configuration
// ============================================================================

// Maximum number of log entries to store
#define STORED_LOGS_MAX_ENTRIES 100

// SPIFFS file path for logs
#define STORED_LOGS_FILE_PATH "/spiffs/logs.bin"

// ============================================================================
// Internal Functions (called by logging.cpp)
// ============================================================================

/**
 * @brief Initialize log storage in SPIFFS
 *
 * Creates log file if it doesn't exist
 * Must be called before any log operations
 *
 * @return true if successful
 * @return false if SPIFFS error
 */
bool stored_logs_init();

/**
 * @brief Append a log entry to storage
 *
 * Writes log entry to SPIFFS file
 * Automatically rotates if max entries exceeded
 *
 * @param entry Log entry to store
 * @return true if successful
 * @return false if SPIFFS error
 */
bool stored_logs_append(const log_entry_t* entry);

/**
 * @brief Read all stored logs
 *
 * Retrieves all log entries from SPIFFS
 * Caller is responsible for freeing returned pointer
 *
 * @param out_count Pointer to store number of entries read
 * @return Pointer to array of log_entry_t, NULL if error
 */
log_entry_t* stored_logs_read_all(uint32_t* out_count);

/**
 * @brief Clear all stored logs
 *
 * Deletes log file and recreates empty file
 *
 * @return true if successful
 * @return false if error
 */
bool stored_logs_clear();

/**
 * @brief Get number of stored entries
 *
 * @return Number of log entries in storage
 */
uint32_t stored_logs_get_count();

/**
 * @brief Get size of log file in bytes
 *
 * @return File size in bytes
 */
uint32_t stored_logs_get_size();

#endif // STORED_LOGS_H
