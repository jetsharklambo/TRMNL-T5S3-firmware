/**
 * @file ap_server.h
 * @brief WiFi AP Mode Server & Captive Portal (Phase 3)
 *
 * Provides:
 * - WiFi Access Point (AP) mode broadcasting
 * - DNS server for captive portal redirection
 * - HTTP web server with credential entry form
 * - Credential validation and storage
 *
 * Flow:
 * 1. AP mode activated when WiFi credentials missing
 * 2. Device broadcasts "TRMNL-XXXX" WiFi network
 * 3. User connects to network
 * 4. DNS server redirects all domains to device IP (192.168.4.1)
 * 5. User opens browser, automatically redirected to captive portal
 * 6. User enters WiFi SSID and password
 * 7. Device validates, saves to NVS, and reboots to connect
 */

#ifndef AP_SERVER_H
#define AP_SERVER_H

// ============================================================================
// AP Server Functions
// ============================================================================

/**
 * @brief Start WiFi AP mode with captive portal server
 *
 * Starts:
 * - WiFi in AP mode with SSID "TRMNL-XXXX" (based on MAC address)
 * - DNS server for captive portal redirection
 * - HTTP web server on port 80
 *
 * Device IP: 192.168.4.1
 * Netmask: 255.255.255.0
 *
 * This function does not return - it blocks in a server loop.
 * The HTTP form allows user to enter WiFi credentials.
 *
 * After successful credential entry:
 * - Credentials saved to NVS
 * - Device automatically reboots to connect to user's WiFi
 *
 * @note Must be called after display_ap_mode() to show setup instructions
 */
void ap_server_start();

/**
 * @brief Stop AP mode and DNS/HTTP servers
 *
 * Cleans up resources and stops broadcasting AP network.
 * Typically called before device reboots to connect to user's WiFi.
 */
void ap_server_stop();

#endif // AP_SERVER_H
