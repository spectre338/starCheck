#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

// WiFi settings
const char* WIFI_SSID = "ZTI";
const char* WIFI_PASS = "Zai123!!Zai123!!";
const char* PING_HOST = "192.168.1.1"; // Replace with your WAP IP address or a local server

// Neopixel settings
const int LED_PIN = 10;
const int NUM_PIXELS = 1;

// Neopixel object
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Function prototypes
void connectToWiFi();
void breatheEffect(uint32_t color);
bool isWiFiConnected();

void setup() {
    Serial.begin(115200);
    delay(1000); // Wait for serial monitor to initialize

    // Initialize Neopixel
    pixels.begin();
    pixels.clear(); // Clear any existing pixel data
    pixels.setBrightness(50); // Set initial brightness (adjust as needed)

    // Attempt WiFi connection
    connectToWiFi();
}

void loop() {
    // Check WiFi status every 5 seconds
    delay(5000);

    if (isWiFiConnected()) {
        // WiFi is still connected, breathe green
        breatheEffect(pixels.Color(0, 255, 0));
    } else {
        // WiFi is not connected, clear any existing pixel data
        pixels.clear();
        pixels.setBrightness(50); // Set brightness for solid color display
        pixels.fill(pixels.Color(255, 0, 0)); // Solid red color
        pixels.show();
        
        Serial.println("WiFi connection lost or not established.");

        // Attempt to reconnect
        connectToWiFi();
    }
}


void connectToWiFi() {
    Serial.println("Connecting to WiFi...");

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait for WiFi connection or timeout after 10 seconds
    unsigned long startAttemptTime = millis();
    while (!isWiFiConnected() && millis() - startAttemptTime < 10000) { // Try for up to 10 seconds
        delay(500); // Check every 0.5 seconds
        Serial.print(".");
    }

    if (isWiFiConnected()) {
        Serial.println("\nConnected to WiFi!");
        // WiFi connection successful, breathe green
        breatheEffect(pixels.Color(0, 255, 0));
    } else {
        Serial.println("\nFailed to connect to WiFi!");
        // WiFi connection failed, display solid red
        pixels.fill(pixels.Color(255, 0, 0));
        pixels.show();
    }
}

void breatheEffect(uint32_t color) {
    // Perform breathing effect
    for (int brightness = 0; brightness <= 255; brightness++) { // Increase brightness
        pixels.setBrightness(brightness);
        pixels.fill(color);
        pixels.show();
        delay(10); // Adjust breathing speed (lower value = faster)
    }
    for (int brightness = 255; brightness >= 0; brightness--) { // Decrease brightness
        pixels.setBrightness(brightness);
        pixels.fill(color);
        pixels.show();
        delay(10); // Adjust breathing speed (lower value = faster)
    }
}

bool isWiFiConnected() {
    return (WiFi.status() == WL_CONNECTED && WiFi.RSSI() != 0); // Check both connection status and signal strength
}
