/**
 * @file stored_logs.cpp
 * @brief Local log storage in SPIFFS (Phase 6)
 *
 * Implements circular buffer storage of log entries in SPIFFS
 */

#include "stored_logs.h"
#include "log_entry.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <cstring>

// ============================================================================
// Private Log File Format
// ============================================================================

// Simple binary format:
// [uint32_t count] [log_entry_t entry[0]] [log_entry_t entry[1]] ...
//
// This allows us to:
// 1. Quickly determine number of entries (first 4 bytes)
// 2. Read all entries sequentially
// 3. Implement rotation by removing oldest entry when full

// ============================================================================
// Public Interface Implementation
// ============================================================================

bool stored_logs_init() {
    Serial.println("[STORED_LOGS] Initializing log storage...");

    // Check if log file exists
    if (SPIFFS.exists(STORED_LOGS_FILE_PATH)) {
        Serial.println("[STORED_LOGS] Log file already exists");
        return true;
    }

    // Create new empty log file
    File f = SPIFFS.open(STORED_LOGS_FILE_PATH, "w");
    if (!f) {
        Serial.println("[STORED_LOGS] ERROR: Failed to create log file!");
        return false;
    }

    // Write initial entry count (0)
    uint32_t count = 0;
    f.write((uint8_t*)&count, sizeof(count));
    f.close();

    Serial.println("[STORED_LOGS] Log file created successfully");
    return true;
}

bool stored_logs_append(const log_entry_t* entry) {
    if (!entry) {
        return false;
    }

    // Open file for reading existing entries
    File f = SPIFFS.open(STORED_LOGS_FILE_PATH, "r");
    if (!f) {
        Serial.println("[STORED_LOGS] ERROR: Failed to open log file!");
        return false;
    }

    // Read current entry count
    uint32_t count = 0;
    f.read((uint8_t*)&count, sizeof(count));

    // Read all existing entries
    log_entry_t* entries = NULL;
    if (count > 0) {
        entries = (log_entry_t*)malloc(count * sizeof(log_entry_t));
        if (!entries) {
            Serial.println("[STORED_LOGS] ERROR: Memory allocation failed!");
            f.close();
            return false;
        }
        f.read((uint8_t*)entries, count * sizeof(log_entry_t));
    }
    f.close();

    // Check if we need to rotate (remove oldest entry)
    if (count >= STORED_LOGS_MAX_ENTRIES) {
        // Shift all entries down (remove oldest)
        for (uint32_t i = 0; i < count - 1; i++) {
            entries[i] = entries[i + 1];
        }
        count--;
    }

    // Open file for writing
    f = SPIFFS.open(STORED_LOGS_FILE_PATH, "w");
    if (!f) {
        Serial.println("[STORED_LOGS] ERROR: Failed to open log file for writing!");
        if (entries) free(entries);
        return false;
    }

    // Write updated count
    count++;
    f.write((uint8_t*)&count, sizeof(count));

    // Write all existing entries
    if (entries && count > 1) {
        f.write((uint8_t*)entries, (count - 1) * sizeof(log_entry_t));
    }

    // Write new entry
    f.write((uint8_t*)entry, sizeof(log_entry_t));

    f.close();

    if (entries) free(entries);

    Serial.print("[STORED_LOGS] Appended log entry (total: ");
    Serial.print(count);
    Serial.println(")");

    return true;
}

log_entry_t* stored_logs_read_all(uint32_t* out_count) {
    if (!out_count) {
        return NULL;
    }

    *out_count = 0;

    File f = SPIFFS.open(STORED_LOGS_FILE_PATH, "r");
    if (!f) {
        Serial.println("[STORED_LOGS] ERROR: Failed to open log file!");
        return NULL;
    }

    // Read entry count
    uint32_t count = 0;
    f.read((uint8_t*)&count, sizeof(count));

    if (count == 0) {
        f.close();
        Serial.println("[STORED_LOGS] No log entries found");
        return NULL;
    }

    // Allocate buffer for all entries
    log_entry_t* entries = (log_entry_t*)malloc(count * sizeof(log_entry_t));
    if (!entries) {
        Serial.println("[STORED_LOGS] ERROR: Memory allocation failed!");
        f.close();
        return NULL;
    }

    // Read all entries
    size_t bytes_read = f.read((uint8_t*)entries, count * sizeof(log_entry_t));
    f.close();

    if (bytes_read != count * sizeof(log_entry_t)) {
        Serial.println("[STORED_LOGS] ERROR: Failed to read all log entries!");
        free(entries);
        return NULL;
    }

    *out_count = count;
    Serial.print("[STORED_LOGS] Read ");
    Serial.print(count);
    Serial.println(" log entries");

    return entries;
}

bool stored_logs_clear() {
    Serial.println("[STORED_LOGS] Clearing logs...");

    // Delete existing file
    if (SPIFFS.exists(STORED_LOGS_FILE_PATH)) {
        SPIFFS.remove(STORED_LOGS_FILE_PATH);
    }

    // Create new empty file
    return stored_logs_init();
}

uint32_t stored_logs_get_count() {
    File f = SPIFFS.open(STORED_LOGS_FILE_PATH, "r");
    if (!f) {
        return 0;
    }

    uint32_t count = 0;
    f.read((uint8_t*)&count, sizeof(count));
    f.close();

    return count;
}

uint32_t stored_logs_get_size() {
    if (!SPIFFS.exists(STORED_LOGS_FILE_PATH)) {
        return 0;
    }

    File f = SPIFFS.open(STORED_LOGS_FILE_PATH, "r");
    if (!f) {
        return 0;
    }

    uint32_t size = f.size();
    f.close();

    return size;
}
