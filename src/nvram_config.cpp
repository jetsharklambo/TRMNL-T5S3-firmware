/**
 * @file nvram_config.cpp
 * @brief Non-Volatile RAM (NVS) configuration implementation
 *
 * Uses ESP32 Preferences API to store and retrieve WiFi credentials and API keys
 * in the device's flash memory. Provides fallback to hardcoded values for safety.
 */

#include "nvram_config.h"
#include "wifi_config.h"
#include <Preferences.h>
#include <Arduino.h>

// ============================================================================
// Constants
// ============================================================================

#define NVRAM_NAMESPACE "data"
#define NVRAM_KEY_API_KEY "api_key"
#define NVRAM_KEY_FRIENDLY_ID "friendly_id"
#define NVRAM_KEY_WIFI_SSID "wifi_ssid"
#define NVRAM_KEY_WIFI_PASSWORD "wifi_password"

// Global Preferences object (persists across function calls)
static Preferences nvram_preferences;
static bool nvram_initialized = false;

// ============================================================================
// Initialization
// ============================================================================

bool nvram_init() {
    if (nvram_initialized) {
        Serial.println("[NVRAM] Already initialized");
        return true;
    }

    // Open Preferences in read-write mode
    // false = read-write, true = read-only
    if (!nvram_preferences.begin(NVRAM_NAMESPACE, false)) {
        Serial.println("[NVRAM] ERROR: Failed to initialize Preferences");
        return false;
    }

    nvram_initialized = true;
    Serial.println("[NVRAM] Initialized successfully");
    nvram_print_status();
    return true;
}

// ============================================================================
// API Key Storage
// ============================================================================

bool nvram_has_api_key() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] WARNING: Not initialized, cannot check API key");
        return false;
    }

    return nvram_preferences.isKey(NVRAM_KEY_API_KEY);
}

bool nvram_read_api_key(char* buffer, size_t buffer_size) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] WARNING: Not initialized, using fallback API key");
        strncpy(buffer, TRMNL_API_KEY, buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
        return true;
    }

    // Try to read from NVS
    if (nvram_preferences.isKey(NVRAM_KEY_API_KEY)) {
        String api_key = nvram_preferences.getString(NVRAM_KEY_API_KEY, "");
        if (api_key.length() > 0) {
            strncpy(buffer, api_key.c_str(), buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            Serial.print("[NVRAM] Loaded API key from NVS: ");
            Serial.println(api_key.c_str());
            return true;
        }
    }

    // Fallback to hardcoded value
    Serial.print("[NVRAM] API key not in NVS, using fallback: ");
    Serial.println(TRMNL_API_KEY);
    strncpy(buffer, TRMNL_API_KEY, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return true;
}

bool nvram_write_api_key(const char* api_key) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: Not initialized, cannot write API key");
        return false;
    }

    if (!api_key || strlen(api_key) == 0) {
        Serial.println("[NVRAM] ERROR: Cannot write empty API key");
        return false;
    }

    if (strlen(api_key) > 255) {
        Serial.println("[NVRAM] ERROR: API key too long (max 255 chars)");
        return false;
    }

    size_t bytes_written = nvram_preferences.putString(NVRAM_KEY_API_KEY, api_key);
    if (bytes_written > 0) {
        Serial.print("[NVRAM] Saved API key to NVS: ");
        Serial.println(api_key);
        return true;
    } else {
        Serial.println("[NVRAM] ERROR: Failed to write API key to NVS");
        return false;
    }
}

// ============================================================================
// Friendly ID Storage (Device Name)
// ============================================================================

bool nvram_has_friendly_id() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] WARNING: Not initialized, cannot check friendly ID");
        return false;
    }

    return nvram_preferences.isKey(NVRAM_KEY_FRIENDLY_ID);
}

