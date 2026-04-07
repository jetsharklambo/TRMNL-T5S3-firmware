/**
 * @file ap_server.cpp
 * @brief WiFi AP Mode Server & Captive Portal Implementation (Phase 3)
 *
 * Implements:
 * - WiFi Access Point (AP) mode broadcasting
 * - Simple DNS server for captive portal (All queries → device IP)
 * - HTTP web server with HTML credential entry form
 * - Form validation and credential storage to NVS
 *
 * Architecture:
 * - WiFi: SoftAP on 192.168.4.1
 * - DNS: UDP listener on port 53 (responds with device IP to all queries)
 * - HTTP: WebServer on port 80 (serves form, handles POST)
 */

#include "ap_server.h"
#include "nvram_config.h"
#include "display.h"
#include <WiFi.h>
#include <WebServer.h>
#include <AsyncUDP.h>
#include <Arduino.h>

// ============================================================================
// Configuration
// ============================================================================

#define AP_SSID_PREFIX "TRMNL-"
#define AP_IP IPAddress(192, 168, 4, 1)
#define AP_GATEWAY IPAddress(192, 168, 4, 1)
#define AP_NETMASK IPAddress(255, 255, 255, 0)
#define HTTP_PORT 80
#define DNS_PORT 53

// ============================================================================
// Global Server Objects
// ============================================================================

WebServer* web_server = nullptr;
AsyncUDP dns_server;

// ============================================================================
// HTML Captive Portal Form
// ============================================================================

const char* getPortalHTML() {
    return R"HTML(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>TRMNL Setup</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 12px;
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.2);
            max-width: 400px;
            width: 100%;
            padding: 40px;
        }
        h1 {
            color: #333;
            margin-bottom: 10px;
            font-size: 28px;
        }
        .subtitle {
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            color: #333;
            font-weight: 500;
            margin-bottom: 8px;
            font-size: 14px;
        }
        input {
            width: 100%;
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            font-size: 14px;
            transition: border-color 0.3s;
            font-family: inherit;
        }
        input:focus {
            outline: none;
            border-color: #667eea;
        }
        .password-toggle {
            margin-top: 8px;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .password-toggle input[type="checkbox"] {
            width: auto;
            cursor: pointer;
        }
        .password-toggle label {
            margin: 0;
            cursor: pointer;
            font-weight: normal;
        }
        button {
            width: 100%;
            padding: 12px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            margin-top: 20px;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4);
        }
        button:active {
            transform: translateY(0);
        }
        .info {
            background: #f0f4ff;
            border-left: 4px solid #667eea;
            padding: 12px;
            border-radius: 4px;
            margin-bottom: 20px;
            font-size: 13px;
            color: #333;
        }
        .error {
            color: #d32f2f;
            font-size: 13px;
            margin-top: 5px;
            display: none;
        }
        .loading {
            display: none;
            text-align: center;
            color: #667eea;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>TRMNL Setup</h1>
        <p class="subtitle">Configure WiFi Credentials</p>

        <form id="setupForm">
            <div class="info">
                📡 Enter your WiFi network name and password. The device will connect and reboot.
            </div>

            <div class="form-group">
                <label for="ssid">WiFi Network Name (SSID)</label>
                <input type="text" id="ssid" name="ssid" placeholder="e.g., MyWiFiNetwork"
                       maxlength="32" required autocomplete="off">
                <div class="error" id="ssidError"></div>
            </div>

            <div class="form-group">
                <label for="password">WiFi Password</label>
                <input type="password" id="password" name="password" placeholder="WiFi password"
                       maxlength="64" autocomplete="off">
                <div class="password-toggle">
                    <input type="checkbox" id="togglePassword" onchange="togglePasswordVisibility()">
                    <label for="togglePassword">Show password</label>
                </div>
                <div class="error" id="passwordError"></div>
            </div>

            <button type="submit">Save & Connect</button>

            <div class="loading" id="loading">
                ⏳ Saving credentials and rebooting...
            </div>
        </form>
    </div>

    <script>
        function togglePasswordVisibility() {
            const pwdInput = document.getElementById('password');
            const toggleCheckbox = document.getElementById('togglePassword');
            pwdInput.type = toggleCheckbox.checked ? 'text' : 'password';
        }

        document.getElementById('setupForm').addEventListener('submit', async (e) => {
            e.preventDefault();

            // Hide previous errors
            document.getElementById('ssidError').style.display = 'none';
            document.getElementById('passwordError').style.display = 'none';

            const ssid = document.getElementById('ssid').value.trim();
            const password = document.getElementById('password').value;

            // Validation
            if (!ssid) {
                document.getElementById('ssidError').textContent = 'WiFi network name is required';
                document.getElementById('ssidError').style.display = 'block';
                return;
            }

            if (ssid.length > 32) {
                document.getElementById('ssidError').textContent = 'WiFi network name too long (max 32 chars)';
                document.getElementById('ssidError').style.display = 'block';
                return;
            }

            if (password.length > 64) {
                document.getElementById('passwordError').textContent = 'Password too long (max 64 chars)';
                document.getElementById('passwordError').style.display = 'block';
                return;
            }

            // Show loading state
            document.getElementById('loading').style.display = 'block';
            document.querySelector('button').disabled = true;

            try {
                const response = await fetch('/api/setup', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ ssid: ssid, password: password })
                });

                if (response.ok) {
                    document.getElementById('loading').textContent =
                        '✓ Credentials saved! Device rebooting in 2 seconds...';
                } else {
                    document.getElementById('passwordError').textContent =
                        'Failed to save credentials. Please try again.';
                    document.getElementById('passwordError').style.display = 'block';
                    document.getElementById('loading').style.display = 'none';
                    document.querySelector('button').disabled = false;
                }
            } catch (error) {
                document.getElementById('passwordError').textContent =
                    'Network error. Please try again.';
                document.getElementById('passwordError').style.display = 'block';
                document.getElementById('loading').style.display = 'none';
                document.querySelector('button').disabled = false;
            }
        });
    </script>
