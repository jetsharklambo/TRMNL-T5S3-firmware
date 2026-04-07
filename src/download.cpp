/**
 * @file download.cpp
 * @brief Image Download Implementation with Retry Logic
 *
 * EXTRACTED FROM: TRMNL-T5S3-Pro/src/main.cpp (lines 273-348)
 * Phase 5: Added 3 download retries with exponential backoff (10s, 20s, 30s)
 */

#include "download.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

// ============================================================================
// Constants
// ============================================================================

#define DOWNLOAD_MAX_RETRIES 3
static const uint32_t DOWNLOAD_RETRY_DELAYS[DOWNLOAD_MAX_RETRIES] = {10000, 20000, 30000};

// ============================================================================
// Internal Helper Function
// ============================================================================

/**
 * @brief Attempt a single download with timeout handling
 * @return HTTP response code, or negative number on network error
 */
static int attempt_single_download(const char* url, const char* dest_path, int attempt_num) {
    Serial.print("[DOWNLOAD] Attempt ");
    Serial.print(attempt_num + 1);
    Serial.print("/");
    Serial.print(DOWNLOAD_MAX_RETRIES);
    Serial.print(": ");

    HTTPClient http;
    http.setConnectTimeout(10000);  // 10 second connect timeout
    http.setTimeout(30000);          // 30 second total timeout

    if (!http.begin(url)) {
        Serial.println("HTTP begin failed");
        return -1;
    }

    int httpCode = http.GET();
    Serial.println(httpCode);

    if (httpCode != HTTP_CODE_OK) {
        http.end();
        return httpCode;
    }

    int totalLength = http.getSize();
    Serial.print("[DOWNLOAD] File size: ");
    Serial.println(totalLength);

    if (totalLength <= 0) {
        Serial.println("[DOWNLOAD] Invalid content length");
        http.end();
        return -1;
    }

    // Delete old file if exists
    if (SPIFFS.exists(dest_path)) {
        SPIFFS.remove(dest_path);
    }

    File file = SPIFFS.open(dest_path, "w");
    if (!file) {
        Serial.println("[DOWNLOAD] Failed to open file for writing");
        http.end();
        return -1;
    }

    WiFiClient* stream = http.getStreamPtr();
    uint8_t buffer[512];
    int bytesDownloaded = 0;

    while (http.connected() && bytesDownloaded < totalLength) {
        size_t available = stream->available();
        if (available > 0) {
            int bytesRead = stream->readBytes(buffer, (available < sizeof(buffer)) ? available : sizeof(buffer));
            if (bytesRead > 0) {
                file.write(buffer, bytesRead);
                bytesDownloaded += bytesRead;

                if (bytesDownloaded % 10240 == 0) {
                    Serial.print("[DOWNLOAD] Progress: ");
                    Serial.print(bytesDownloaded);
                    Serial.print("/");
                    Serial.println(totalLength);
                }
            }
        }
        vTaskDelay(1);  // Yield to other tasks
    }

    file.close();
    http.end();

    Serial.print("[DOWNLOAD] Complete! Downloaded: ");
    Serial.println(bytesDownloaded);

    return (bytesDownloaded > 0) ? HTTP_CODE_OK : -1;
}

// ============================================================================
// Public Interface
// ============================================================================

download_result_t download_image(const char* url, const char* dest_path) {
    download_result_t result = {false, 0};

    Serial.print("[DOWNLOAD] Fetching: ");
    Serial.println(url);

    // Try up to DOWNLOAD_MAX_RETRIES attempts
    for (int attempt = 0; attempt < DOWNLOAD_MAX_RETRIES; attempt++) {
        result.attempts_made = attempt + 1;
        int http_result = attempt_single_download(url, dest_path, attempt);

        if (http_result == HTTP_CODE_OK) {
            // Success on this attempt
            result.success = true;
            return result;
        }

        // Failed - check if we should retry
        if (attempt < DOWNLOAD_MAX_RETRIES - 1) {
            // Not the last attempt, wait before retrying
            uint32_t delay_ms = DOWNLOAD_RETRY_DELAYS[attempt];
            Serial.print("[DOWNLOAD] Failed - waiting ");
            Serial.print(delay_ms / 1000);
            Serial.println("s before retry...");
            delay(delay_ms);
            Serial.print("[DOWNLOAD] Attempting retry ");
            Serial.print(attempt + 2);
            Serial.print("/");
            Serial.println(DOWNLOAD_MAX_RETRIES);
        } else {
            // Last attempt failed
            Serial.println("[DOWNLOAD] All retries exhausted - giving up");
        }
    }

    return result;
}