bool nvram_read_friendly_id(char* buffer, size_t buffer_size) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] WARNING: Not initialized, cannot read friendly ID");
        buffer[0] = '\0';
        return false;
    }

    // Try to read from NVS
    if (nvram_preferences.isKey(NVRAM_KEY_FRIENDLY_ID)) {
        String friendly_id = nvram_preferences.getString(NVRAM_KEY_FRIENDLY_ID, "");
        if (friendly_id.length() > 0) {
            strncpy(buffer, friendly_id.c_str(), buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            Serial.print("[NVRAM] Loaded friendly ID from NVS: ");
            Serial.println(friendly_id.c_str());
            return true;
        }
    }

    // No friendly ID stored
    Serial.println("[NVRAM] Friendly ID not in NVS");
    buffer[0] = '\0';
    return false;
}

bool nvram_write_friendly_id(const char* friendly_id) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: Not initialized, cannot write friendly ID");
        return false;
    }

    if (!friendly_id || strlen(friendly_id) == 0) {
        Serial.println("[NVRAM] ERROR: Cannot write empty friendly ID");
        return false;
    }

    if (strlen(friendly_id) > 255) {
        Serial.println("[NVRAM] ERROR: Friendly ID too long (max 255 chars)");
        return false;
    }

    size_t bytes_written = nvram_preferences.putString(NVRAM_KEY_FRIENDLY_ID, friendly_id);
    if (bytes_written > 0) {
        Serial.print("[NVRAM] Saved friendly ID to NVS: ");
        Serial.println(friendly_id);
        return true;
    } else {
        Serial.println("[NVRAM] ERROR: Failed to write friendly ID to NVS");
        return false;
    }
}

// ============================================================================
// Debug & Status
// ============================================================================

void nvram_print_status() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] Status: Not initialized");
        return;
    }

    Serial.println("[NVRAM] ===== NVS Status =====");

    // Check API key
    if (nvram_preferences.isKey(NVRAM_KEY_API_KEY)) {
        String api_key = nvram_preferences.getString(NVRAM_KEY_API_KEY, "");
        Serial.print("[NVRAM] API Key (NVS): ");
        // Print first 20 chars + "..." for security
        if (api_key.length() > 20) {
            Serial.print(api_key.substring(0, 20));
            Serial.println("...");
        } else {
            Serial.println(api_key);
        }
    } else {
        Serial.print("[NVRAM] API Key (FALLBACK): ");
        Serial.println(TRMNL_API_KEY);
    }

    // Check friendly ID
    if (nvram_preferences.isKey(NVRAM_KEY_FRIENDLY_ID)) {
        String friendly_id = nvram_preferences.getString(NVRAM_KEY_FRIENDLY_ID, "");
        Serial.print("[NVRAM] Friendly ID: ");
        Serial.println(friendly_id);
    } else {
        Serial.println("[NVRAM] Friendly ID: (not set)");
    }

    Serial.println("[NVRAM] =====================");
}

// ============================================================================
// Credential Management (Phase 2c)
// ============================================================================

bool nvram_clear_all() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: Not initialized, cannot clear");
        return false;
    }

    Serial.println("[NVRAM] Clearing all credentials...");

    // Clear our "data" namespace
    nvram_preferences.clear();
    Serial.println("[NVRAM] Cleared NVS namespace: data");

    // Also clear WiFi credentials stored by Arduino WiFi library
    // This removes saved SSID and password
    WiFi.disconnect(true);  // true = power off radio and clear credentials
    Serial.println("[NVRAM] Cleared WiFi credentials");

    Serial.println("[NVRAM] All credentials cleared. Please reboot to enter setup mode.");
    return true;
}

// ============================================================================
// WiFi Credentials Storage (Phase 3)
// ============================================================================

bool nvram_read_wifi_ssid(char* buffer, size_t buffer_size) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] WARNING: Not initialized, cannot read WiFi SSID");
        buffer[0] = '\0';
        return false;
    }

    // Try to read from NVS
    if (nvram_preferences.isKey(NVRAM_KEY_WIFI_SSID)) {
        String ssid = nvram_preferences.getString(NVRAM_KEY_WIFI_SSID, "");
        if (ssid.length() > 0) {
            strncpy(buffer, ssid.c_str(), buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            Serial.print("[NVRAM] Loaded WiFi SSID from NVS: ");
            Serial.println(ssid.c_str());
            return true;
        }
    }

    // Not stored
    Serial.println("[NVRAM] WiFi SSID not in NVS");
    buffer[0] = '\0';
    return false;
}

