/**
 * @file main.cpp
 * @brief TRMNL T5S3 Pro - Modular Firmware (Phases 1-7)
 *
 * SIMPLIFIED main loop - all logic extracted to separate modules:
 * - display.cpp: E-paper display operations
 * - wifi.cpp: WiFi connectivity
 * - download.cpp: Image downloading
 * - sleep.cpp: Power management
 * - button.cpp: Button wakeup detection (Phase 5)
 * - power.cpp: Battery voltage and memory monitoring (Phase 5)
 * - logging.cpp: Centralized logging system (Phase 6)
 * - sd_logging.cpp: SD card log export (Phase 6)
 * - ota.cpp: Over-the-air firmware updates (Phase 7)
 *
 * This is the LINEAR FLOW version before state machine (Phase 8).
 * All extracted functions work EXACTLY as before.
 *
 * Phase 5 additions:
 * - Button module detects GPIO48 wakeup vs timer wakeup
 * - Power module reads battery voltage via BQ27220 fuel gauge
 * - Free heap memory monitoring for debugging
 *
 * Phase 6 additions:
 * - Centralized logging with device telemetry
 * - SPIFFS circular buffer storage (100 entries)
 * - SD card daily log export (human-readable)
 *
 * Phase 7 additions:
 * - OTA firmware updates from TRMNL server
 * - Configurable firmware URL (custom/API/official)
 * - Battery safety checks and timeout protection
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <esp_ota_ops.h>
#include <SPIFFS.h>

// Include all module headers
#include "display.h"
#include "wifi_module.h"
#include "download.h"
#include "sleep.h"
#include "config.h"
#include "nvram_config.h"
#include "wifi_config.h"
#include "trmnl_api_client.h"
#include "ap_server.h"
#include "button.h"        // Phase 5: Button detection
#include "power.h"         // Phase 5: Battery monitoring
#include "logging.h"       // Phase 6: Central logging API
#include "sd_logging.h"    // Phase 6: SD card log export
#include "device_status.h" // Phase 6: Device status tracking
#include "ota.h"           // Phase 7: OTA firmware updates
#include "api-client/submit_log.h"  // Phase 7: API log submission
#include "test_config.h"   // Test/debug configuration

// ============================================================================
// RTC Memory for Test State Tracking
// ============================================================================
// RTC memory persists across deep sleep but resets on power cycle
// Used to track boot count for error injection testing

#if (TEST_DOWNLOAD_ERROR || TEST_DECODE_ERROR || TEST_WIFI_ERROR)
RTC_DATA_ATTR int test_boot_count = 0;
#endif

// ============================================================================
// Button Handler (Phase 5)
// ============================================================================

/**
 * @brief Handle button events (press, hold, etc)
 *
 * Supports:
 * - Short press: Just detect and log
 * - 5s hold: Soft reset (reload config, reboot)
 * - 15s hold: Hard reset (clear all credentials, enter AP mode)
 */
void button_event_handler(button_event_t event, uint32_t hold_duration_ms) {
    switch (event) {
        case BUTTON_EVENT_PRESSED:
            Serial.println("[BUTTON] Button pressed by user");
            break;

        case BUTTON_EVENT_RELEASED:
            Serial.print("[BUTTON] Button released after ");
            Serial.print(hold_duration_ms);
            Serial.println("ms");
            break;

        case BUTTON_EVENT_HELD_5S:
            Serial.println("[BUTTON] 5-second hold detected - performing SOFT RESET");
            display_text("Rebooting...");
            delay(2000);
            ESP.restart();  // Soft reboot
            break;

        case BUTTON_EVENT_HELD_15S:
            Serial.println("[BUTTON] 15-second hold detected - performing HARD RESET");
            display_text("Resetting Device...");
            delay(2000);

            // Clear all credentials
            if (nvram_clear_all()) {
                Serial.println("[BUTTON] Credentials cleared, rebooting into AP mode");
                delay(1000);
                ESP.restart();  // Reboot into AP mode
            } else {
                Serial.println("[BUTTON] ERROR: Failed to clear credentials");
                display_text("Reset Failed");
                delay(3000);
            }
            break;
    }
}

