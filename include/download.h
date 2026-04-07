/**
 * @file download.h
 * @brief Image download functions
 *
 * EXTRACTED FROM: TRMNL-T5S3-Pro/src/main.cpp (lines 273-348)
 * Handles HTTP/HTTPS downloads with streaming to SPIFFS
 * Phase 5: Added retry tracking
 */

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

// Structure to track download attempt information
typedef struct {
    bool success;           // true if download succeeded
    int attempts_made;      // Number of attempts made (1-3)
} download_result_t;

/**
 * @brief Download image from HTTP/HTTPS URL to SPIFFS with automatic retry
 *
 * Streams data in 512-byte chunks to conserve memory.
 * Does NOT buffer entire file - writes directly to disk.
 *
 * Features:
 * - HTTP/HTTPS support
 * - 3 automatic retry attempts on failure
 * - Exponential backoff: 10s, 20s, 30s between retries
 * - 10-second connect timeout per attempt
 * - 30-second total timeout per attempt
 * - Chunked 512-byte reading
 * - Progress reporting every 10KB
 * - Automatic old file cleanup
 * - Memory-efficient streaming
 *
 * Retry Logic:
 * - Attempt 1: Immediate try
 * - Attempt 2: If failed, wait 10s then retry
 * - Attempt 3: If failed, wait 20s then retry
 * - Attempt 4: If failed, wait 30s then retry
 * - Returns false if all 4 attempts fail
 *
 * @param url Full HTTP/HTTPS URL to image file
 * @param dest_path SPIFFS destination path (e.g., "/spiffs/display.bmp")
 *
 * @return download_result_t struct containing:
 *   - success: true if download succeeded on any attempt
 *   - attempts_made: number of attempts made (1-3)
 *
 * Time: 3-10 seconds on success, up to 60+ seconds if retrying
 * Power: 100 mA during download
 *
 * Example:
 *   download_result_t result = download_image("https://example.com/image.png", "/spiffs/display.bmp");
 *   if (result.success) {
 *       display_image("/spiffs/display.bmp");
 *   } else {
 *       char msg[32];
 *       snprintf(msg, sizeof(msg), "Download Failed (Attempt %d/3)", result.attempts_made);
 *       display_text(msg);
 *   }
 */
download_result_t download_image(const char* url, const char* dest_path);

#endif // DOWNLOAD_H