</body>
</html>
)HTML";
}

// ============================================================================
// DNS Server Implementation
// ============================================================================

/**
 * @brief DNS packet handler - responds to all DNS queries with device IP
 *
 * Implements simple DNS spoofing for captive portal:
 * - All DNS A record queries return device IP (192.168.4.1)
 * - User's browser/OS detects and shows captive portal
 */
void handleDNSRequest(AsyncUDPPacket packet) {
    // Simple DNS response: just echo back with answer pointing to device IP
    uint8_t* data = packet.data();
    size_t len = packet.length();

    // DNS query minimum is 12 bytes header + question
    if (len < 12) return;

    // Check if this is a query (bit 7 of byte 2 should be 0)
    if ((data[2] & 0x80) != 0) return;  // It's a response, ignore

    // Simple approach: build response with transaction ID from query
    // We'll just respond with device IP for any A record query
    uint8_t response[512];
    memcpy(response, data, len);

    // Set response flags: QR=1 (response), AA=1 (authoritative), RD=1
    response[2] = 0x84;
    response[3] = 0x00;

    // Set question count and answer count
    response[6] = 0;
    response[7] = 1;  // 1 answer

    // Simple answer section: point to device IP
    // For simplicity, we just append a minimal answer RR
    size_t pos = len;

    // Name pointer to question (0xc00c = pointer to offset 12)
    response[pos++] = 0xc0;
    response[pos++] = 0x0c;

    // Type A (0x0001)
    response[pos++] = 0x00;
    response[pos++] = 0x01;

    // Class IN (0x0001)
    response[pos++] = 0x00;
    response[pos++] = 0x01;

    // TTL (32-bit, use 60 seconds = 0x0000003c)
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x3c;

    // RDLENGTH (4 bytes for IPv4)
    response[pos++] = 0x00;
    response[pos++] = 0x04;

    // RDATA - device IP (192.168.4.1)
    response[pos++] = 192;
    response[pos++] = 168;
    response[pos++] = 4;
    response[pos++] = 1;

    // Send response
    packet.write(response, pos);
}

// ============================================================================
// HTTP Server Routes
// ============================================================================

/**
 * @brief Handle root path - serve captive portal HTML form
 */
void handleRoot() {
    web_server->send(200, "text/html", getPortalHTML());
}

/**
 * @brief Handle redirects to captive portal
 *
 * Redirects common captive portal detection URLs to root
 */
void handleRedirect() {
    web_server->sendHeader("Location", "http://192.168.4.1/", true);
    web_server->send(302, "text/plain", "");
}

/**
 * @brief Handle API setup POST request
 *
 * Receives JSON with SSID and password:
 * { "ssid": "network_name", "password": "wifi_password" }
 *
 * Validates, saves to NVS, and queues device reboot.
 */
