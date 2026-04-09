/**
 * @file wifi.cpp
 * @brief WiFi Implementation
 *
 * EXTRACTED FROM: TRMNL-T5S3-Pro/src/main.cpp
 * Lines 637-658 and 696
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "wifi_module.h"
#include "wifi_config.h"
#include "config.h"
#include "test_config.h"

// WiFi connection implementation
bool wifi_connect_blocking() {
    Serial.println("[WIFI] Starting connection...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < WIFI_CONNECT_TIMEOUT) {
        delay(1000);
        retry++;
        Serial.print(".");
    }

    return WiFi.status() == WL_CONNECTED;
}

void wifi_disconnect_and_poweroff() {
    WiFi.disconnect(true);  // true = power off radio
}

// ============================================================================
// WiFi Status Decoder (Debug Utility)
// ============================================================================

/**
 * @brief Convert WiFi.status() code to human-readable string
 *
 * Helps debug WiFi connection issues by translating status codes.
 * All possible wl_status_t values from ESP32 WiFi library.
 *
 * @param status WiFi status code from WiFi.status()
 * @return Human-readable status string
 */
const char* wifi_status_string(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS:      return "Idle";
        case WL_NO_SSID_AVAIL:    return "No SSID Available";
        case WL_SCAN_COMPLETED:   return "Scan Completed";
        case WL_CONNECTED:        return "Connected";
        case WL_CONNECT_FAILED:   return "Connection Failed";
        case WL_CONNECTION_LOST:  return "Connection Lost";
        case WL_DISCONNECTED:     return "Disconnected";
        default:                  return "Unknown";
    }
}

/**
 * @brief Print current WiFi status to Serial (debug helper)
 *
 * Example output: "[WIFI_DEBUG] Status: Connected (3)"
 */
void debug_wifi_status() {
#if DEBUG_WIFI_STATUS
    wl_status_t status = WiFi.status();
    Serial.print("[WIFI_DEBUG] Status: ");
    Serial.print(wifi_status_string(status));
    Serial.print(" (");
    Serial.print(status);
    Serial.println(")");
#endif
}