// ============================================================================
// API Log Submission Helper (Phase 7)
// ============================================================================

/**
 * @brief Submit the most recent error log to TRMNL server
 *
 * Retrieves credentials from NVRAM and submits the last log entry
 * to /api/log endpoint for remote diagnostics.
 *
 * Only submits if device has API credentials (post-registration).
 */
void submit_error_log_to_server() {
    // Read API credentials from NVRAM
    char api_key[256], friendly_id[64];
    nvram_read_api_key(api_key, sizeof(api_key));
    nvram_read_friendly_id(friendly_id, sizeof(friendly_id));

    // Only submit if we have credentials (device is registered)
    if (strlen(api_key) == 0 || strlen(friendly_id) == 0) {
        Serial.println("[API_LOG] Skipping log submission - device not registered yet");
        return;
    }

    // Get all log entries
    uint32_t count = 0;
    log_entry_t* entries = logging_read_all(&count);

    if (!entries || count == 0) {
        Serial.println("[API_LOG] No logs to submit");
        return;
    }

    // Submit most recent entry (last in array)
    Serial.print("[API_LOG] Submitting log to server: ");
    Serial.println(entries[count - 1].message);

    bool success = trmnlApiSubmitLog(api_key, friendly_id, &entries[count - 1]);

    if (success) {
        Serial.println("[API_LOG] Log submitted successfully");
    } else {
        Serial.println("[API_LOG] Failed to submit log to server");
    }

    // Free memory
    free(entries);
}

// ============================================================================
// Serial Command Handler (Phase 2c)
// ============================================================================

/**
 * @brief Handle serial commands for device control
 *
 * Supports:
 * - "clear" - Erase all credentials (NVS + WiFi) and prepare for AP mode
 *
 * This function is called during startup to listen for early commands
 * before the main application continues.
 */
void handle_serial_commands() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();

        if (command == "clear") {
            Serial.println("\n[SERIAL] Clear command received!");
            Serial.println("[SERIAL] Erasing all credentials...");

            // Clear NVRAM and WiFi credentials
            if (nvram_clear_all()) {
                Serial.println("[SERIAL] SUCCESS: All credentials cleared!");
                Serial.println("[SERIAL] Device will enter AP mode after reboot.");
                Serial.println("[SERIAL] Please restart the device now.");
                delay(2000);
            } else {
                Serial.println("[SERIAL] ERROR: Failed to clear credentials");
            }
        } else if (command.length() > 0) {
            Serial.print("[SERIAL] Unknown command: ");
            Serial.println(command);
            Serial.println("[SERIAL] Available commands: clear");
        }
    }
}

// ============================================================================
// Main Application Flow
// ============================================================================

