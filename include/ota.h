/**
 * @file ota.h
 * @brief Over-The-Air (OTA) Firmware Update Interface (Phase 7)
 *
 * Provides secure firmware update capabilities with configurable URL sources.
 * Supports three firmware URL sources with priority:
 *   1. Custom URL from config.h (developer override for testing)
 *   2. Firmware URL from API response (TRMNL server managed)
 *   3. Official TRMNL firmware (fallback/default)
 *
 * Key Features:
 * - HTTPS streaming download (no full buffering required)
 * - ESP32 dual partition OTA (OTA_0 ↔ OTA_1)
 * - Battery level safety check (min 30%)
 * - Timeout protection (2 min default)
 * - Automatic validation and rollback on failure
 *
 * Hardware: ESP32-S3 with 8MB flash (dual OTA partitions)
 */

#ifndef OTA_H
#define OTA_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// OTA Result Codes
// ============================================================================

typedef enum {
    OTA_SUCCESS = 0,              // Update completed successfully
    OTA_ERR_NOT_ENABLED,          // OTA disabled in config.h
    OTA_ERR_LOW_BATTERY,          // Battery below minimum threshold
    OTA_ERR_NO_URL,               // No firmware URL available
    OTA_ERR_NETWORK,              // Network/HTTP error during download
    OTA_ERR_TIMEOUT,              // Download exceeded timeout
    OTA_ERR_VALIDATION,           // Firmware validation failed
    OTA_ERR_FLASH_WRITE,          // Flash write operation failed
    OTA_ERR_NO_PARTITION,         // No OTA partition available
    OTA_ERR_ALREADY_UPDATING      // Update already in progress
} ota_result_t;

// ============================================================================
// OTA Firmware Source Priority
// ============================================================================

typedef enum {
    OTA_SOURCE_CUSTOM = 0,        // Custom URL from config.h (highest priority)
    OTA_SOURCE_API,               // Firmware URL from TRMNL API
    OTA_SOURCE_OFFICIAL,          // Official TRMNL firmware (fallback)
    OTA_SOURCE_NONE               // No source available
} ota_source_t;

// ============================================================================
// OTA Public API
// ============================================================================

/**
 * @brief Initialize OTA subsystem
 *
 * Checks OTA configuration and prepares partition for updates.
 * Must be called before any other OTA functions.
 *
 * @return true if OTA is enabled and ready, false otherwise
 */
bool ota_init();

/**
 * @brief Check if OTA updates are enabled
 *
 * @return true if OTA_ENABLE == 1 in config.h
 */
bool ota_is_enabled();

/**
 * @brief Get current firmware version string
 *
 * @return Version string (e.g., "v1.0.0") or "unknown"
 */
const char* ota_get_version();

/**
 * @brief Determine which firmware URL source to use
 *
 * Priority order:
 *   1. Custom URL from config.h (if non-empty)
 *   2. API firmware URL (if provided)
 *   3. Official TRMNL firmware (fallback)
 *
 * @param api_firmware_url Firmware URL from API response (can be NULL)
 * @return OTA source type that will be used
 */
ota_source_t ota_get_firmware_source(const char* api_firmware_url);

/**
 * @brief Get the firmware URL that will be used for updates
 *
 * Applies priority logic: custom → API → official
 *
 * @param api_firmware_url Firmware URL from API response (can be NULL)
 * @return Firmware URL string (never NULL)
 */
const char* ota_get_firmware_url(const char* api_firmware_url);

/**
 * @brief Perform OTA firmware update from specified URL
 *
 * Downloads firmware via HTTPS, validates, and flashes to inactive partition.
 * On success, device will reboot into new firmware.
 *
 * Safety checks:
 * - Battery must be >= OTA_MIN_BATTERY_PERCENT
 * - Download must complete within OTA_TIMEOUT_MS
 * - Firmware must pass validation
 *
 * @param api_firmware_url Firmware URL from API (can be NULL, will use priority logic)
 * @return OTA result code (OTA_SUCCESS on success)
 */
ota_result_t ota_update_from_url(const char* api_firmware_url);

/**
 * @brief Check if device should update based on API response
 *
 * Evaluates update_firmware flag and battery level.
 *
 * @param update_firmware Flag from API /display response
 * @return true if update should proceed
 */
bool ota_should_update(bool update_firmware);

/**
 * @brief Convert OTA result code to human-readable string
 *
 * @param result OTA result code
 * @return Error description string
 */
const char* ota_result_to_string(ota_result_t result);

/**
 * @brief Get current OTA partition info
 *
 * Logs the active partition and next update partition.
 * Useful for debugging partition switching.
 */
void ota_print_partition_info();

#endif // OTA_H
