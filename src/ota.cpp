/**
 * @file ota.cpp
 * @brief Over-The-Air (OTA) Firmware Update Implementation (Phase 7)
 *
 * Implements secure HTTPS firmware download and ESP32 partition management.
 * Uses ESP32 Update library for dual partition OTA (OTA_0 ↔ OTA_1).
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include "ota.h"
#include "config.h"
#include "power.h"
#include "logging.h"

// ============================================================================
// Private State
// ============================================================================

static bool ota_initialized = false;
static bool ota_in_progress = false;

// Firmware version is now defined in config.h

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Check if battery level is sufficient for OTA
 */
static bool check_battery_level() {
    uint8_t battery_percent = power_get_battery_percentage();

    if (battery_percent < OTA_MIN_BATTERY_PERCENT) {
        Serial.print("[OTA] ERROR: Battery too low for update: ");
        Serial.print(battery_percent);
        Serial.print("% (minimum: ");
        Serial.print(OTA_MIN_BATTERY_PERCENT);
        Serial.println("%)");
        return false;
    }

    Serial.print("[OTA] Battery level OK: ");
    Serial.print(battery_percent);
    Serial.println("%");
    return true;
}

/**
 * @brief Validate firmware URL
 */
static bool is_valid_url(const char* url) {
    if (!url || strlen(url) == 0) {
        return false;
    }

    // Must start with https://
    if (strncmp(url, "https://", 8) != 0) {
        Serial.print("[OTA] ERROR: URL must use HTTPS: ");
        Serial.println(url);
        return false;
    }

    return true;
}

/**
 * @brief Download and flash firmware from URL
 */
static ota_result_t download_and_flash(const char* url) {
    HTTPClient http;

    Serial.print("[OTA] Downloading firmware from: ");
    Serial.println(url);

    // Configure HTTP client
    http.begin(url);
    http.setTimeout(OTA_TIMEOUT_MS);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // Send GET request
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.print("[OTA] ERROR: HTTP GET failed: ");
        Serial.println(httpCode);
        http.end();
        return OTA_ERR_NETWORK;
    }

    // Get content length
    int contentLength = http.getSize();
    if (contentLength <= 0) {
        Serial.println("[OTA] ERROR: Invalid content length");
        http.end();
        return OTA_ERR_VALIDATION;
    }

    Serial.print("[OTA] Firmware size: ");
    Serial.print(contentLength);
    Serial.println(" bytes");

    // Check if we have enough space
    if (!Update.begin(contentLength)) {
        Serial.print("[OTA] ERROR: Not enough space for update. Free: ");
        Serial.println(Update.size());
        http.end();
        return OTA_ERR_NO_PARTITION;
    }

    Serial.println("[OTA] Starting firmware download and flash...");

    // Stream firmware to flash partition
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buffer[OTA_BUFFER_SIZE];
    unsigned long start_time = millis();

    while (http.connected() && (written < contentLength)) {
        // Check timeout
        if (millis() - start_time > OTA_TIMEOUT_MS) {
            Serial.println("[OTA] ERROR: Download timeout");
            Update.abort();
            http.end();
            return OTA_ERR_TIMEOUT;
        }

        // Read chunk from stream
        size_t available = stream->available();
        if (available) {
            int bytesRead = stream->readBytes(buffer, min(available, sizeof(buffer)));

            // Write chunk to flash
            size_t bytesWritten = Update.write(buffer, bytesRead);
            if (bytesWritten != bytesRead) {
                Serial.println("[OTA] ERROR: Flash write failed");
                Update.abort();
                http.end();
                return OTA_ERR_FLASH_WRITE;
            }

            written += bytesWritten;

            // Progress indicator (every 10%)
            static int last_percent = -1;
            int percent = (written * 100) / contentLength;
            if (percent >= last_percent + 10) {
                Serial.print("[OTA] Progress: ");
                Serial.print(percent);
                Serial.println("%");
                last_percent = percent;
            }
        }

        delay(1); // Yield to watchdog
    }

    http.end();

    // Verify we got all bytes
    if (written != contentLength) {
        Serial.print("[OTA] ERROR: Size mismatch. Expected: ");
        Serial.print(contentLength);
        Serial.print(", Got: ");
        Serial.println(written);
        Update.abort();
        return OTA_ERR_VALIDATION;
    }

    Serial.println("[OTA] Download complete, finalizing...");

    // Finalize update (triggers validation)
    if (!Update.end(true)) {
        Serial.print("[OTA] ERROR: Update finalization failed: ");
        Serial.println(Update.getError());
        return OTA_ERR_VALIDATION;
    }

    Serial.println("[OTA] Firmware update successful!");
    Serial.println("[OTA] Device will reboot into new firmware in 3 seconds...");

    return OTA_SUCCESS;
}

// ============================================================================
// Public API Implementation
// ============================================================================

bool ota_init() {
#if OTA_ENABLE == 0
    Serial.println("[OTA] OTA updates disabled in config.h");
    return false;
#endif

    Serial.println("[OTA] Initializing OTA subsystem...");

    // Print partition info
    ota_print_partition_info();

    ota_initialized = true;
    Serial.println("[OTA] OTA subsystem ready");

    return true;
}

bool ota_is_enabled() {
#if OTA_ENABLE == 1
    return true;
#else
    return false;
#endif
}

const char* ota_get_version() {
    return FIRMWARE_VERSION;
}

