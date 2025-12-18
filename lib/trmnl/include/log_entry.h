/**
 * @file log_entry.h
 * @brief Log entry structure definition (Phase 6)
 *
 * Separated into own file to avoid circular dependencies
 * This is used by logging.h and stored_logs.h
 */

#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <cstdint>

// ============================================================================
// Log Levels
// ============================================================================

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
} log_level_t;

// ============================================================================
// Device Status Context
// ============================================================================

typedef struct {
    float battery_voltage;      // In volts
    uint8_t battery_soc;        // State of Charge (0-100%)
    int8_t battery_temp;        // Battery temperature in °C

    int wifi_rssi;              // WiFi signal strength (dBm, usually -30 to -100)

    uint32_t free_heap;         // Free heap memory in bytes
    uint32_t total_heap;        // Total heap memory in bytes

    char current_image[64];     // Current image filename being displayed
    char last_error[128];       // Last error encountered

    uint16_t uptime_seconds;    // Device uptime in seconds since boot
} device_status_t;

// ============================================================================
// Log Entry Structure
// ============================================================================

typedef struct {
    uint32_t timestamp;         // Unix timestamp
    log_level_t level;          // Log level
    char message[256];          // Log message
    device_status_t status;     // Device status snapshot
} log_entry_t;

#endif // LOG_ENTRY_H
