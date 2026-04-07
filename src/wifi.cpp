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
