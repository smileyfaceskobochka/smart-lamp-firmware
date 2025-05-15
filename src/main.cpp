#include "DeviceClient.h"
#include "DeviceLight.h"
#include <Arduino.h>

#define L_LED_PIN 5
#define L_LED_COUNT 32
#define R_LED_PIN 6
#define R_LED_COUNT 32
#define PHOTO_PIN 10

Network knownNets[] = {
    {"HUAWEI-FR71E3", "0123456789"}, {"aRolf", "Chilllll"}, {"SSID", "PASS"}};

DeviceClient lamp(knownNets, sizeof(knownNets) / sizeof(knownNets[0]),
                  "esp32-s3-device");

using Light =
    DeviceLight<L_LED_PIN, L_LED_COUNT, R_LED_PIN, R_LED_COUNT, PHOTO_PIN>;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  Light::begin();
  lamp.begin("meowww.su", 80);
}

void loop() {
  lamp.loop();
  Light::update(lamp.getState());
  delay(100);
}