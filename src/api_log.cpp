/**
 * @file api_log.cpp
 * @brief TRMNL /api/log endpoint implementation (Phase 6)
 *
 * Submits device logs to server for diagnostics and monitoring
 */

#include "api-client/submit_log.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Convert log level to string
 */
static const char* log_level_to_string(log_level_t level) {
    switch (level) {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR:   return "ERROR";
        default:         return "UNKNOWN";
    }
}

/**
 * @brief Serialize single log entry to JSON
 */
static bool serialize_log_entry(const log_entry_t* entry, JsonObject& json_obj) {
    if (!entry) {
        return false;
    }

    json_obj["timestamp"] = entry->timestamp;
    json_obj["level"] = log_level_to_string(entry->level);
    json_obj["message"] = entry->message;

    // Add device status
    JsonObject status = json_obj.createNestedObject("device_status");
    status["battery_voltage"] = entry->status.battery_voltage;
    status["battery_soc"] = entry->status.battery_soc;
    status["battery_temp"] = entry->status.battery_temp;
    status["wifi_rssi"] = entry->status.wifi_rssi;
    status["free_heap"] = entry->status.free_heap;
    status["total_heap"] = entry->status.total_heap;
    status["uptime_seconds"] = entry->status.uptime_seconds;
    status["current_image"] = entry->status.current_image;
    status["last_error"] = entry->status.last_error;

    return true;
}

// ============================================================================
// Public Interface Implementation
// ============================================================================

bool trmnlApiSubmitLog(const char* api_key, const char* device_id, const log_entry_t* entry) {
    if (!api_key || !device_id || !entry) {
        Serial.println("[API_LOG] ERROR: Invalid parameters");
        return false;
    }

    Serial.print("[API_LOG] Submitting log: ");
    Serial.println(entry->message);

    // Create JSON document
    DynamicJsonDocument doc(1024);
    doc["device_id"] = device_id;
    doc["timestamp"] = entry->timestamp;
    doc["log_message"] = entry->message;

    // Add device status
    JsonObject status = doc.createNestedObject("device_status");
    status["battery_voltage"] = entry->status.battery_voltage;
    status["battery_soc"] = entry->status.battery_soc;
    status["battery_temp"] = entry->status.battery_temp;
    status["wifi_rssi"] = entry->status.wifi_rssi;
    status["free_heap"] = entry->status.free_heap;
    status["total_heap"] = entry->status.total_heap;
    status["uptime_seconds"] = entry->status.uptime_seconds;
    status["current_image"] = entry->status.current_image;
    status["last_error"] = entry->status.last_error;

    // Serialize JSON
    String json_string;
    serializeJson(doc, json_string);

    // Send HTTP POST request
    HTTPClient http;
    http.setConnectTimeout(10000);  // 10 second connect timeout
    http.setTimeout(15000);          // 15 second total timeout

    // Construct URL (assume TRMNL server)
    String url = "https://api.usetrmnl.com/api/log";

    if (!http.begin(url)) {
        Serial.println("[API_LOG] ERROR: HTTP begin failed");
        return false;
    }

    // Set headers
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Access-Token", api_key);

    // Send POST request
    int httpCode = http.POST(json_string);

    Serial.print("[API_LOG] HTTP response code: ");
    Serial.println(httpCode);

    bool success = (httpCode == 200);

    http.end();

    return success;
}

bool trmnlApiSubmitLogBatch(const char* api_key, const char* device_id,
                            const log_entry_t* entries, uint32_t count) {
    if (!api_key || !device_id || !entries || count == 0) {
        Serial.println("[API_LOG] ERROR: Invalid parameters");
        return false;
    }

    Serial.print("[API_LOG] Submitting batch of ");
    Serial.print(count);
    Serial.println(" logs");

    // Create JSON document for batch
    DynamicJsonDocument doc(4096);
    doc["device_id"] = device_id;

    // Create array of log entries
    JsonArray logs = doc.createNestedArray("logs");

    for (uint32_t i = 0; i < count; i++) {
        JsonObject log_obj = logs.createNestedObject();
        serialize_log_entry(&entries[i], log_obj);
    }

    // Serialize JSON
    String json_string;
    serializeJson(doc, json_string);

    // Send HTTP POST request
    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(15000);

    String url = "https://api.usetrmnl.com/api/log/batch";

    if (!http.begin(url)) {
        Serial.println("[API_LOG] ERROR: HTTP begin failed");
        return false;
    }

    // Set headers
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Access-Token", api_key);

    // Send POST request
    int httpCode = http.POST(json_string);

    Serial.print("[API_LOG] HTTP response code: ");
    Serial.println(httpCode);

    bool success = (httpCode == 200);

    http.end();

    return success;
}