void app_main() {
    // Print startup banner
    Serial.println("\n\n================================");
    Serial.println("TRMNL T5S3 Pro - Main Firmware");
    Serial.println("================================");

#if (TEST_DOWNLOAD_ERROR || TEST_DECODE_ERROR || TEST_WIFI_ERROR)
    Serial.print("\n[TEST] Boot count: ");
    Serial.println(test_boot_count);
    if (test_boot_count == 0) {
        Serial.println("[TEST] First boot - normal operation, error will trigger on next boot");
    } else {
        Serial.println("[TEST] Second+ boot - error injection will activate");
    }
#endif

    delay(1000);

    // Phase 5: Resume button polling task after waking from sleep
    // The task was suspended before entering deep sleep to allow EXT0 wakeup
    button_resume();

    // Initialize SPIFFS (filesystem) - MUST happen before logging
    Serial.println("[SPIFFS] Initializing filesystem...");
    if (!SPIFFS.begin(true)) {
        Serial.println("[SPIFFS] Warning: SPIFFS initialization reported error");
        Serial.println("[SPIFFS] Continuing anyway...");
    }
    Serial.println("[SPIFFS] Filesystem ready");

    // Phase 5: Initialize battery monitoring BEFORE logging
    // This ensures battery data is available for first log entry
    if (!power_init()) {
        Serial.println("[POWER] WARNING: Battery initialization failed");
    }

    // Phase 6: Initialize logging system (after SPIFFS and power are ready)
    Serial.println("[LOGGING] Initializing logging system...");
    if (!logging_init()) {
        Serial.println("[LOGGING] WARNING: Logging initialization failed!");
    }

    // Phase 6: Initialize SD card logging (optional, graceful degradation)
    Serial.println("[SD_LOG] Initializing SD card logging...");
    sd_logging_init();  // No error check - device works without SD card

    // Phase 7: Initialize OTA firmware update subsystem
    if (ota_init()) {
        Serial.print("[OTA] Current firmware version: ");
        Serial.println(ota_get_version());
    }

    // Phase 5: Check wakeup reason using button module
    bool button_wake = button_was_pressed();
    if (button_wake) {
        Serial.println("[WAKE] Button press detected (GPIO48)");
        logging_write(LOG_INFO, "Device woke from button press");
    } else {
        Serial.println("[WAKE] Timer wakeup - scheduled update");
        logging_write(LOG_INFO, "Device woke from scheduled timer");
    }

    // Initialize NVRAM (Non-Volatile Storage for credentials)
    // Phase 2: Persistent storage of API keys and device IDs
    if (NVRAM_ENABLE) {
        nvram_init();
    } else {
        Serial.println("[NVRAM] Disabled - using hardcoded fallbacks only");
    }

    // Phase 2c/3: Listen for serial commands (after NVRAM is initialized)
    // Allow user to clear credentials via serial during startup window
    Serial.println("[SETUP] Listening for serial commands...");
    Serial.println("[SETUP] Type 'clear' to erase credentials and enter AP mode");
    Serial.println("[SETUP] Starting main application in 3 seconds...\n");

    for (int i = 3; i > 0; i--) {
        handle_serial_commands();
        delay(1000);
    }

    // Initialize display (I2C, VCOM, state)
    // CRITICAL: This must happen before any display operations
    display_init();

    // Phase 3: Load WiFi credentials from NVS
    // Try to use saved credentials if available, otherwise use hardcoded
    char wifi_ssid[33];
    char wifi_password[65];
    bool has_saved_ssid = nvram_read_wifi_ssid(wifi_ssid, sizeof(wifi_ssid));
    bool has_saved_password = nvram_read_wifi_password(wifi_password, sizeof(wifi_password));

    // Connect to WiFi
    Serial.println("[WIFI] Initializing WiFi...");

    // If we have saved credentials, try them; otherwise use hardcoded
    bool wifi_connected = false;
    if (has_saved_ssid) {
        Serial.print("[WIFI] Using saved WiFi SSID: ");
        Serial.println(wifi_ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifi_ssid, has_saved_password ? wifi_password : "");

        // Wait for connection with timeout
        int retry = 0;
        Serial.println("[WIFI] Starting connection...");
        while (WiFi.status() != WL_CONNECTED && retry < WIFI_CONNECT_TIMEOUT) {
            delay(1000);
            retry++;
            Serial.print(".");
        }

        wifi_connected = (WiFi.status() == WL_CONNECTED);
        if (!wifi_connected) {
            Serial.println("\n[WIFI] Failed to connect with saved credentials!");
            logging_write(LOG_ERROR, "WiFi connection failed with saved credentials");

            // Increment WiFi failure counter
            uint8_t fail_count = nvram_increment_wifi_failure_count();

            // Display WiFi error icon (no text, preserves previous display)
            display_wifi_error_icon();

            // Check if we've exceeded maximum failures
            if (fail_count >= 3) {
                Serial.println("[WIFI] Maximum failures reached - clearing credentials and entering AP mode");

                // Submit error log to server before resetting
                submit_error_log_to_server();

                display_text("Resetting...");
                delay(2000);

                // Clear WiFi credentials to allow re-entry
                nvram_write_wifi_ssid("");
                nvram_write_wifi_password("");
                nvram_reset_wifi_failure_count();

                delay(1000);
                // Enter AP mode for credential re-entry
                display_ap_mode();
                ap_server_start();  // Blocks until reboot
            }

            delay(5000);
            goto sleep_now;
        } else {
            // Successfully connected - reset failure counter
            nvram_reset_wifi_failure_count();
            logging_write(LOG_INFO, "WiFi connected successfully");
        }
    } else if (!wifi_connect_blocking()) {
        Serial.println("\n[WIFI] Failed to connect with hardcoded credentials!");
        logging_write(LOG_ERROR, "WiFi connection failed with hardcoded credentials");

        // Phase 2c/3: No saved credentials and no hardcoded - enter AP mode for setup
        Serial.println("[WIFI] No WiFi credentials configured - entering AP mode");
        display_ap_mode();
        // Phase 3: Start captive portal server
        // This function blocks forever, handling HTTP requests and DNS spoofing
        // Device will reboot after user submits credentials
        ap_server_start();
        // Should never reach here (ap_server_start blocks until reboot)
    } else {
        wifi_connected = true;
        logging_write(LOG_INFO, "WiFi connected successfully");
    }

    Serial.println("\n[WIFI] Connected!");
    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.localIP());

    // Phase 6: Sync time with NTP servers (for accurate log timestamps)
    {
        Serial.println("[TIME] Synchronizing with NTP servers...");
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // GMT offset 0, daylight 0

        // Wait for time sync (with timeout)
        int ntp_retry = 0;
        while (time(NULL) < 100000 && ntp_retry < 10) {
            delay(500);
            ntp_retry++;
        }

        if (time(NULL) > 100000) {
            time_t now = time(NULL);
            struct tm* timeinfo = localtime(&now);
            Serial.print("[TIME] Time synchronized: ");
            Serial.print(asctime(timeinfo));
        } else {
            Serial.println("[TIME] WARNING: Time sync failed, timestamps may be incorrect");
        }
    }

    // Phase 5: Read battery voltage and memory status
    {
        float battery_voltage = power_get_battery_voltage();
        uint8_t battery_soc = power_get_state_of_charge();
        uint32_t free_heap = power_get_free_heap();
        uint32_t total_heap = power_get_total_heap();
        Serial.print("[POWER] Free heap: ");
        Serial.print(free_heap);
        Serial.print(" / ");
        Serial.print(total_heap);
        Serial.println(" bytes");

        // Log battery warning if low
        if (battery_soc < 20) {
            logging_write(LOG_WARNING, "Low battery detected");
        }
    }

    // Get image URL from API or use fallback
    {
        const char* image_url = NULL;
        const char* image_file = "/spiffs/display.bmp";

        // Phase 4: Auto-registration flow
        // Check if device has been registered (has API key in NVS)
        bool has_api_key = nvram_has_api_key();

        if (!has_api_key) {
            // First time setup - call /api/setup to auto-register device
            Serial.println("[MAIN] No API key found - performing auto-registration...");

            TRMNLSetupResponse setupResponse;
            memset(&setupResponse, 0, sizeof(setupResponse));

            if (trmnlApiSetup(&setupResponse)) {
                Serial.println("[MAIN] Auto-registration successful!");
                logging_write(LOG_INFO, "Device auto-registration successful");

                // Save API key to NVS for next boot
                if (nvram_write_api_key(setupResponse.api_key)) {
                    Serial.print("[MAIN] Saved API key to NVS: ");
                    Serial.println(setupResponse.api_key);
                }

                // Save friendly ID if provided
                if (strlen(setupResponse.friendly_id) > 0) {
                    if (nvram_write_friendly_id(setupResponse.friendly_id)) {
                        Serial.print("[MAIN] Saved friendly ID to NVS: ");
                        Serial.println(setupResponse.friendly_id);
                    }
                }

                // Now that we have the API key, call /api/display to get the real image
                Serial.println("[MAIN] Calling /api/display to get actual image...");
                TRMNLDisplayResponse displayResponse;
                if (trmnlApiDisplay(setupResponse.api_key, &displayResponse)) {
                    image_url = displayResponse.image_url;
                    Serial.println("[MAIN] Got image URL from TRMNL API display endpoint");
                    logging_write(LOG_INFO, "API /display call successful");

                    // Phase 6: Track image filename for logging
                    if (strlen(displayResponse.filename) > 0) {
                        status_set_current_image(displayResponse.filename);
                    }

                    // Phase 4: Save refresh rate from server
                    if (displayResponse.refresh_rate > 0) {
                        nvram_write_refresh_rate((uint16_t)displayResponse.refresh_rate);
                    }

                    // Phase 7: Handle firmware update command
                    if (displayResponse.update_firmware) {
                        Serial.println("[MAIN] Server commanded firmware update!");
                        if (strlen(displayResponse.firmware_url) > 0) {
                            Serial.print("[MAIN] Firmware URL from API: ");
                            Serial.println(displayResponse.firmware_url);
                        }

                        // Check if OTA should proceed (battery level, etc)
                        if (ota_should_update(displayResponse.update_firmware)) {
                            display_text("Updating...");

                            // Perform OTA update (will reboot on success)
                            ota_result_t result = ota_update_from_url(displayResponse.firmware_url);

                            // If we reach here, update failed
                            Serial.print("[MAIN] OTA update failed: ");
                            Serial.println(ota_result_to_string(result));

                            // Submit OTA failure log to server
                            submit_error_log_to_server();

                            char error_msg[64];
                            snprintf(error_msg, sizeof(error_msg), "OTA failed: %s",
                                     ota_result_to_string(result));
                            display_text(error_msg);
                            delay(3000);
                        } else {
                            Serial.println("[MAIN] OTA update skipped (requirements not met)");
                        }
                    }

                    // Phase 4: Handle device reset command
                    if (displayResponse.reset_firmware) {
                        Serial.println("[MAIN] Server commanded device reset!");
                        display_text("Rebooting...");
                        wifi_disconnect_and_poweroff();
                        delay(2000);
                        ESP.restart();  // Soft reboot
                    }
                } else {
                    // Fallback to setup image if display call fails
                    Serial.println("[MAIN] Display call failed, using setup image");
                    logging_write(LOG_WARNING, "API /display call failed after registration");
                    if (strlen(setupResponse.image_url) > 0) {
                        image_url = setupResponse.image_url;
                    }
                }
            } else {
                Serial.println("[MAIN] Auto-registration failed");
                logging_write(LOG_ERROR, "Device auto-registration failed");
            }
        }

        // If we still don't have an image URL, use /api/display with existing API key
        if (!image_url) {
            char api_key[256];
            nvram_read_api_key(api_key, sizeof(api_key));
            Serial.print("[MAIN] Using API key: ");
            Serial.println(api_key);

            // Try TRMNL API display endpoint
            TRMNLDisplayResponse displayResponse;
            if (trmnlApiDisplay(api_key, &displayResponse)) {
                image_url = displayResponse.image_url;
                Serial.println("[MAIN] Got image URL from TRMNL API display endpoint");
                logging_write(LOG_INFO, "API /display call successful");

                // Phase 6: Track image filename for logging
                if (strlen(displayResponse.filename) > 0) {
                    status_set_current_image(displayResponse.filename);
                }

                // Phase 4: Save refresh rate from server
                if (displayResponse.refresh_rate > 0) {
                    nvram_write_refresh_rate((uint16_t)displayResponse.refresh_rate);
                }

                // Phase 7: Handle firmware update command
                if (displayResponse.update_firmware) {
                    Serial.println("[MAIN] Server commanded firmware update!");
                    if (strlen(displayResponse.firmware_url) > 0) {
                        Serial.print("[MAIN] Firmware URL from API: ");
                        Serial.println(displayResponse.firmware_url);
                    }

                    // Check if OTA should proceed (battery level, etc)
                    if (ota_should_update(displayResponse.update_firmware)) {
                        display_text("Updating...");

                        // Perform OTA update (will reboot on success)
                        ota_result_t result = ota_update_from_url(displayResponse.firmware_url);

                        // If we reach here, update failed
                        Serial.print("[MAIN] OTA update failed: ");
                        Serial.println(ota_result_to_string(result));

                        // Submit OTA failure log to server
                        submit_error_log_to_server();

                        char error_msg[64];
                        snprintf(error_msg, sizeof(error_msg), "OTA failed: %s",
                                 ota_result_to_string(result));
                        display_text(error_msg);
                        delay(3000);
                    } else {
                        Serial.println("[MAIN] OTA update skipped (requirements not met)");
                    }
                }

                // Phase 4: Handle device reset command
                if (displayResponse.reset_firmware) {
                    Serial.println("[MAIN] Server commanded device reset!");
                    display_text("Rebooting...");
                    wifi_disconnect_and_poweroff();
                    delay(2000);
                    ESP.restart();  // Soft reboot
                }
            } else {
                Serial.println("[MAIN] TRMNL API display failed or device not registered");
                logging_write(LOG_ERROR, "API /display call failed");

                // Submit API failure log to server
                submit_error_log_to_server();
            }
        }

        // Fallback to test image if API fails
        if (!image_url) {
            image_url = TRMNL_TEST_IMAGE_URL;
            Serial.println("[MAIN] Using fallback image URL");
        }

        // Download the image
        download_result_t download_result = download_image(image_url, image_file);
        if (!download_result.success) {
            Serial.print("[DOWNLOAD] Failed after ");
            Serial.print(download_result.attempts_made);
            Serial.println(" attempts!");
            logging_write(LOG_ERROR, "Image download failed after retries");

            // Submit download failure log to server
            submit_error_log_to_server();

#if TEST_DOWNLOAD_ERROR
            // In test mode, clear display first so error icon is visible
            Serial.println("[TEST] Clearing display before showing error icon...");
            display_clear();
            delay(2000);
#endif

            // Display WiFi error icon (download failures usually WiFi-related)
            display_wifi_error_icon();

#if TEST_DOWNLOAD_ERROR
            Serial.println("[TEST] Download error icon displayed");
            Serial.println("[TEST] Observe e-paper for WiFi-off icon in lower right");
            delay(10000);  // Hold for observation
#endif

            wifi_disconnect_and_poweroff();
            delay(5000);
            goto sleep_now;
        }

        Serial.println("[DOWNLOAD] Success!");
        logging_write(LOG_INFO, "Image downloaded successfully");

        // Display the image (CRITICAL: This is where PNG rendering happens)
        display_image(image_file);
        logging_write(LOG_INFO, "Image displayed on e-paper");
    }

    // ========================================================================
    // TEST INJECTION: WiFi Error (compile-time test flag)
    // ========================================================================
