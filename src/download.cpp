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
 * @param url URL to download
 * @param dest_path Destination file path
 * @param attempt_num Attempt number (0-based)
 * @param bytes_downloaded_out Output parameter for bytes downloaded
 * @param error_reason_out Output buffer for error description (64 bytes)
 * @return HTTP response code, or negative number on network error
 */
static int attempt_single_download(const char* url, const char* dest_path, int attempt_num,
                                   uint32_t* bytes_downloaded_out, char* error_reason_out) {
    Serial.print("[DOWNLOAD] Attempt ");
    Serial.print(attempt_num + 1);
    Serial.print("/");
    Serial.print(DOWNLOAD_MAX_RETRIES);
    Serial.print(": ");

    if (bytes_downloaded_out) *bytes_downloaded_out = 0;
    if (error_reason_out) error_reason_out[0] = '\0';

    HTTPClient http;
    http.setConnectTimeout(10000);  // 10 second connect timeout
    http.setTimeout(30000);          // 30 second total timeout

    if (!http.begin(url)) {
        Serial.println("HTTP begin failed");
        if (error_reason_out) {
            snprintf(error_reason_out, 64, "HTTP begin failed");
        }
        return -1;
    }

    int httpCode = http.GET();
    Serial.println(httpCode);

    if (httpCode != HTTP_CODE_OK) {
        if (error_reason_out) {
            if (httpCode > 0) {
                snprintf(error_reason_out, 64, "HTTP %d", httpCode);
            } else if (httpCode == -1) {
                snprintf(error_reason_out, 64, "Connection failed");
            } else if (httpCode == -11) {
                snprintf(error_reason_out, 64, "Read timeout");
            } else {
                snprintf(error_reason_out, 64, "Network error %d", httpCode);
            }
        }
        http.end();
        return httpCode;
    }

    int totalLength = http.getSize();
    Serial.print("[DOWNLOAD] File size: ");
    Serial.println(totalLength);

    if (totalLength <= 0) {
        Serial.println("[DOWNLOAD] Invalid content length");
        if (error_reason_out) {
            snprintf(error_reason_out, 64, "Invalid content length: %d", totalLength);
        }
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
        if (error_reason_out) {
            snprintf(error_reason_out, 64, "SPIFFS open failed");
        }
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

    if (bytes_downloaded_out) *bytes_downloaded_out = bytesDownloaded;

    Serial.print("[DOWNLOAD] Complete! Downloaded: ");
    Serial.print(bytesDownloaded);
    Serial.print(" bytes");

    // === INTEGRITY VERIFICATION ===

    // Verify file size matches expected
    if (bytesDownloaded != totalLength) {
        Serial.println();
        Serial.print("[DOWNLOAD] ⚠ SIZE MISMATCH: Expected ");
        Serial.print(totalLength);
        Serial.print(" bytes, got ");
        Serial.print(bytesDownloaded);
        Serial.println(" bytes");
        if (error_reason_out) {
            snprintf(error_reason_out, 64, "Size mismatch: %d/%d bytes", bytesDownloaded, totalLength);
        }
        return -1;
    }
    Serial.println(" ✓");

    // Verify file exists in SPIFFS
    if (!SPIFFS.exists(dest_path)) {
        Serial.print("[DOWNLOAD] ✗ ERROR: File not found in SPIFFS after write: ");
        Serial.println(dest_path);
        if (error_reason_out) {
            snprintf(error_reason_out, 64, "File not found after write");
        }
        return -1;
    }

    // Verify SPIFFS file size matches download
    File verify_file = SPIFFS.open(dest_path, "r");
    if (!verify_file) {
        Serial.print("[DOWNLOAD] ✗ ERROR: Cannot open file for verification: ");
        Serial.println(dest_path);
        if (error_reason_out) {
            snprintf(error_reason_out, 64, "Cannot verify file");
        }
        return -1;
    }

    size_t spiffs_size = verify_file.size();
    verify_file.close();

    if (spiffs_size != totalLength) {
        Serial.print("[DOWNLOAD] ✗ SPIFFS SIZE MISMATCH: File shows ");
        Serial.print(spiffs_size);
        Serial.print(" bytes, expected ");
        Serial.print(totalLength);
        Serial.println(" bytes");
        if (error_reason_out) {
            snprintf(error_reason_out, 64, "SPIFFS mismatch: %d/%d bytes", spiffs_size, totalLength);
        }
        return -1;
    }

    Serial.print("[DOWNLOAD] ✓ Integrity verified: ");
    Serial.print(dest_path);
    Serial.print(" (");
    Serial.print(spiffs_size);
    Serial.println(" bytes)");

    if (error_reason_out) {
        snprintf(error_reason_out, 64, "Success");
    }

    return HTTP_CODE_OK;
}

// ============================================================================
// Public Interface
// ============================================================================

download_result_t download_image(const char* url, const char* dest_path) {
    download_result_t result = {
        .success = false,
        .attempts_made = 0,
        .http_status_code = -1,
        .total_time_ms = 0,
        .bytes_downloaded = 0,
        .error_reason = {0}
    };

    Serial.print("[DOWNLOAD] Fetching: ");
    Serial.println(url);

    uint32_t start_time = millis();

    // Try up to DOWNLOAD_MAX_RETRIES attempts
    for (int attempt = 0; attempt < DOWNLOAD_MAX_RETRIES; attempt++) {
        result.attempts_made = attempt + 1;

        uint32_t attempt_start = millis();
        uint32_t bytes_this_attempt = 0;

        int http_result = attempt_single_download(url, dest_path, attempt,
                                                  &bytes_this_attempt, result.error_reason);

        uint32_t attempt_time = millis() - attempt_start;
        result.http_status_code = http_result;
        result.bytes_downloaded = bytes_this_attempt;

        Serial.print("[DOWNLOAD] Attempt took ");
        Serial.print(attempt_time);
        Serial.println("ms");

        if (http_result == HTTP_CODE_OK) {
            // Success on this attempt
            result.success = true;
            result.total_time_ms = millis() - start_time;
            snprintf(result.error_reason, sizeof(result.error_reason), "Success");
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

    result.total_time_ms = millis() - start_time;

    Serial.print("[DOWNLOAD] Final result: Failed after ");
    Serial.print(result.attempts_made);
    Serial.print(" attempts, ");
    Serial.print(result.total_time_ms);
    Serial.print("ms total. Last error: ");
    Serial.println(result.error_reason);

    return result;
}
