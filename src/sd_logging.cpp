/**
 * @file sd_logging.cpp
 * @brief SD Card Daily Log Export Implementation (Phase 6 Extension)
 *
 * Exports device logs to SD card as human-readable daily files
 */

#include "sd_logging.h"
#include "logging.h"
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <time.h>

// ============================================================================
// Private State
// ============================================================================

static bool sd_available = false;

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Convert log level enum to string
 */
static const char* log_level_to_string(log_level_t level) {
    switch (level) {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR:   return "ERROR";
        default:          return "UNKNOWN";
    }
}

/**
 * @brief Format timestamp as readable string
 */
static String format_timestamp(uint32_t timestamp) {
    if (timestamp == 0) {
        return "[No Timestamp]";
    }

    time_t t = (time_t)timestamp;
    struct tm* timeinfo = localtime(&t);

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "[%04d-%02d-%02d %02d:%02d:%02d]",
             timeinfo->tm_year + 1900,
             timeinfo->tm_mon + 1,
             timeinfo->tm_mday,
             timeinfo->tm_hour,
             timeinfo->tm_min,
             timeinfo->tm_sec);

    return String(buffer);
}

/**
 * @brief Create log file header for new daily file
 */
static void write_log_file_header(const String& filename) {
    File file = SD.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        return;
    }

    file.println("========================================");

    // Extract date from filename (/logs/YYYY-MM-DD.log)
    String date = filename.substring(6, 16); // Skip "/logs/" and ".log"
    file.print("TRMNL Device Log - ");
    file.println(date);

    // Get device MAC address
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    file.print("Device MAC: ");
    file.println(mac_str);
    file.println("========================================\n");

    file.close();
}

// ============================================================================
// Public API Implementation
// ============================================================================

bool sd_logging_init() {
#if SD_LOGGING_ENABLE == 0
    Serial.println("[SD_LOG] SD logging disabled in config");
    return false;
#endif

    Serial.println("[SD_LOG] Initializing SD card...");

    // Pull CS pins HIGH to avoid SPI conflicts
    pinMode(BOARD_SD_CS, OUTPUT);
    digitalWrite(BOARD_SD_CS, HIGH);

    // Initialize SPI with T5S3 Pro pins
    SPI.begin(BOARD_SD_SCLK, BOARD_SD_MISO, BOARD_SD_MOSI);

    // Attempt SD card initialization
    if (!SD.begin(BOARD_SD_CS)) {
        Serial.println("[SD_LOG] No SD card detected - continuing without SD logging");
        sd_available = false;
        return false;
    }

    // Check card type
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[SD_LOG] No SD card attached");
        sd_available = false;
        return false;
    }

    // Log card info
    Serial.print("[SD_LOG] SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.print("[SD_LOG] SD Card Size: ");
    Serial.print((uint32_t)cardSize);
    Serial.println("MB");

    // Create /logs directory if it doesn't exist
    if (!SD.exists(SD_LOG_DIR)) {
        Serial.println("[SD_LOG] Creating /logs directory...");
        if (SD.mkdir(SD_LOG_DIR)) {
            Serial.println("[SD_LOG] /logs directory created");
        } else {
            Serial.println("[SD_LOG] ERROR: Failed to create /logs directory");
            sd_available = false;
            return false;
        }
    }

    sd_available = true;
    Serial.println("[SD_LOG] SD card initialized successfully");
    return true;
}

bool sd_logging_is_available() {
    return sd_available;
}

String sd_logging_get_daily_filename() {
    time_t now = time(NULL);
    if (now == 0) {
        Serial.println("[SD_LOG] WARNING: Time not set, using default filename");
        return String(SD_LOG_DIR) + "/unknown.log";
    }

    struct tm* timeinfo = localtime(&now);

    char filename[32];
    snprintf(filename, sizeof(filename), "%s/%04d-%02d-%02d.log",
             SD_LOG_DIR,
             timeinfo->tm_year + 1900,
             timeinfo->tm_mon + 1,
             timeinfo->tm_mday);

    return String(filename);
}

