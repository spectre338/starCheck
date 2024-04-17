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

volatile bool dnsReachable = true;
unsigned long lastDNSCheck = 0;
unsigned long pingFailStartTime = 0;
bool pingCurrentlyFailing = false;

TaskHandle_t NetworkTaskHandle = NULL;

// Function prototype for the WiFi event handler
void onWiFiEvent(WiFiEvent_t event);

void NetworkTask(void *parameter) {
    for (;;) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi is disconnected, DNS check is not possible.");
            dnsReachable = false;  // Explicitly set DNS as unreachable
            pingCurrentlyFailing = true;  // Assume failing until proven otherwise
            pingFailStartTime = millis();  // Start tracking failure time
            WiFi.begin(WIFI_SSID, WIFI_PASS);
        } else {
            // Only attempt to ping if WiFi is connected
            dnsReachable = Ping.ping("1.1.1.1", 4);
            if (!dnsReachable) {
                Serial.println("Ping failed!");
                if (!pingCurrentlyFailing) {
                    pingFailStartTime = millis();
                    pingCurrentlyFailing = true;
                } else if (millis() - pingFailStartTime > 60000) {
                    Serial.println("Ping failure exceeded 60 seconds, rebooting...");
                    ESP.restart();
                }
            } else {
                pingCurrentlyFailing = false;  // Reset on successful ping
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds
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

    xTaskCreatePinnedToCore(NetworkTask, "Network Task", 10000, NULL, 1, &NetworkTaskHandle, 0);
}

void loop() {
    static float phase = 0.0;
    float brightness = (sin(phase) + 1.0) / 2.0 * 255; // Normalize and scale
    phase += 0.005; // Slowing down the pulse by reducing the phase increment
    if (phase > 2 * PI) {
        phase -= 2 * PI;
    }

    uint32_t wifiColor = WiFi.status() == WL_CONNECTED ? statusPixel.Color(0, brightness, 0) : statusPixel.Color(brightness, 0, 0);
    uint32_t dnsColor = dnsReachable ? statusPixel.Color(0, 0, brightness) : statusPixel.Color(brightness, 0, 0);
    
    statusPixel.setPixelColor(0, wifiColor);
    statusPixel.setPixelColor(1, dnsColor);
    statusPixel.show();

    delay(20);
}

void onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Connected to WiFi!");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi.");
            dnsReachable = false;  // Update DNS status on WiFi disconnect
            break;
    }
}