bool nvram_write_wifi_ssid(const char* ssid) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: Not initialized, cannot write WiFi SSID");
        return false;
    }

    if (!ssid || strlen(ssid) == 0) {
        Serial.println("[NVRAM] ERROR: Cannot write empty WiFi SSID");
        return false;
    }

    // WiFi spec: max 32 characters for SSID
    if (strlen(ssid) > 32) {
        Serial.println("[NVRAM] ERROR: WiFi SSID too long (max 32 chars)");
        return false;
    }

    size_t bytes_written = nvram_preferences.putString(NVRAM_KEY_WIFI_SSID, ssid);
    if (bytes_written > 0) {
        Serial.print("[NVRAM] Saved WiFi SSID to NVS: ");
        Serial.println(ssid);
        return true;
    } else {
        Serial.println("[NVRAM] ERROR: Failed to write WiFi SSID to NVS");
        return false;
    }
}

bool nvram_read_wifi_password(char* buffer, size_t buffer_size) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] WARNING: Not initialized, cannot read WiFi password");
        buffer[0] = '\0';
        return false;
    }

    // Try to read from NVS
    if (nvram_preferences.isKey(NVRAM_KEY_WIFI_PASSWORD)) {
        String password = nvram_preferences.getString(NVRAM_KEY_WIFI_PASSWORD, "");
        if (password.length() > 0) {
            strncpy(buffer, password.c_str(), buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            Serial.print("[NVRAM] Loaded WiFi password from NVS (");
            Serial.print(password.length());
            Serial.println(" chars)");
            return true;
        }
    }

    // Not stored
    Serial.println("[NVRAM] WiFi password not in NVS");
    buffer[0] = '\0';
    return false;
}

bool nvram_write_wifi_password(const char* password) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: Not initialized, cannot write WiFi password");
        return false;
    }

    if (!password || strlen(password) == 0) {
        Serial.println("[NVRAM] ERROR: Cannot write empty WiFi password");
        return false;
    }

    // WiFi spec: max 64 characters for password
    if (strlen(password) > 64) {
        Serial.println("[NVRAM] ERROR: WiFi password too long (max 64 chars)");
        return false;
    }

    size_t bytes_written = nvram_preferences.putString(NVRAM_KEY_WIFI_PASSWORD, password);
    if (bytes_written > 0) {
        Serial.print("[NVRAM] Saved WiFi password to NVS (");
        Serial.print(strlen(password));
        Serial.println(" chars)");
        return true;
    } else {
        Serial.println("[NVRAM] ERROR: Failed to write WiFi password to NVS");
        return false;
    }
}

// ============================================================================
// WiFi Failure Recovery (Phase 2 - Error Handling)
// ============================================================================

/**
 * @brief Get WiFi connection failure count
 *
 * Retrieves the number of consecutive WiFi connection failures from NVS.
 * Returns 0 if counter not yet set.
 *
 * @return Current failure count (0-5), or 0 if not set
 */
uint8_t nvram_get_wifi_failure_count() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: NVS not initialized");
        return 0;
    }

    uint8_t count = nvram_preferences.getUChar("wifi_fail_count", 0);
    Serial.print("[NVRAM] WiFi failure count: ");
    Serial.println(count);
    return count;
}

/**
 * @brief Increment WiFi connection failure counter
 *
 * Increments the failure counter and stores in NVS.
 * Capped at maximum of 5 to prevent overflow.
 *
 * @return New failure count after increment
 */