String sd_logging_format_entry(const log_entry_t* entry) {
    if (!entry) {
        return "";
    }

    String output = "";

    // Timestamp and log level
    output += format_timestamp(entry->timestamp);
    output += " ";
    output += log_level_to_string(entry->level);
    output += ": ";
    output += entry->message;
    output += "\n";

    // Error details (if this is an error with diagnostics)
    char detail_line[256];
    bool has_error_details = false;

    if (entry->level == LOG_ERROR) {
        // Check if we have HTTP/download error details
        if (entry->error.http_status_code != -1) {
            snprintf(detail_line, sizeof(detail_line),
                     "  HTTP Status: %d",
                     entry->error.http_status_code);
            output += detail_line;
            has_error_details = true;

            if (entry->error.failure_reason[0] != '\0') {
                snprintf(detail_line, sizeof(detail_line),
                         " (%s)", entry->error.failure_reason);
                output += detail_line;
            }
            output += "\n";
        }

        // Retry attempt info
        if (entry->error.retry_attempt > 0) {
            snprintf(detail_line, sizeof(detail_line),
                     "  Retry Attempt: %d/3, Time: %ums\n",
                     entry->error.retry_attempt,
                     entry->error.operation_time_ms);
            output += detail_line;
            has_error_details = true;
        }

        // Bytes transferred (for download errors)
        if (entry->error.bytes_transferred > 0) {
            snprintf(detail_line, sizeof(detail_line),
                     "  Bytes Transferred: %u\n",
                     entry->error.bytes_transferred);
            output += detail_line;
            has_error_details = true;
        }

        // PNG decode error details
        if (entry->error.png_error_code != -1) {
            snprintf(detail_line, sizeof(detail_line),
                     "  PNG Error Code: %d", entry->error.png_error_code);
            output += detail_line;

            if (entry->error.image_width > 0 && entry->error.image_height > 0) {
                snprintf(detail_line, sizeof(detail_line),
                         " (Image: %dx%d, %dbpp)",
                         entry->error.image_width,
                         entry->error.image_height,
                         entry->error.image_bpp);
                output += detail_line;
            }
            output += "\n";
            has_error_details = true;
        }

        // Generic failure reason (if not already shown)
        if (!has_error_details && entry->error.failure_reason[0] != '\0') {
            snprintf(detail_line, sizeof(detail_line),
                     "  Reason: %s\n",
                     entry->error.failure_reason);
            output += detail_line;
        }
    }

    // Device status (indented)
    char status_line[256];

    // Battery and charging status
    snprintf(status_line, sizeof(status_line),
             "  Battery: %.2fV (%d%%, %d°C), Heap: %.1fMB/%.1fMB, WiFi: %ddBm\n",
             entry->status.battery_voltage,
             entry->status.battery_soc,
             entry->status.battery_temp,
             entry->status.free_heap / 1048576.0f,
             entry->status.total_heap / 1048576.0f,
             entry->status.wifi_rssi);
    output += status_line;

    // Uptime and current image
    snprintf(status_line, sizeof(status_line),
             "  Uptime: %us, Image: %s\n",
             entry->status.uptime_seconds,
             entry->status.current_image);
    output += status_line;

    // Last error
    snprintf(status_line, sizeof(status_line),
             "  Last Error: %s\n",
             entry->status.last_error);
    output += status_line;

    return output;
}

bool sd_logging_export_entry(const log_entry_t* entry) {
    if (!sd_available) {
        // Silently return if SD not available (no errors)
        return false;
    }

    if (!entry) {
        return false;
    }

    // Get daily log filename
    String filename = sd_logging_get_daily_filename();

    // Check if file exists - if not, write header
    bool file_exists = SD.exists(filename.c_str());

    // Open file in append mode
    File file = SD.open(filename.c_str(), FILE_APPEND);
    if (!file) {
        Serial.print("[SD_LOG] ERROR: Failed to open log file: ");
        Serial.println(filename);
        return false;
    }

    // Write header if this is a new file
    if (!file_exists) {
        file.close();
        write_log_file_header(filename);
        file = SD.open(filename.c_str(), FILE_APPEND);
        if (!file) {
            Serial.println("[SD_LOG] ERROR: Failed to reopen log file after header");
            return false;
        }
    }

    // Format and write log entry
    String formatted = sd_logging_format_entry(entry);
    file.print(formatted);
    file.println(); // Extra blank line between entries

    file.close();

    return true;
}

bool sd_logging_export_all() {
    if (!sd_available) {
        Serial.println("[SD_LOG] SD card not available");
        return false;
    }

    Serial.println("[SD_LOG] Exporting all logs to SD card...");

    // Read all entries from SPIFFS
    uint32_t count = 0;
    log_entry_t* entries = logging_read_all(&count);

    if (!entries || count == 0) {
        Serial.println("[SD_LOG] No logs to export");
        return false;
    }

    Serial.print("[SD_LOG] Exporting ");
    Serial.print(count);
    Serial.println(" entries...");

    // Export each entry
    uint32_t exported = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (sd_logging_export_entry(&entries[i])) {
            exported++;
        }
    }

    free(entries);

    Serial.print("[SD_LOG] Export complete: ");
    Serial.print(exported);
    Serial.print("/");
    Serial.print(count);
    Serial.println(" entries exported");

    return (exported > 0);
}

void sd_logging_list_files() {
    if (!sd_available) {
        Serial.println("[SD_LOG] SD card not available");
        return;
    }

    Serial.println("[SD_LOG] Listing log files:");

    File root = SD.open(SD_LOG_DIR);
    if (!root) {
        Serial.println("[SD_LOG] ERROR: Failed to open /logs directory");
        return;
    }

    if (!root.isDirectory()) {
        Serial.println("[SD_LOG] ERROR: /logs is not a directory");
        root.close();
        return;
    }

    File file = root.openNextFile();
    int file_count = 0;
    while (file) {
        if (!file.isDirectory()) {
            Serial.print("  ");
            Serial.print(file.name());
            Serial.print(" - ");
            Serial.print(file.size());
            Serial.println(" bytes");
            file_count++;
        }
        file = root.openNextFile();
    }

    root.close();

    Serial.print("[SD_LOG] Total: ");
    Serial.print(file_count);
    Serial.println(" log files");
}

void sd_logging_print_info() {
    if (!sd_available) {
        Serial.println("[SD_LOG] SD card not available");
        return;
    }

    uint64_t totalBytes = SD.totalBytes();
    uint64_t usedBytes = SD.usedBytes();

    Serial.println("[SD_LOG] SD Card Info:");
    Serial.print("  Total: ");
    Serial.print((uint32_t)(totalBytes / (1024 * 1024)));
    Serial.println("MB");
    Serial.print("  Used: ");
    Serial.print((uint32_t)(usedBytes / (1024 * 1024)));
    Serial.println("MB");
    Serial.print("  Free: ");
    Serial.print((uint32_t)((totalBytes - usedBytes) / (1024 * 1024)));
    Serial.println("MB");
}
