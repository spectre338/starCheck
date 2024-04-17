#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include "secrets.h" // Include secrets.h for WiFi credentials

// Neopixel settings
const int LED_PIN = 10;     // Neopixel LED pin
const int NUM_PIXELS = 2;   // Total number of Neopixels

// Neopixel object
Adafruit_NeoPixel statusPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Function prototypes
bool checkDNSHealth();
void connectToWiFi();

void setup() {
    Serial.begin(115200);
    delay(1000); // Wait for serial monitor to initialize
    Serial.println("Initializing...");

    // Initialize Neopixel
    statusPixel.begin();
    statusPixel.clear(); // Clear any existing pixel data
    statusPixel.setBrightness(50); // Set initial brightness (adjust as needed)

    // Attempt initial WiFi connection
    connectToWiFi();
}

void loop() {
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected");
        // WiFi connected, set WiFi status LED (first LED) to green
        statusPixel.setPixelColor(0, statusPixel.Color(0, 255, 0)); // Green color (first LED)
    } else {
        Serial.println("WiFi not connected");
        // WiFi not connected, set WiFi status LED (first LED) to red
        statusPixel.setPixelColor(0, statusPixel.Color(255, 0, 0)); // Red color (first LED)

        // Attempt WiFi reconnection
        connectToWiFi(); // Try to reconnect to WiFi
    }

    // Check DNS health
    bool isDNSHealthy = checkDNSHealth();

    // Set DNS status LED (second LED) based on DNS health
    if (isDNSHealthy) {
        Serial.println("DNS servers reachable");
        // DNS servers reachable, set DNS status LED (second LED) to blue
        statusPixel.setPixelColor(1, statusPixel.Color(0, 0, 255)); // Blue color (second LED)
    } else {
        Serial.println("DNS servers not reachable");
        // DNS servers not reachable, set DNS status LED (second LED) to red
        statusPixel.setPixelColor(1, statusPixel.Color(255, 0, 0)); // Red color (second LED)
    }

    // Show Neopixel updates
    statusPixel.show();

    // Delay before next loop iteration
    delay(5000); // Adjust as needed
}

void connectToWiFi() {
    Serial.println("Attempting WiFi connection...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 5) {
        delay(5000); // Wait 5 seconds before retrying WiFi connection
        attempts++;
        Serial.println("WiFi connection attempt failed. Retrying...");
        WiFi.reconnect(); // Attempt to reconnect to WiFi
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected successfully!");
    } else {
        Serial.println("WiFi connection failed after multiple attempts.");
    }
}

bool checkDNSHealth() {
    int reachableCount = 0;
    // Ping multiple DNS servers and count reachable ones
    if (Ping.ping("8.8.8.8")) {
        reachableCount++;
    }
    if (Ping.ping("1.1.1.1")) {
        reachableCount++;
    }
    // Add more DNS servers as needed
    return (reachableCount >= 2); // Return true if 2 or more DNS servers are reachable
}
