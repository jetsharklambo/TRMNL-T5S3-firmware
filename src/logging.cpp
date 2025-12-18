/**
 * @file logging.cpp
 * @brief Central logging implementation (Phase 6)
 *
 * Provides centralized logging with local SPIFFS storage
 * and server submission capability
 */

#include "logging.h"
#include "device_status.h"
#include "stored_logs.h"
#include "sd_logging.h"
#include <Arduino.h>
#include <time.h>

// ============================================================================
// Private Variables
// ============================================================================

static bool logging_initialized = false;

// ============================================================================
// Public Interface Implementation
// ============================================================================

bool logging_init() {
    Serial.println("[LOGGING] Initializing logging system...");

    if (!stored_logs_init()) {
        Serial.println("[LOGGING] ERROR: Failed to initialize local storage!");
        return false;
    }

    logging_initialized = true;
    Serial.println("[LOGGING] Logging initialized successfully");
    return true;
}

void logging_write(log_level_t level, const char* message) {
    if (!logging_initialized) {
        Serial.println("[LOGGING] WARNING: Not initialized, skipping write");
        return;
    }

    if (!message) {
        return;
    }

    // Get current device status
    device_status_t status = status_get_current();

    // Create log entry
    log_entry_t entry = {};
    entry.timestamp = time(NULL);  // Unix timestamp
    entry.level = level;
    snprintf(entry.message, sizeof(entry.message), "%s", message);
    entry.status = status;

    // Write to local storage
    if (!stored_logs_append(&entry)) {
        Serial.println("[LOGGING] ERROR: Failed to append log entry!");
        return;
    }

    // Export to SD card if available (silently fails if no SD card)
    sd_logging_export_entry(&entry);

    // Log to serial for immediate feedback
    const char* level_str = "";
    switch (level) {
        case LOG_DEBUG:   level_str = "DEBUG"; break;
        case LOG_INFO:    level_str = "INFO"; break;
        case LOG_WARNING: level_str = "WARNING"; break;
        case LOG_ERROR:   level_str = "ERROR"; break;
    }

    Serial.print("[LOG] ");
    Serial.print(level_str);
    Serial.print(": ");
    Serial.println(message);
}

log_entry_t* logging_read_all(uint32_t* out_count) {
    if (!logging_initialized) {
        Serial.println("[LOGGING] WARNING: Not initialized");
        if (out_count) *out_count = 0;
        return NULL;
    }

    return stored_logs_read_all(out_count);
}

bool logging_clear() {
    if (!logging_initialized) {
        Serial.println("[LOGGING] WARNING: Not initialized");
        return false;
    }

    Serial.println("[LOGGING] Clearing all logs...");
    return stored_logs_clear();
}

uint32_t logging_get_count() {
    if (!logging_initialized) {
        return 0;
    }

    return stored_logs_get_count();
}

uint32_t logging_get_size() {
    if (!logging_initialized) {
        return 0;
    }

    return stored_logs_get_size();
}

bool logging_submit_to_server(const char* api_key) {
    // TODO: Implement in api_log.cpp
    Serial.println("[LOGGING] Log submission not yet implemented");
    return false;
}
