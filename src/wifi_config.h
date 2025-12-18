/**
 * @file wifi_config.h
 * @brief WiFi configuration and connection management
 *
 * Handles WiFi setup, connection, and status checking for T5S3 Pro board.
 */

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFi.h>
#include <string.h>
#include <cstdio>

// ============================================================================
// CONFIGURATION - Edit these values for your WiFi network
// ============================================================================

#define WIFI_SSID ""           // WiFi network name
#define WIFI_PASSWORD ""    // WiFi password

// TRMNL API Configuration
// The device will register with TRMNL backend and get an image URL
#define TRMNL_API_BASE "https://trmnl.app"
#define TRMNL_API_SETUP TRMNL_API_BASE "/api/setup"
#define TRMNL_API_DISPLAY TRMNL_API_BASE "/api/display"

// Your TRMNL API Key - Get this from https://usetrmnl.com after registering your device
// On first boot without an API key, the device will call /api/setup to register using MAC address
// After registration, update this with your API key
#define TRMNL_API_KEY ""  // API key from usetrmnl.com

// For initial testing, use a public test BMP image
// Once you register with TRMNL, this will be replaced by the API response
#define TRMNL_TEST_IMAGE_URL "https://www.sample-videos.com/img/sample-bmp.bmp"

// NOTE: Deep sleep and shutdown timer settings have been moved to include/config.h
// to avoid duplicate definitions. Edit config.h to adjust these values.

// WiFi connection timeout (seconds)
#define WIFI_CONNECT_TIMEOUT 15

// ============================================================================
// WiFi Functions
// ============================================================================
// NOTE: These inline functions are defined here but not currently used by the
// modularized firmware. They're kept for potential future use.
// If you need any of these functions, uncomment them below.

/*
 * NOTE: These functions have been disabled due to compilation issues with
 * inline functions across multiple translation units in the modular build.
 * To use them, move to wifi.cpp as non-inline functions or ensure they're
 * only instantiated in files that properly include WiFi.h before this header.

inline bool wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    return true;
}

inline bool wifi_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

inline const char* wifi_get_ssid() {
    return WiFi.SSID().c_str();
}

inline const char* wifi_get_ip() {
    return WiFi.localIP().toString().c_str();
}

inline int wifi_get_rssi() {
    return WiFi.RSSI();
}

inline void wifi_deinit() {
    WiFi.disconnect(true);
}

inline void wifi_debug_info() {
    if (WiFi.status() == WL_CONNECTED) {
        printf("WiFi Status: CONNECTED\n");
        printf("SSID: %s\n", WiFi.SSID().c_str());
        printf("IP: %s\n", WiFi.localIP().toString().c_str());
        printf("RSSI: %d dBm\n", WiFi.RSSI());
    } else {
        printf("WiFi Status: DISCONNECTED\n");
        printf("WiFi State: %d\n", WiFi.status());
    }
}
*/

#endif // WIFI_CONFIG_H
