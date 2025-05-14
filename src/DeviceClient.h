#ifndef DEVICE_CLIENT_H
#define DEVICE_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <functional>

// Описание Wi-Fi сети
struct Network {
  const char* ssid;
  const char* password;
};

// Состояние устройства
struct State {
  String id;
  bool power;
  uint8_t color[3];
  uint8_t brightness;
  bool auto_brightness;
  uint8_t position[4];
  bool auto_position;
};

// WebSocket-клиент на ESP32
class DeviceClient {
public:
  DeviceClient(const Network* nets, uint8_t count, const String& deviceId);

  // Инициализация: Wi-Fi + WebSocket
  void begin(const char* host, uint16_t port);

  // Вызывать в loop()
  void loop();

  // Отправить текущее состояние на сервер
  void sendState();

  // Установить колбэк на изменение состояния (при control-сообщении)
  void onStateUpdated(std::function<void(const State&)> cb);

  // Получить текущее локальное состояние
  const State& getState() const;

  // Сеттеры для изменения state и (по умолчанию) немедленной отправки
  void setPower(bool on, bool sendNow = true);
  void setBrightness(uint8_t b, bool sendNow = true);
  void setColor(uint8_t r, uint8_t g, uint8_t b, bool sendNow = true);
  void setAutoBrightness(bool en, bool sendNow = true);
  void setPosition(const uint8_t pos[4], bool sendNow = true);
  void setAutoPosition(bool en, bool sendNow = true);

private:
  const Network* networks;
  uint8_t netCount;
  WebSocketsClient ws;
  State state;
  const char* wsHost;
  uint16_t wsPort;
  unsigned long lastReconnect;
  unsigned long lastPing = 0;
  std::function<void(const State&)> stateCb;

  void connectWiFi();
  void connectWS();
  void handleEvent(WStype_t type, uint8_t* payload, size_t length);
  void processControl(const JsonObject& s);
};

#endif // DEVICE_CLIENT_H