ota_source_t ota_get_firmware_source(const char* api_firmware_url) {
    // Priority 1: Custom URL from config.h (developer override)
    if (strlen(OTA_CUSTOM_FIRMWARE_URL) > 0) {
        return OTA_SOURCE_CUSTOM;
    }

    // Priority 2: Firmware URL from API
    if (api_firmware_url && strlen(api_firmware_url) > 0) {
        return OTA_SOURCE_API;
    }

    // Priority 3: Official TRMNL firmware (always available)
    return OTA_SOURCE_OFFICIAL;
}

const char* ota_get_firmware_url(const char* api_firmware_url) {
    ota_source_t source = ota_get_firmware_source(api_firmware_url);

    switch (source) {
        case OTA_SOURCE_CUSTOM:
            Serial.println("[OTA] Using CUSTOM firmware URL from config.h");
            return OTA_CUSTOM_FIRMWARE_URL;

        case OTA_SOURCE_API:
            Serial.println("[OTA] Using API firmware URL from TRMNL server");
            return api_firmware_url;

        case OTA_SOURCE_OFFICIAL:
            Serial.println("[OTA] Using OFFICIAL TRMNL firmware (fallback)");
            return OTA_OFFICIAL_FIRMWARE_URL;

        default:
            Serial.println("[OTA] ERROR: No firmware source available");
            return "";
    }
}

ota_result_t ota_update_from_url(const char* api_firmware_url) {
    // Check if OTA is enabled
    if (!ota_is_enabled()) {
        Serial.println("[OTA] ERROR: OTA updates disabled in config.h");
        return OTA_ERR_NOT_ENABLED;
    }

    // Check if already updating
    if (ota_in_progress) {
        Serial.println("[OTA] ERROR: Update already in progress");
        return OTA_ERR_ALREADY_UPDATING;
    }

    ota_in_progress = true;

    Serial.println("========================================");
    Serial.println("[OTA] Starting firmware update...");
    Serial.print("[OTA] Current version: ");
    Serial.println(ota_get_version());

    // Safety check: Battery level
    if (!check_battery_level()) {
        ota_in_progress = false;
        LOG_ERROR("OTA aborted: battery too low");
        return OTA_ERR_LOW_BATTERY;
    }

    // Determine firmware URL using priority logic
    const char* firmware_url = ota_get_firmware_url(api_firmware_url);

    // Validate URL
    if (!is_valid_url(firmware_url)) {
        Serial.println("[OTA] ERROR: No valid firmware URL available");
        ota_in_progress = false;
        LOG_ERROR("OTA aborted: no valid firmware URL");
        return OTA_ERR_NO_URL;
    }

    // Log OTA attempt
    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Starting OTA update from %s",
             (ota_get_firmware_source(api_firmware_url) == OTA_SOURCE_CUSTOM) ? "custom URL" :
             (ota_get_firmware_source(api_firmware_url) == OTA_SOURCE_API) ? "API URL" : "official URL");
    LOG_INFO(log_msg);

    // Perform download and flash
    ota_result_t result = download_and_flash(firmware_url);

    if (result == OTA_SUCCESS) {
        LOG_INFO("OTA update successful, rebooting...");

        // Give time for log to write
        delay(500);

        // Reboot into new firmware
        ESP.restart();

        // Will never reach here
    } else {
        Serial.print("[OTA] Update failed: ");
        Serial.println(ota_result_to_string(result));

        snprintf(log_msg, sizeof(log_msg), "OTA update failed: %s",
                 ota_result_to_string(result));
        LOG_ERROR(log_msg);
    }

    ota_in_progress = false;
    Serial.println("========================================");

    return result;
}

bool ota_should_update(bool update_firmware) {
    // Check if OTA is enabled
    if (!ota_is_enabled()) {
        return false;
    }

    // Check API flag
    if (!update_firmware) {
        return false;
    }

    // Check battery level
    if (!check_battery_level()) {
        Serial.println("[OTA] Skipping update: battery too low");
        return false;
    }

    return true;
}

const char* ota_result_to_string(ota_result_t result) {
    switch (result) {
        case OTA_SUCCESS:              return "Success";
        case OTA_ERR_NOT_ENABLED:      return "OTA disabled";
        case OTA_ERR_LOW_BATTERY:      return "Battery too low";
        case OTA_ERR_NO_URL:           return "No firmware URL";
        case OTA_ERR_NETWORK:          return "Network error";
        case OTA_ERR_TIMEOUT:          return "Download timeout";
        case OTA_ERR_VALIDATION:       return "Validation failed";
        case OTA_ERR_FLASH_WRITE:      return "Flash write error";
        case OTA_ERR_NO_PARTITION:     return "No OTA partition";
        case OTA_ERR_ALREADY_UPDATING: return "Update in progress";
        default:                       return "Unknown error";
    }
}

void ota_print_partition_info() {
    const esp_partition_t* running = esp_ota_get_running_partition();
    const esp_partition_t* next = esp_ota_get_next_update_partition(NULL);

    Serial.println("[OTA] Partition Info:");

    if (running) {
        Serial.print("  Running: ");
        Serial.print(running->label);
        Serial.print(" (");
        Serial.print(running->address, HEX);
        Serial.print(", ");
        Serial.print(running->size / 1024);
        Serial.println("KB)");
    }

    if (next) {
        Serial.print("  Next Update: ");
        Serial.print(next->label);
        Serial.print(" (");
        Serial.print(next->address, HEX);
        Serial.print(", ");
        Serial.print(next->size / 1024);
        Serial.println("KB)");
    }
}
