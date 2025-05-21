#include "DeviceClient.h"
#include "DeviceLight.h"
#include "DeviceMotors.h"
#include <Arduino.h>

// LED and photo-sensor config
#define L_LED_PIN 9
#define L_LED_CNT 32
#define R_LED_PIN 10
#define R_LED_CNT 32
#define PHOTO_PIN 11

// Wi-Fi networks
Network knownNets[] = {
    {"HUAWEI-FR71E3", "0123456789"}, {"aRolf", "Chilllll"}, {"SSID", "PASS"}};

using Light =
    DeviceLight<L_LED_PIN, L_LED_CNT, R_LED_PIN, R_LED_CNT, PHOTO_PIN>;

// Motor configurations moved to main
static const MotorConfig motorConfigs[3] = {
    {4, 5, 6, 0},    // EN, DIR, STEP, address 0
    {12, 13, 14, 1}, // EN, DIR, STEP, address 1
    {19, 20, 21, 2}  // EN, DIR, STEP, address 2
};

// UART pins for PDN_UART
static const uint8_t UART_RX = 18;
static const uint8_t UART_TX = 17;

DeviceClient lamp(knownNets, sizeof(knownNets) / sizeof(knownNets[0]),
                  "esp32-s3-device");
DeviceMotors motors(motorConfigs, 3, Serial1, UART_RX, UART_TX);

int32_t zeros[3] = {0, 0, 0}; // Array to hold zero position 


void setup() {
  Serial.begin(115200);
  Serial.println("Starting.");

  Light::begin();
  lamp.begin("192.168.3.4", 80);

  motors.begin(); // Initialize stepper drivers
}

void loop() {
  lamp.loop();
  const State &st = lamp.getState();

  Serial.printf("Power: %d, Color: (%d, %d, %d), Brightness: %d, Auto "
                "Brightness: %d, pos: (%d, %d, %d)\n",
                st.power, st.color[0], st.color[1], st.color[2], st.brightness,
                st.auto_brightness, st.position[0], st.position[1],
                st.position[2]);

  Light::update(st);
  motors.move(st.position);
  lamp.setPosition(zeros, false); // Reset position after moving

  delay(100);
}
