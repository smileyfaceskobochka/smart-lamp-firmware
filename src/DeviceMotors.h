#ifndef DEVICE_MOTORS_H
#define DEVICE_MOTORS_H

#include <Arduino.h>
#include <TMCStepper.h>

struct MotorConfig {
  uint8_t enPin;
  uint8_t dirPin;
  uint8_t stepPin;
  uint8_t address;
};

class DeviceMotors {
public:
  DeviceMotors(const MotorConfig *configs, uint8_t count, HardwareSerial &uart,
               uint8_t rxPin, uint8_t txPin);

  void begin();

  void move(const int32_t *steps);

private:
  const MotorConfig *_configs;
  uint8_t _count;
  HardwareSerial &_serial;
  uint8_t _rxPin, _txPin;
  TMC2209Stepper **drivers;
};

#endif // DEVICE_MOTORS_H