/**
 * @file nvram_config.h
 * @brief Non-Volatile RAM (NVS) configuration storage
 *
 * Provides persistent storage for WiFi credentials, API keys, and device IDs
 * using ESP32's Preferences API (NVS flash memory).
 *
 * Aligns with official TRMNL firmware architecture:
 * - Namespace: "data"
 * - Keys: API key, friendly ID, WiFi SSID/password
 * - Fallback: Hardcoded values if NVS empty or corrupted
 */

#ifndef NVRAM_CONFIG_H
#define NVRAM_CONFIG_H

#include <cstddef>
#include <cstdint>

// ============================================================================
// NVRAM Initialization
// ============================================================================

/**
 * @brief Initialize NVS (Non-Volatile Storage) system
 *
 * Must be called once during boot before any NVS read/write operations.
 * Safe to call multiple times.
 *
 * @return true if initialized successfully, false if error
 */
bool nvram_init();

// ============================================================================
// API Key Storage
// ============================================================================

/**
 * @brief Check if API key exists in NVS
 *
 * @return true if API key is stored in NVS, false otherwise
 */
bool nvram_has_api_key();

/**
 * @brief Read API key from NVS
 *
 * Returns API key from NVS storage if available, otherwise returns
 * fallback value (hardcoded TRMNL_API_KEY from wifi_config.h)
 *
 * @param buffer - Pointer to char array to store API key
 * @param buffer_size - Maximum bytes to write (should be >= 256)
 * @return true if key loaded successfully, false if error
 *
 * @note Always returns a valid key (NVS or fallback)
 */
bool nvram_read_api_key(char* buffer, size_t buffer_size);

/**
 * @brief Write API key to NVS
 *
 * Stores API key in NVS under "data" namespace, key name "api_key".
 * Called after successful /api/setup registration.
 *
 * @param api_key - API key string to store (max 255 chars)
 * @return true if stored successfully, false if error
 */
bool nvram_write_api_key(const char* api_key);

// ============================================================================
// Friendly ID Storage (Device Name)
// ============================================================================

/**
 * @brief Check if friendly ID exists in NVS
 *
 * @return true if friendly ID is stored in NVS, false otherwise
 */
bool nvram_has_friendly_id();

/**
 * @brief Read friendly ID from NVS
 *
 * Returns friendly ID (device name) from NVS if available, otherwise
 * returns empty string.
 *
 * @param buffer - Pointer to char array to store friendly ID
 * @param buffer_size - Maximum bytes to write (should be >= 256)
 * @return true if friendly ID loaded, false if not stored or error
 */
bool nvram_read_friendly_id(char* buffer, size_t buffer_size);

/**
 * @brief Write friendly ID to NVS
 *
 * Stores friendly ID (device name) in NVS under "data" namespace.
 * Called after successful /api/setup registration.
 *
 * @param friendly_id - Device friendly ID string to store (max 255 chars)
 * @return true if stored successfully, false if error
 */
bool nvram_write_friendly_id(const char* friendly_id);

// ============================================================================
// Debug & Status
// ============================================================================

/**
 * @brief Print NVRAM status to serial for debugging
 *
 * Shows what credentials are stored and from where they were loaded
 * (NVS or fallback). Useful for debugging credential issues.
 */
void nvram_print_status();

// ============================================================================
// WiFi Credentials Storage (Phase 3)
// ============================================================================

/**
 * @brief Read WiFi SSID from NVS
 *
 * @param buffer - Pointer to char array to store SSID
 * @param buffer_size - Maximum bytes to write (should be >= 33 for SSID + null)
 * @return true if SSID exists and loaded, false if not stored
 */
bool nvram_read_wifi_ssid(char* buffer, size_t buffer_size);

/**
 * @brief Write WiFi SSID to NVS
 *
 * Stores WiFi SSID for connection after AP mode setup.
 *
 * @param ssid - WiFi SSID string to store (max 32 chars per WiFi spec)
 * @return true if stored successfully, false if error
 */
bool nvram_write_wifi_ssid(const char* ssid);

/**
 * @brief Read WiFi password from NVS
 *
 * @param buffer - Pointer to char array to store password
 * @param buffer_size - Maximum bytes to write (should be >= 65 for password + null)
 * @return true if password exists and loaded, false if not stored
 */
bool nvram_read_wifi_password(char* buffer, size_t buffer_size);

/**
 * @brief Write WiFi password to NVS
 *
 * Stores WiFi password for connection after AP mode setup.
 *
 * @param password - WiFi password string to store (max 64 chars per WiFi spec)
 * @return true if stored successfully, false if error
 */
bool nvram_write_wifi_password(const char* password);

// ============================================================================
// Credential Management (Phase 2c)
// ============================================================================

/**
 * @brief Clear all credentials from NVS
 *
 * Erases the entire "data" namespace, removing all stored credentials.
 * Also clears WiFi credentials from Arduino's WiFi Preferences.
 * Called when user requests factory reset or setup mode.
 *
 * After calling this, device must be rebooted to enter setup mode.
 *
 * @return true if cleared successfully, false if error
 */
bool nvram_clear_all();

// ============================================================================
// WiFi Failure Recovery (Phase 2 - Error Handling)
// ============================================================================

/**
 * @brief Get WiFi connection failure count
 *
 * Retrieves the number of consecutive WiFi connection failures.
 * Used to implement automatic recovery after repeated failures.
 *
 * @return Current failure count (0-5), or 0 if not set
 */
uint8_t nvram_get_wifi_failure_count();

/**
 * @brief Increment WiFi connection failure counter
 *
 * Increments the failure counter and stores in NVS.
 * Capped at maximum of 5 to prevent overflow.
 * Called after each failed WiFi connection attempt.
 *
 * @return New failure count after increment
 */
uint8_t nvram_increment_wifi_failure_count();

/**
 * @brief Reset WiFi connection failure counter to zero
 *
 * Clears the failure counter from NVS.
 * Called after successful WiFi connection.
 *
 * @return true if reset successfully, false if error
 */
bool nvram_reset_wifi_failure_count();

// ============================================================================
// Refresh Rate Storage (Phase 4)
// ============================================================================

/**
 * @brief Get refresh rate (sleep interval) from NVS
 *
 * Retrieves the server-provided refresh rate if available,
 * otherwise returns default value (300 seconds).
 *
 * @return Refresh rate in seconds, or 300 if not set
 */
uint16_t nvram_get_refresh_rate();

/**
 * @brief Save refresh rate (sleep interval) to NVS
 *
 * Stores the server-provided refresh rate for next boot.
 * Called after successful /api/display response.
 *
 * @param refresh_rate_seconds Refresh rate in seconds
 * @return true if saved successfully, false if error
 */
bool nvram_write_refresh_rate(uint16_t refresh_rate_seconds);

#endif // NVRAM_CONFIG_H
