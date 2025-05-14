#include "DeviceClient.h"
#include <Arduino.h>
#include <FastLED.h>

#define L_LED_PIN 5
#define L_LED_COUNT 32

#define R_LED_PIN 6
#define R_LED_COUNT 32

#define PHOTO_PIN 10

Network knownNets[] = {
    {"SSID0", "PASS0"}, {"SSID1", "PASS1"}, {"SSID2", "PASS2"}};

DeviceClient lamp(knownNets, sizeof(knownNets) / sizeof(knownNets[0]),
                  "esp32-s3-device");

CRGB ledsL[L_LED_COUNT];
CRGB ledsR[R_LED_COUNT];

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  FastLED.addLeds<WS2812B, L_LED_PIN, GRB>(ledsL, L_LED_COUNT);
  FastLED.addLeds<WS2812B, R_LED_PIN, GRB>(ledsR, R_LED_COUNT);
  FastLED.setBrightness(0);

  pinMode(PHOTO_PIN, INPUT);

  lamp.onStateUpdated([](const State &s) {
    Serial.printf("State update:\n");
    Serial.printf("  id: %s\n", s.id.c_str());
    Serial.printf("  power: %s\n", s.power ? "ON" : "OFF");
    Serial.printf("  color: [%u, %u, %u]\n", s.color[0], s.color[1],
                  s.color[2]);
    Serial.printf("  brightness: %u\n", s.brightness);
    Serial.printf("  auto_brightness: %s\n", s.auto_brightness ? "ON" : "OFF");
  });

  lamp.begin("meowww.su", 80);
}

void loop() {
  lamp.loop();

  int photoValue = analogRead(PHOTO_PIN); // Read photoresistor
  State state = lamp.getState();

  uint8_t brightness;
  if (state.auto_brightness) {
    brightness = map(photoValue, 0, 4095, 255, 0);
  } else {
    brightness = state.brightness;
  }

  FastLED.setBrightness(brightness);

  if (state.power) {
    CRGB color = CRGB(state.color[0], state.color[1], state.color[2]);

    fill_solid(ledsL, L_LED_COUNT, color);
    fill_solid(ledsR, R_LED_COUNT, color);

    FastLED.show();
  } else {
    // Turn off LEDs
    FastLED.clear();
    FastLED.show();
  }

  delay(100);
}