uint8_t nvram_increment_wifi_failure_count() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: NVS not initialized");
        return 0;
    }

    uint8_t current_count = nvram_preferences.getUChar("wifi_fail_count", 0);
    uint8_t new_count = (current_count < 5) ? current_count + 1 : 5;  // Cap at 5

    nvram_preferences.putUChar("wifi_fail_count", new_count);

    Serial.print("[NVRAM] WiFi failure count incremented to: ");
    Serial.println(new_count);

    return new_count;
}

/**
 * @brief Reset WiFi connection failure counter to zero
 *
 * Clears the failure counter from NVS.
 * Called after successful WiFi connection.
 *
 * @return true if reset successfully, false if error
 */
bool nvram_reset_wifi_failure_count() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: NVS not initialized");
        return false;
    }

    nvram_preferences.putUChar("wifi_fail_count", 0);
    Serial.println("[NVRAM] WiFi failure count reset to 0");
    return true;
}

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
uint16_t nvram_get_refresh_rate() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: NVS not initialized, using default refresh rate");
        return 300;  // Default: 5 minutes
    }

    uint16_t refresh_rate = nvram_preferences.getUShort("refresh_rate", 300);

    Serial.print("[NVRAM] Refresh rate: ");
    Serial.print(refresh_rate);
    Serial.println(" seconds");

    return refresh_rate;
}

/**
 * @brief Save refresh rate (sleep interval) to NVS
 *
 * Stores the server-provided refresh rate for next boot.
 * Called after successful /api/display response.
 *
 * @param refresh_rate_seconds Refresh rate in seconds
 * @return true if saved successfully, false if error
 */
bool nvram_write_refresh_rate(uint16_t refresh_rate_seconds) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: NVS not initialized");
        return false;
    }

    // Clamp to reasonable values (minimum 10s, maximum 65535s ~18 hours for uint16_t)
    if (refresh_rate_seconds < 10) {
        Serial.println("[NVRAM] WARNING: Refresh rate too low (<10s), clamping to 10s");
        refresh_rate_seconds = 10;
    }
    if (refresh_rate_seconds > 65535) {
        Serial.println("[NVRAM] WARNING: Refresh rate too high (>65535s), clamping to 65535s");
        refresh_rate_seconds = 65535;
    }

    nvram_preferences.putUShort("refresh_rate", refresh_rate_seconds);

    Serial.print("[NVRAM] Saved refresh rate: ");
    Serial.print(refresh_rate_seconds);
    Serial.println(" seconds");

    return true;
}

// ============================================================================
// NTP Sync Timestamp Management (Battery Optimization)
// ============================================================================

/**
 * @brief Get last NTP sync timestamp from NVS
 *
 * Used to implement periodic NTP sync optimization.
 * Only sync every 6 hours; ESP32 RTC maintains accurate time between syncs.
 *
 * @return Unix timestamp of last NTP sync, or 0 if never synced
 */
time_t nvram_get_last_ntp_sync() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: NVRAM not initialized, cannot read NTP sync time");
        return 0;
    }

    // Read as UInt64 (time_t is 64-bit on ESP32)
    uint64_t last_sync = nvram_preferences.getULong64("ntp_sync", 0);

    Serial.print("[NVRAM] Last NTP sync: ");
    if (last_sync > 0) {
        Serial.print(last_sync);
        Serial.print(" (");

        // Calculate hours ago, checking for valid RTC time to prevent overflow
        time_t now = time(NULL);
        if (now > 100000 && now >= last_sync) {
            time_t hours_ago = (now - last_sync) / 3600;
            Serial.print(hours_ago);
            Serial.println(" hours ago)");
        } else {
            Serial.println("RTC not synced yet)");
        }
    } else {
        Serial.println("never");
    }

    return (time_t)last_sync;
}

/**
 * @brief Save last NTP sync timestamp to NVS
 *
 * Called after successful NTP time synchronization.
 *
 * @param timestamp Unix timestamp of NTP sync
 * @return true if saved successfully, false if error
 */
