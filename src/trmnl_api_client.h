/**
 * @file trmnl_api_client.h
 * @brief TRMNL API Client - Communicate with TRMNL backend
 *
 * Implements /api/setup and /api/display endpoints
 * Base URL: https://trmnl.app
 */

#ifndef TRMNL_API_CLIENT_H
#define TRMNL_API_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "power.h"
#include "config.h"

#define TRMNL_API_BASE "https://trmnl.app"
#define TRMNL_API_SETUP TRMNL_API_BASE "/api/setup"
#define TRMNL_API_DISPLAY TRMNL_API_BASE "/api/display"

// Structure for setup response
struct TRMNLSetupResponse {
    int status;
    char api_key[64];
    char friendly_id[16];
    char image_url[512];
    char filename[128];
};

// Structure for display response
struct TRMNLDisplayResponse {
    int status;
    char image_url[512];
    char filename[128];
    int refresh_rate;
    bool update_firmware;
    char firmware_url[512];
    bool reset_firmware;
};

/**
 * @brief Get device MAC address as XX:XX:XX:XX:XX:XX format
 */
String getTRMNLMacAddress() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    Serial.print("[TRMNL] MAC Address: ");
    Serial.println(macStr);

    return String(macStr);
}

/**
 * @brief Call /api/setup to register device and get API key
 * First time setup - exchange MAC for API key
 */
