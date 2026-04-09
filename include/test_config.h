/**
 * @file test_config.h
 * @brief Compile-time test configuration for error state testing
 *
 * Purpose: Test error handling paths without waiting for real errors to occur.
 *
 * Usage:
 * 1. Set ONE test flag to 1 (leave others at 0)
 * 2. Compile and flash firmware
 * 3. Device will:
 *    - Complete one successful cycle (connect, download, display)
 *    - Force the specific error condition
 *    - Trigger error handling path
 *    - Display error icon/message
 *
 * IMPORTANT: Set ALL flags to 0 for production builds!
 */

#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

// ============================================================================
// Error State Test Flags (SET ONLY ONE TO 1 AT A TIME)
// ============================================================================

/**
 * Test WiFi error handling
 * Flow:
 * 1. Connect to WiFi successfully
 * 2. Download and display image
 * 3. Force WiFi disconnect
 * 4. Trigger WiFi error path
 * 5. Display WiFi error icon (lower right corner)
 */
#define TEST_WIFI_ERROR 0

/**
 * Test download error handling
 * Flow:
 * 1. Connect to WiFi successfully
 * 2. Download first image successfully
 * 3. Display first image
 * 4. Force download failure on second attempt
 * 5. Trigger retry logic and error display
 */
#define TEST_DOWNLOAD_ERROR 0

/**
 * Test PNG decode error handling
 * Flow:
 * 1. Connect to WiFi successfully
 * 2. Download image successfully
 * 3. Display first image successfully
 * 4. Force PNG decode failure on second image
 * 5. Display decode error icon (lower right corner)
 */
#define TEST_DECODE_ERROR 0

/**
 * Test API error handling
 * Flow:
 * 1. Connect to WiFi successfully
 * 2. Complete first API call successfully
 * 3. Display first image
 * 4. Force API call failure
 * 5. Trigger API error logging and handling
 */
#define TEST_API_ERROR 0

// ============================================================================
// Debug Output Control
// ============================================================================

/**
 * Enable verbose WiFi status debugging
 * Prints human-readable WiFi.status() codes to Serial
 */
#define DEBUG_WIFI_STATUS 0

// ============================================================================
// Test Configuration
// ============================================================================

/**
 * Sleep duration for test modes (seconds)
 * Shortened from default 300s (5 min) to allow faster testing iterations
 */
#define TEST_SLEEP_SECONDS 15

#endif // TEST_CONFIG_H
