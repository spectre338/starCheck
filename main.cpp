#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include "secrets.h"  // Ensure this file contains definitions for WIFI_SSID and WIFI_PASS
#include <ESP32Ping.h>

const int LED_PIN = 10;
const int NUM_PIXELS = 2;
Adafruit_NeoPixel statusPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

bool dnsReachable = false;
unsigned long lastDNSCheck = 0;

// Function prototype
bool checkDNSHealth();

void onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Connected to WiFi!");
            statusPixel.setPixelColor(0, statusPixel.Color(0, 255, 0)); // Green for WiFi connected
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi, attempting to reconnect...");
            WiFi.begin(WIFI_SSID, WIFI_PASS); // Reconnect automatically
            statusPixel.setPixelColor(0, statusPixel.Color(255, 0, 0)); // Red for WiFi disconnected
            break;
    }
    statusPixel.show();
}

void setup() {
    Serial.begin(115200);
    WiFi.onEvent(onWiFiEvent);
    
    statusPixel.begin();
    statusPixel.clear();
    statusPixel.setBrightness(50);
    statusPixel.show();

    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
    if (millis() - lastDNSCheck >= 5000) {
        lastDNSCheck = millis();
        dnsReachable = checkDNSHealth();
        statusPixel.setPixelColor(1, dnsReachable ? statusPixel.Color(0, 0, 255) : statusPixel.Color(255, 0, 0));
        statusPixel.show();
    }
}

// Function definition
bool checkDNSHealth() {
    const char* hostname = "google.com";
    IPAddress resolvedIP;
    if (WiFi.hostByName(hostname, resolvedIP) && Ping.ping("1.1.1.1", 4)) {
        Serial.println("DNS lookup and ping successful");
        return true;
    } else {
        Serial.println("DNS lookup failed or ping unsuccessful");
        return false;
    }
}