bool trmnlApiSetup(TRMNLSetupResponse* response) {
    if (!response) {
        Serial.println("[TRMNL] Setup response pointer is null");
        return false;
    }

    Serial.println("[TRMNL] Calling /api/setup for device registration...");

    String macAddress = getTRMNLMacAddress();

    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(15000);

    Serial.print("[TRMNL] GET ");
    Serial.println(TRMNL_API_SETUP);

    if (!http.begin(TRMNL_API_SETUP)) {
        Serial.println("[TRMNL] Failed to begin HTTP connection");
        return false;
    }

    // Add required headers
    http.addHeader("Content-Type", "application/json");
    http.addHeader("ID", macAddress);
    http.addHeader("FW-Version", FIRMWARE_VERSION);

    int httpCode = http.GET();
    Serial.print("[TRMNL] HTTP Response Code: ");
    Serial.println(httpCode);

    if (httpCode != HTTP_CODE_OK) {
        Serial.print("[TRMNL] Setup failed with code: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    Serial.print("[TRMNL] Response: ");
    Serial.println(payload);

    // Parse JSON response
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.print("[TRMNL] JSON parse error: ");
        Serial.println(error.c_str());
        return false;
    }

    // Extract fields
    response->status = doc["status"] | 0;
    const char* api_key = doc["api_key"] | "";
    const char* friendly_id = doc["friendly_id"] | "";
    const char* image_url = doc["image_url"] | "";
    const char* filename = doc["filename"] | "";

    strncpy(response->api_key, api_key, sizeof(response->api_key) - 1);
    strncpy(response->friendly_id, friendly_id, sizeof(response->friendly_id) - 1);
    strncpy(response->image_url, image_url, sizeof(response->image_url) - 1);
    strncpy(response->filename, filename, sizeof(response->filename) - 1);

    Serial.print("[TRMNL] Status: ");
    Serial.println(response->status);
    Serial.print("[TRMNL] API Key: ");
    Serial.println(response->api_key);
    Serial.print("[TRMNL] Friendly ID: ");
    Serial.println(response->friendly_id);
    Serial.print("[TRMNL] Image URL: ");
    Serial.println(response->image_url);

    // Success: status 200 with api_key present
    return response->status == 200 && strlen(response->api_key) > 0;
}

/**
 * @brief Call /api/display to get current image URL
 * Requires API key from previous setup call
 */
bool trmnlApiDisplay(const char* api_key, TRMNLDisplayResponse* response) {
    if (!api_key || !response) {
        Serial.println("[TRMNL] Invalid parameters");
        return false;
    }

    Serial.println("[TRMNL] Calling /api/display for image update...");

    String macAddress = getTRMNLMacAddress();

    // Get telemetry before making request
    float battery_voltage = power_get_battery_voltage();
    uint8_t battery_percentage = power_get_state_of_charge();
    const char* battery_status = power_get_battery_status();
    bool is_charging = power_is_charging();
    float battery_temp = power_get_battery_temperature();
    int wifi_rssi = WiFi.RSSI();

    Serial.print("[TRMNL] Telemetry - Battery: ");
    Serial.print(battery_voltage, 2);
    Serial.print("V (");
    Serial.print(battery_percentage);
    Serial.print("%, ");
    Serial.print(battery_status);
    Serial.print(", ");
    Serial.print(is_charging ? "CHARGING" : "DISCHARGING");
    Serial.print(", ");
    Serial.print(battery_temp, 1);
    Serial.print("°C), RSSI: ");
    Serial.print(wifi_rssi);
    Serial.println("dBm");

    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(15000);

    Serial.print("[TRMNL] GET ");
    Serial.println(TRMNL_API_DISPLAY);

    if (!http.begin(TRMNL_API_DISPLAY)) {
        Serial.println("[TRMNL] Failed to begin HTTP connection");
        return false;
    }

    // Format telemetry values as strings for headers
    char battery_str[16];
    snprintf(battery_str, sizeof(battery_str), "%.2f", battery_voltage);

    char battery_pct_str[4];
    snprintf(battery_pct_str, sizeof(battery_pct_str), "%d", battery_percentage);

    char battery_temp_str[8];
    snprintf(battery_temp_str, sizeof(battery_temp_str), "%.1f", battery_temp);

    char rssi_str[8];
    snprintf(rssi_str, sizeof(rssi_str), "%d", wifi_rssi);

    // Add required headers with real telemetry from BQ27220
    http.addHeader("Content-Type", "application/json");
    http.addHeader("ID", macAddress);  // Use MAC address as ID
    http.addHeader("Access-Token", api_key);  // Use API key for authentication
    http.addHeader("Refresh-Rate", "10");  // Our 10-second interval
    http.addHeader("Battery-Voltage", battery_str);  // Raw voltage from BQ27220: 3.7V
    http.addHeader("Battery-Percentage", battery_pct_str);  // State of charge: 50%
    http.addHeader("Battery-Status", battery_status);  // Status: normal/low/critical/full/charged
    http.addHeader("Battery-Charging", is_charging ? "true" : "false");  // REAL charging status!
    http.addHeader("Battery-Temperature", battery_temp_str);  // Battery temp in °C
    http.addHeader("RSSI", rssi_str);  // WiFi signal strength in dBm
    http.addHeader("FW-Version", FIRMWARE_VERSION);

    int httpCode = http.GET();
    Serial.print("[TRMNL] HTTP Response Code: ");
    Serial.println(httpCode);

    if (httpCode != HTTP_CODE_OK) {
        Serial.print("[TRMNL] Display failed with code: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    Serial.print("[TRMNL] Response: ");
    Serial.println(payload);

    // Parse JSON response
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.print("[TRMNL] JSON parse error: ");
        Serial.println(error.c_str());
        return false;
    }

    // Extract fields
    response->status = doc["status"] | 0;
    const char* image_url = doc["image_url"] | "";
    const char* filename = doc["filename"] | "";
    const char* firmware_url = doc["firmware_url"] | "";

    // Ensure null-termination and proper bounds
    size_t url_len = strlen(image_url);
    if (url_len > 0) {
        if (url_len >= sizeof(response->image_url)) {
            Serial.print("[TRMNL] WARNING: Image URL truncated from ");
            Serial.print(url_len);
            Serial.print(" to ");
            Serial.println(sizeof(response->image_url) - 1);
        }
        strncpy(response->image_url, image_url, sizeof(response->image_url) - 1);
        response->image_url[sizeof(response->image_url) - 1] = '\0';
    }

    strncpy(response->filename, filename, sizeof(response->filename) - 1);
    response->filename[sizeof(response->filename) - 1] = '\0';
    response->refresh_rate = doc["refresh_rate"] | 60;
    response->update_firmware = doc["update_firmware"] | false;
    strncpy(response->firmware_url, firmware_url, sizeof(response->firmware_url) - 1);
    response->firmware_url[sizeof(response->firmware_url) - 1] = '\0';
    response->reset_firmware = doc["reset_firmware"] | false;

    Serial.print("[TRMNL] Status: ");
    Serial.println(response->status);
    Serial.print("[TRMNL] Image URL length: ");
    Serial.println(strlen(response->image_url));
    Serial.print("[TRMNL] Image URL: ");
    Serial.println(response->image_url);
    Serial.print("[TRMNL] Filename: ");
    Serial.println(response->filename);
    Serial.print("[TRMNL] Refresh Rate: ");
    Serial.println(response->refresh_rate);

    // Success: status 0 with image_url present (or 500 with error for unregistered device)
    if (response->status == 500) {
        Serial.println("[TRMNL] Device not yet registered - need to run /api/setup first");
        return false;
    }
    // Display endpoint returns status 0 for success
    return response->status == 0 && strlen(response->image_url) > 0;
}

#endif // TRMNL_API_CLIENT_H