#if TEST_WIFI_ERROR
    // Only trigger error on second boot (after first image displayed)
    if (test_boot_count >= 1) {
        Serial.println("\n[TEST] ===== WIFI ERROR TEST MODE ACTIVE =====");
        Serial.println("[TEST] First cycle completed successfully");
        Serial.println("[TEST] Now forcing WiFi error condition...");

        // Wait to see successful image
        delay(3000);

        // Clear display to white (ensures framebuffer is initialized)
        Serial.println("[TEST] Clearing display to prepare for error icon...");
        display_clear();
        delay(2000);

        // Force WiFi disconnect to simulate connection loss
        WiFi.disconnect(true);
        delay(1000);

        debug_wifi_status();  // Show WiFi status after disconnect

        Serial.println("[TEST] Triggering WiFi error display...");
        // Display WiFi error icon overlay (on white background)
        display_wifi_error_icon();

        Serial.println("[TEST] WiFi error icon displayed");
        Serial.println("[TEST] Observe e-paper display for WiFi-off icon in lower right");

        // Hold for observation
        delay(10000);
    } else {
        Serial.println("\n[TEST] First boot - WiFi error will activate on next wake");
    }
#endif

    // Cleanup before sleep
    wifi_disconnect_and_poweroff();
    Serial.println("[WIFI] Disconnected and powered off");

    // Shutdown timer - wait for configured duration before deep sleep
    // Allows time for any remaining tasks to complete
    {
        unsigned long startup_time = millis();
        unsigned long shutdown_timeout_ms = TRMNL_SHUTDOWN_TIMER_SECONDS * 1000;
        unsigned long elapsed_time = millis() - startup_time;

        if (elapsed_time < shutdown_timeout_ms) {
            unsigned long remaining_time = shutdown_timeout_ms - elapsed_time;
            Serial.print("[SHUTDOWN] Waiting ");
            Serial.print(remaining_time / 1000);
            Serial.println(" seconds before sleep...");
            delay(remaining_time);
        } else {
            Serial.println("[SHUTDOWN] Timeout exceeded, entering sleep immediately");
        }
    }

