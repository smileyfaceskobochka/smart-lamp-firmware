#ifndef DEVICE_CLIENT_H
#define DEVICE_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <esp_system.h>

using namespace websockets;

struct Network {
  const char *ssid;
  const char *password;
};

struct State {
  String id;
  bool power;
  uint8_t color[3];
  uint8_t brightness;
  bool auto_brightness;
  uint8_t position[4];
  bool auto_position;
  float distance;
};

class DeviceClient {
public:
  DeviceClient(const Network *nets, uint8_t count, const String &deviceId);

  void begin(const char *host, uint16_t port);
  void loop();
  void sendState();
  void onStateUpdated(std::function<void(const State &)> cb);
  const State &getState() const;

  void setPower(bool on, bool sendNow = true);
  void setBrightness(uint8_t b, bool sendNow = true);
  void setColor(uint8_t r, uint8_t g, uint8_t b, bool sendNow = true);
  void setAutoBrightness(bool en, bool sendNow = true);
  void setPosition(const uint8_t pos[4], bool sendNow = true);
  void setAutoPosition(bool en, bool sendNow = true);
  void setDistance(float d, bool sendNow = true);

private:
  const Network *networks;
  uint8_t netCount;
  WebsocketsClient ws;
  State state;
  const char *wsHost;
  uint16_t wsPort;
  bool wsConnected;
  unsigned long lastReconnect;
  unsigned long lastPing;
  std::function<void(const State &)> stateCb;

  void connectWiFi();
  void connectWS();
  void processControl(const JsonObject &s, const JsonVariantConst cmd);
};

#endif // DEVICE_CLIENT_H