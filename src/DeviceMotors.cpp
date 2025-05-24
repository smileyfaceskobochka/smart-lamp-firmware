#include "DeviceMotors.h"

#define R_SENSE 0.11f
#define RMS_CURRENT 1900
#define MICROSTEPS 16
#define USE_SPREAD true
static const uint32_t STEP_DELAY_US = 250;

DeviceMotors::DeviceMotors(const MotorConfig *configs, uint8_t count,
                           HardwareSerial &uart, uint8_t rxPin, uint8_t txPin)
    : _configs(configs), _count(count), _serial(uart), _rxPin(rxPin),
      _txPin(txPin) {
  drivers = new TMC2209Stepper *[count];
}

void DeviceMotors::begin() {
  // Configure motor pins and enable drivers
  for (uint8_t i = 0; i < _count; ++i) {
    pinMode(_configs[i].enPin, OUTPUT);
    pinMode(_configs[i].dirPin, OUTPUT);
    pinMode(_configs[i].stepPin, OUTPUT);
    digitalWrite(_configs[i].enPin, LOW);
  }
  // Init UART
  _serial.begin(115200, SERIAL_8N1, _rxPin, _txPin);
  for (uint8_t i = 0; i < _count; ++i) {
    drivers[i] = new TMC2209Stepper(&_serial, R_SENSE, _configs[i].address);
    drivers[i]->begin();
    drivers[i]->toff(5);
    drivers[i]->rms_current(RMS_CURRENT);
    drivers[i]->microsteps(MICROSTEPS);
    drivers[i]->en_spreadCycle(USE_SPREAD);
  }
}

void DeviceMotors::move(const int32_t *steps) {
  for (uint8_t i = 0; i < _count; ++i) {
    int32_t cnt = steps[i] * MICROSTEPS;
    if (cnt == 0)
      continue;
    digitalWrite(_configs[i].dirPin, cnt > 0 ? HIGH : LOW);
    for (int32_t s = 0; s < abs(cnt); ++s) {
      digitalWrite(_configs[i].stepPin, HIGH);
      delayMicroseconds(STEP_DELAY_US);
      digitalWrite(_configs[i].stepPin, LOW);
      delayMicroseconds(STEP_DELAY_US);
    }
  }
}