bool nvram_write_last_ntp_sync(time_t timestamp) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: NVRAM not initialized, cannot write NTP sync time");
        return false;
    }

    nvram_preferences.putULong64("ntp_sync", (uint64_t)timestamp);

    Serial.print("[NVRAM] Saved NTP sync timestamp: ");
    Serial.println(timestamp);

    return true;
}

// ============================================================================
// WiFi Fast Connect Optimization (Phase 2 Battery Optimization)
// ============================================================================

/**
 * @brief Get cached WiFi BSSID from NVS
 *
 * Retrieves the last known good BSSID to enable fast WiFi connect.
 * By connecting directly to the cached BSSID, we skip the full AP scan.
 *
 * @param bssid Output buffer for BSSID (must be 6 bytes)
 * @return true if valid BSSID found, false if not cached
 */
bool nvram_get_wifi_bssid(uint8_t* bssid) {
    if (!nvram_initialized || bssid == NULL) {
        Serial.println("[NVRAM] ERROR: Cannot read WiFi BSSID (not initialized or NULL buffer)");
        return false;
    }

    // Read 6-byte BSSID from NVS
    size_t len = nvram_preferences.getBytes("wifi_bssid", bssid, 6);

    if (len == 6) {
        Serial.print("[NVRAM] Cached WiFi BSSID: ");
        for (int i = 0; i < 6; i++) {
            if (bssid[i] < 16) Serial.print("0");
            Serial.print(bssid[i], HEX);
            if (i < 5) Serial.print(":");
        }
        Serial.println();
        return true;
    } else {
        Serial.println("[NVRAM] No cached WiFi BSSID found");
        return false;
    }
}

/**
 * @brief Save WiFi BSSID to NVS for fast connect
 *
 * Caches the BSSID after successful connection for fast reconnect.
 *
 * @param bssid BSSID to cache (6 bytes)
 * @return true if saved successfully, false if error
 */
bool nvram_write_wifi_bssid(const uint8_t* bssid) {
    if (!nvram_initialized || bssid == NULL) {
        Serial.println("[NVRAM] ERROR: Cannot write WiFi BSSID (not initialized or NULL)");
        return false;
    }

    // Write 6-byte BSSID to NVS
    nvram_preferences.putBytes("wifi_bssid", bssid, 6);

    Serial.print("[NVRAM] Saved WiFi BSSID: ");
    for (int i = 0; i < 6; i++) {
        if (bssid[i] < 16) Serial.print("0");
        Serial.print(bssid[i], HEX);
        if (i < 5) Serial.print(":");
    }
    Serial.println();

    return true;
}

/**
 * @brief Get cached WiFi channel from NVS
 *
 * Retrieves the last known WiFi channel for fast connect.
 *
 * @return Channel number (1-14), or 0 if not cached
 */
int8_t nvram_get_wifi_channel() {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: Cannot read WiFi channel (not initialized)");
        return 0;
    }

    int8_t channel = nvram_preferences.getChar("wifi_chan", 0);

    if (channel > 0 && channel <= 14) {
        Serial.print("[NVRAM] Cached WiFi channel: ");
        Serial.println(channel);
        return channel;
    } else {
        Serial.println("[NVRAM] No cached WiFi channel found");
        return 0;
    }
}

/**
 * @brief Save WiFi channel to NVS for fast connect
 *
 * Caches the WiFi channel after successful connection.
 *
 * @param channel WiFi channel number (1-14)
 * @return true if saved successfully, false if error
 */
bool nvram_write_wifi_channel(int8_t channel) {
    if (!nvram_initialized) {
        Serial.println("[NVRAM] ERROR: Cannot write WiFi channel (not initialized)");
        return false;
    }

    if (channel < 1 || channel > 14) {
        Serial.print("[NVRAM] WARNING: Invalid WiFi channel: ");
        Serial.println(channel);
        return false;
    }

    nvram_preferences.putChar("wifi_chan", channel);

    Serial.print("[NVRAM] Saved WiFi channel: ");
    Serial.println(channel);

    return true;
}
