/**
 * @file config.h
 * @brief Global configuration macros for TRMNL T5S3 Pro
 *
 * Centralized configuration values used across modules
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Firmware Version
// ============================================================================

// Firmware version string - reported to TRMNL server via FW-Version header
// Update this for each release
#define FIRMWARE_VERSION "1.1.0"

// ============================================================================
// Deep Sleep Configuration
// ============================================================================

// Build mode: set to 1 for development, 0 for production
// Development mode: longer wake time for debugging, shorter sleep intervals
// Production mode: minimal wake time for battery efficiency
#ifndef DEV_MODE
#define DEV_MODE 0  // Set to 0 for production builds
#endif

// Deep sleep interval in seconds (how long device sleeps between updates)
// NOTE: This value is used by the firmware. For testing, set to 10 seconds.
// For production, set to 300 seconds (5 minutes) or higher.
#define TRMNL_DEEP_SLEEP_SECONDS 10

// Shutdown timer in seconds (how long device stays awake before entering deep sleep)
// Development: 60s allows time for serial debugging and observation
// Production: 5s minimizes battery drain while ensuring clean shutdown
#if DEV_MODE
  #define TRMNL_SHUTDOWN_TIMER_SECONDS 60
#else
  #define TRMNL_SHUTDOWN_TIMER_SECONDS 2  // Optimized: reduced from 5s to 2s for battery savings
#endif

// WiFi connection timeout (seconds)
#define WIFI_CONNECT_TIMEOUT 15

// ============================================================================
// NVRAM (Non-Volatile Storage) Configuration
// ============================================================================

// Enable/disable persistent storage of API keys and device IDs
// Set to 1 to enable, 0 to disable (use hardcoded fallbacks only)
// When enabled, device reads credentials from flash on boot
// When disabled, always uses hardcoded values
#define NVRAM_ENABLE 1

// NVS namespace for storing credentials
// Must match official TRMNL firmware for compatibility
#define NVRAM_NAMESPACE "data"

// ============================================================================
// Battery & Power Monitoring Configuration (Phase 5)
// ============================================================================

// Battery ADC pin (GPIO36 on LilyGo T5S3 Pro)
// Connected via 1:1 voltage divider (100K + 100K resistors)
#define BATTERY_ADC_PIN_NUM 36

// Battery voltage thresholds (volts)
#define BATTERY_MIN_VOLTAGE     3.0   // Low battery warning threshold
#define BATTERY_MAX_VOLTAGE     4.2   // Maximum Lithium cell voltage
#define BATTERY_LOW_THRESHOLD   50000 // Low heap memory warning (bytes)

// ============================================================================
// Button Configuration (Phase 5)
// ============================================================================

// NOTE: Button GPIO pin is defined in button.h as BUTTON_WAKEUP_PIN (GPIO48)
// That is the source of truth for the button pin number

// Button hold time thresholds (milliseconds) - for future multi-hold detection
#define BUTTON_HOLD_SHORT       1000  // 1 second - potential future use
#define BUTTON_HOLD_MEDIUM      5000  // 5 seconds - hard reset (future)
#define BUTTON_HOLD_LONG        15000 // 15 seconds - soft reset (future)

// ============================================================================
// OTA Firmware Update Configuration (Phase 7)
// ============================================================================

// Enable/disable OTA updates
// Set to 1 to enable firmware updates, 0 to disable
#define OTA_ENABLE 1

// Official TRMNL firmware repository (fallback/default source)
// This URL points to the latest stable release from TRMNL GitHub
#define OTA_OFFICIAL_FIRMWARE_URL "https://github.com/usetrmnl/trmnl-firmware/releases/latest/download/firmware.bin"

// Custom firmware URL override (for testing/development)
// Leave empty ("") to use API or official firmware
// Set to custom HTTPS URL to override for testing (highest priority)
// Example: "https://your-server.com/test-firmware.bin"
#define OTA_CUSTOM_FIRMWARE_URL ""

// OTA safety and performance settings
#define OTA_MIN_BATTERY_PERCENT 30      // Minimum battery % required for OTA
#define OTA_TIMEOUT_MS          60000   // Download timeout (60 seconds)
#define OTA_BUFFER_SIZE         4096    // Download buffer size (bytes)

// Automatic OTA updates
// When enabled, device will automatically update when API sends update_firmware flag
// When disabled, updates must be triggered manually
#define OTA_AUTO_UPDATE 1

#endif // CONFIG_H
