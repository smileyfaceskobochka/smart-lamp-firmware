#ifndef DEVICE_MOTORS_H
#define DEVICE_MOTORS_H

#include <Arduino.h>
#include <TMCStepper.h>

/**
 * @brief Configuration for a single TMC2209 driver.
 */
struct MotorConfig {
  uint8_t enPin; ///< Enable pin (active LOW). Рус: Пин включения (активен LOW).
  uint8_t dirPin;  ///< Direction pin. Рус: Пин направления.
  uint8_t stepPin; ///< Step pin. Рус: Шаговый пин.
  uint8_t address; ///< UART slave address (0-3). Рус: UART адрес (0-3).
};

/**
 * @class DeviceMotors
 * @brief Manages multiple TMC2209 stepper drivers over a single UART bus.
 */
class DeviceMotors {
public:
  /**
   * @brief Construct the DeviceMotors object.
   * @brief_ru Конструктор класса DeviceMotors.
   * @param configs Array of MotorConfig for each driver.
   * @param count Number of drivers in configs array.
   * @param uart Serial port reference (e.g., Serial1).
   * @param rxPin UART RX pin.
   * @param txPin UART TX pin.
   */
  DeviceMotors(const MotorConfig *configs, uint8_t count, HardwareSerial &uart,
               uint8_t rxPin, uint8_t txPin);

  /**
   * @brief Initialize GPIO pins and UART for all drivers.
   */
  void begin();

  /**
   * @brief Move each motor by specified steps.
   * @brief_ru Двигает каждый мотор на указанное число шагов.
   * @param steps Array of step counts: positive=CW, negative=CCW.
   */
  void move(const int32_t *steps);

private:
  const MotorConfig *_configs; ///< Driver configurations.
  uint8_t _count;              ///< Number of drivers.
  HardwareSerial &_serial;     ///< UART serial instance.
  uint8_t _rxPin, _txPin;      ///< UART pins.
  TMC2209Stepper **drivers;    ///< Array of driver objects.
};

#endif // DEVICE_MOTORS_H