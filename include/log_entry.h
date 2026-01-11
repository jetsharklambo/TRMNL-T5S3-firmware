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
// Error Detail Context (for enhanced diagnostics)
// ============================================================================

typedef struct {
    // HTTP/Download errors
    int16_t http_status_code;    // HTTP status code (200, 404, 503, etc.), -1 if N/A
    int8_t retry_attempt;        // Which retry attempt this was (1-3), 0 if N/A
    uint32_t operation_time_ms;  // Time spent on operation in milliseconds
    uint32_t bytes_transferred;  // Bytes downloaded/uploaded
    char failure_reason[64];     // Short description of failure

    // PNG/Image decode errors
    int8_t png_error_code;       // PNG decoder error code, -1 if N/A
    uint16_t image_width;        // Image width if known, 0 if N/A
    uint16_t image_height;       // Image height if known, 0 if N/A
    uint8_t image_bpp;           // Bits per pixel if known, 0 if N/A
} error_detail_t;

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
    error_detail_t error;       // Error details (populated for ERROR level logs)
} log_entry_t;

// ============================================================================
// Helper Functions for Error Details
// ============================================================================

/**
 * @brief Initialize error_detail_t with default "no error" values
 */
inline void error_detail_init(error_detail_t* error) {
    if (!error) return;
    error->http_status_code = -1;
    error->retry_attempt = 0;
    error->operation_time_ms = 0;
    error->bytes_transferred = 0;
    error->failure_reason[0] = '\0';
    error->png_error_code = -1;
    error->image_width = 0;
    error->image_height = 0;
    error->image_bpp = 0;
}

#endif // LOG_ENTRY_H
