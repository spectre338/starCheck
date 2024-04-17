#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include "secrets.h"
#include <ESP32Ping.h>
#include <math.h>

const int LED_PIN = 10;
const int NUM_PIXELS = 2;
const int BUZZER_PIN = 7;
Adafruit_NeoPixel statusPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

bool dnsReachable = true;
unsigned long lastDNSCheck = 0;
int brightness = 0;
int fadeAmount = 5;  // Amount by which the brightness will change
int failureCount = 0;  // Counter for consecutive DNS failures

bool checkDNSHealth();

void onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Connected to WiFi!");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi, attempting to reconnect...");
            WiFi.begin(WIFI_SSID, WIFI_PASS);
            break;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    WiFi.onEvent(onWiFiEvent);
    
    statusPixel.begin();
    statusPixel.clear();
    statusPixel.show();

    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastDNSCheck >= 5000) {
        lastDNSCheck = currentMillis;
        if (checkDNSHealth()) {
            failureCount = 0;  // Reset the failure counter on success
            dnsReachable = true;
        } else {
            failureCount++;  // Increment the failure counter on failure
            if (failureCount >= 10) {
                dnsReachable = false;
                digitalWrite(BUZZER_PIN, HIGH);  // Turn on the buzzer after 10 failures
            }
        }
    }

    // Update the brightness for the pulse effect
    brightness += fadeAmount;
    if (brightness <= 0 || brightness >= 255) {
        fadeAmount = -fadeAmount;
        brightness = max(0, min(255, brightness));
    }

    uint32_t wifiColor = WiFi.status() == WL_CONNECTED ? statusPixel.Color(0, brightness, 0) : statusPixel.Color(brightness, 0, 0);
    uint32_t dnsColor = dnsReachable ? statusPixel.Color(0, 0, brightness) : statusPixel.Color(brightness, 0, 0);
    
    statusPixel.setPixelColor(0, wifiColor);
    statusPixel.setPixelColor(1, dnsColor);
    statusPixel.show();

    delay(50);  // Delay to slow down the fading effect
}

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