void handleAPISetup() {
    if (web_server->method() != HTTP_POST) {
        web_server->send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }

    String body = web_server->arg("plain");
    Serial.print("[AP_SERVER] Received setup request: ");
    Serial.println(body);

    // Parse JSON
    // For simplicity, we'll do basic JSON parsing instead of ArduinoJson
    // (to keep dependencies minimal)

    // Find SSID in JSON: "ssid":"value"
    int ssidStart = body.indexOf("\"ssid\":\"");
    if (ssidStart == -1) {
        web_server->send(400, "application/json", "{\"error\":\"Missing SSID\"}");
        return;
    }
    ssidStart += 8;  // Move past "ssid":"
    int ssidEnd = body.indexOf("\"", ssidStart);
    if (ssidEnd == -1) {
        web_server->send(400, "application/json", "{\"error\":\"Malformed SSID\"}");
        return;
    }
    String ssid = body.substring(ssidStart, ssidEnd);

    // Find password in JSON: "password":"value"
    int pwStart = body.indexOf("\"password\":\"");
    int pwEnd = body.length();
    String password = "";
    if (pwStart != -1) {
        pwStart += 12;  // Move past "password":"
        pwEnd = body.indexOf("\"", pwStart);
        if (pwEnd != -1) {
            password = body.substring(pwStart, pwEnd);
        }
    }

    Serial.print("[AP_SERVER] SSID: ");
    Serial.println(ssid);
    Serial.print("[AP_SERVER] Password length: ");
    Serial.println(password.length());

    // Validate SSID
    if (ssid.length() == 0 || ssid.length() > 32) {
        web_server->send(400, "application/json", "{\"error\":\"Invalid SSID length\"}");
        return;
    }

    // Validate password
    if (password.length() > 64) {
        web_server->send(400, "application/json", "{\"error\":\"Invalid password length\"}");
        return;
    }

    // Save to NVS
    if (!nvram_write_wifi_ssid(ssid.c_str())) {
        web_server->send(500, "application/json", "{\"error\":\"Failed to save SSID\"}");
        return;
    }

    if (password.length() > 0) {
        if (!nvram_write_wifi_password(password.c_str())) {
            web_server->send(500, "application/json", "{\"error\":\"Failed to save password\"}");
            return;
        }
    }

    Serial.println("[AP_SERVER] Credentials saved successfully!");
    web_server->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Credentials saved. Rebooting...\"}");

    // Reboot after a short delay (gives response time to reach client)
    Serial.println("[AP_SERVER] Rebooting in 2 seconds...");
    delay(2000);
    ESP.restart();
}

// ============================================================================
// AP Server Public Functions
// ============================================================================

void ap_server_start() {
    Serial.println("[AP_SERVER] Starting AP mode server...");

    // Get device MAC address for SSID
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char ap_ssid[20];
    snprintf(ap_ssid, sizeof(ap_ssid), "%s%02X%02X", AP_SSID_PREFIX, mac[4], mac[5]);

    Serial.print("[AP_SERVER] AP SSID: ");
    Serial.println(ap_ssid);

    // Start WiFi in AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_NETMASK);
    WiFi.softAP(ap_ssid, "");  // No password for easy access

    Serial.print("[AP_SERVER] AP IP: ");
    Serial.println(WiFi.softAPIP());

    // Start DNS server on port 53
    Serial.println("[AP_SERVER] Starting DNS server...");
    if (dns_server.listen(DNS_PORT)) {
        dns_server.onPacket(handleDNSRequest);
        Serial.print("[AP_SERVER] DNS server listening on port ");
        Serial.println(DNS_PORT);
    } else {
        Serial.println("[AP_SERVER] ERROR: Failed to start DNS server");
    }

    // Start HTTP web server
    Serial.println("[AP_SERVER] Starting HTTP web server...");
    web_server = new WebServer(HTTP_PORT);

    // Register routes
    web_server->on("/", HTTP_GET, handleRoot);
    web_server->on("/api/setup", HTTP_POST, handleAPISetup);

    // Captive portal detection URLs - redirect to root
    web_server->on("/generate_204", HTTP_GET, handleRedirect);
    web_server->on("/fwlink", HTTP_GET, handleRedirect);
    web_server->on("/hotspot-detect.html", HTTP_GET, handleRedirect);
    web_server->on("/canonical.html", HTTP_GET, handleRedirect);

    // Catch-all: redirect to root
    web_server->onNotFound(handleRedirect);

    web_server->begin();
    Serial.print("[AP_SERVER] HTTP server listening on port ");
    Serial.println(HTTP_PORT);

    Serial.println("[AP_SERVER] ========================================");
    Serial.println("[AP_SERVER] Captive Portal Server Ready!");
    Serial.println("[AP_SERVER] ");
    Serial.print("[AP_SERVER] 1. Connect to WiFi: ");
    Serial.println(ap_ssid);
    Serial.println("[AP_SERVER] 2. Open web browser (auto-redirect or visit 192.168.4.1)");
    Serial.println("[AP_SERVER] 3. Enter WiFi credentials");
    Serial.println("[AP_SERVER] 4. Device will save and reboot");
    Serial.println("[AP_SERVER] ========================================");

    // Main server loop - handle incoming requests
    // This function blocks forever until device reboots
    while (true) {
        web_server->handleClient();
        delay(10);
    }
}

void ap_server_stop() {
    Serial.println("[AP_SERVER] Stopping servers...");

    if (web_server) {
        web_server->stop();
        delete web_server;
        web_server = nullptr;
    }

    dns_server.close();
    WiFi.softAPdisconnect(true);

    Serial.println("[AP_SERVER] Servers stopped");
}