sleep_now:
    // Phase 4: Load dynamic refresh rate from NVRAM (or use default)
    uint16_t refresh_rate = nvram_get_refresh_rate();

#if (TEST_DOWNLOAD_ERROR || TEST_DECODE_ERROR || TEST_WIFI_ERROR)
    // Override sleep time for faster testing
    refresh_rate = TEST_SLEEP_SECONDS;
    Serial.print("[TEST] Using test sleep time: ");
    Serial.print(refresh_rate);
    Serial.println(" seconds");

    // Increment boot counter for next wake
    test_boot_count++;
    Serial.print("[TEST] Incremented boot count to: ");
    Serial.println(test_boot_count);
#else
    Serial.print("[MAIN] Using refresh rate: ");
    Serial.print(refresh_rate);
    Serial.println(" seconds");
#endif

    // Log entering sleep
    char sleep_msg[64];
    snprintf(sleep_msg, sizeof(sleep_msg), "Entering deep sleep for %d seconds", refresh_rate);
    logging_write(LOG_INFO, sleep_msg);

    // Enter deep sleep (GPIO button wakeup + timer wakeup)
    // This function doesn't return - device resets on wakeup
    enter_deep_sleep(refresh_rate);

    // Should never reach here
    delay(10000);
}

// ============================================================================
// Arduino Framework Entry Points
// ============================================================================

/**
 * @brief Arduino setup() - called once at boot
 */
void setup() {
    Serial.begin(115200);
    // Removed 2-second delay for faster boot

    // Phase 7: Mark OTA update as valid (prevent automatic rollback)
    // This tells the ESP32 bootloader that the current firmware is working
    // Without this, failed OTA updates could cause bootloop
    esp_ota_mark_app_valid_cancel_rollback();
    Serial.println("[OTA] Confirmed valid boot - rollback protection active");

    // Serial command listening has been moved to app_main() after NVRAM is initialized

    // Phase 5: Initialize button detection with polling task
    Serial.println("[SETUP] Initializing button detection...");
    if (!button_init()) {
        Serial.println("[SETUP] WARNING: Button initialization failed!");
    } else {
        // Register button event callback
        button_register_callback(button_event_handler);
    }

    // Phase 6: SD card logging will be initialized in app_main() after SPIFFS
    // Logging system will be initialized in app_main() after SPIFFS is ready
}

/**
 * @brief Arduino loop() - called repeatedly
 * In our case, we call app_main() which enters deep sleep and never returns
 */
void loop() {
    app_main();
    // Should never reach here (enter_deep_sleep doesn't return)
    delay(10000);
}
