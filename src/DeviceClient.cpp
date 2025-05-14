#include "DeviceClient.h"
#include <Arduino.h>

DeviceClient::DeviceClient(const Network *nets, uint8_t count,
                           const String &deviceId)
    : networks(nets), netCount(count), lastReconnect(0) {
  state.id = deviceId;
  state.power = false;
  state.color[0] = state.color[1] = state.color[2] = 0;
  state.brightness = 0;
  state.auto_brightness = false;
  for (int i = 0; i < 4; ++i)
    state.position[i] = 0;
  state.auto_position = false;
}

void DeviceClient::begin(const char *host, uint16_t port) {
  wsHost = host;
  wsPort = port;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  connectWiFi();
  connectWS();
}

void DeviceClient::connectWiFi() {
  WiFi.disconnect(true);
  delay(100);

  // Доступные сети
  int n = WiFi.scanNetworks();
  Serial.printf("Scan found %d networks\n", n);
  for (int i = 0; i < n; ++i) {
    Serial.printf("  %s (%d dBm)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }

  for (uint8_t i = 0; i < netCount; ++i) {
    Serial.printf("Attempt SSID `%s`\n", networks[i].ssid);
    WiFi.begin(networks[i].ssid, networks[i].password);
    int res = WiFi.waitForConnectResult();
    Serial.printf(" Result code %d\n", res);
    if (res == WL_CONNECTED) {
      Serial.printf("Connected to `%s`, IP=%s\n", networks[i].ssid,
                    WiFi.localIP().toString().c_str());
      return;
    }
    Serial.println(" Disconnect and next");
    WiFi.disconnect(true);
    delay(500);
  }

  Serial.println("All attempts failed; retry in 5s");
  delay(5000);
  connectWiFi();
}

void DeviceClient::connectWS() {
  Serial.printf("Connecting WS to %s:%u\n", wsHost, wsPort);
  ws.disconnect();
  ws.begin(wsHost, wsPort, "/ws/device");
  ws.onEvent([this](WStype_t t, uint8_t *p, size_t l) {
    Serial.printf("WS event: %d\n", t);
    handleEvent(t, p, l);
  });
  delay(200);
}

void DeviceClient::loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost; reconnecting");
    connectWiFi();
    return;
  }
  if (!ws.isConnected() && millis() - lastReconnect > 5000) {
    Serial.println("WS lost; reconnecting");
    lastReconnect = millis();
    connectWS();
  }
  ws.loop();

  if (millis() - lastPing > 15000) {
    ws.sendPing();
    lastPing = millis();
  }
}

void DeviceClient::handleEvent(WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_PING) {
    Serial.println("Received PING");
    return;
  }
  if (type == WStype_CONNECTED) {
    DynamicJsonDocument doc(128);
    auto obj = doc.to<JsonObject>();
    obj["type"] = "register";
    obj["id"] = state.id;
    String out;
    serializeJson(doc, out);
    Serial.printf("Sending register: %s\n", out.c_str());
    ws.sendTXT(out);
    return;
  }
  if (type == WStype_TEXT) {
    String msg((char *)payload, length);
    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, msg) == DeserializationError::Ok &&
        doc["type"] == "control") {
      processControl(doc["state"].as<JsonObject>());
      if (stateCb)
        stateCb(state);
    }
  }
}

void DeviceClient::processControl(const JsonObject &s) {
  state.power = s["power"];
  for (int i = 0; i < 3; ++i)
    state.color[i] = s["color"][i];
  state.brightness = s["brightness"];
  state.auto_brightness = s["auto_brightness"];
  for (int i = 0; i < 4; ++i)
    state.position[i] = s["position"][i];
  state.auto_position = s["auto_position"];
}

void DeviceClient::onStateUpdated(std::function<void(const State &)> cb) {
  stateCb = cb;
}

const State &DeviceClient::getState() const { return state; }

void DeviceClient::sendState() {
  DynamicJsonDocument doc(256);
  auto root = doc.to<JsonObject>();
  root["type"] = "state";
  root["id"] = state.id;
  JsonObject s = root.createNestedObject("state");
  s["power"] = state.power;
  JsonArray col = s.createNestedArray("color");
  for (int i = 0; i < 3; ++i)
    col.add(state.color[i]);
  s["brightness"] = state.brightness;
  s["auto_brightness"] = state.auto_brightness;
  JsonArray pos = s.createNestedArray("position");
  for (int i = 0; i < 4; ++i)
    pos.add(state.position[i]);
  s["auto_position"] = state.auto_position;
  String out;
  serializeJson(doc, out);
  ws.sendTXT(out);
}

void DeviceClient::setPower(bool on, bool sendNow) {
  state.power = on;
  if (sendNow)
    sendState();
}
void DeviceClient::setBrightness(uint8_t b, bool sendNow) {
  state.brightness = b;
  if (sendNow)
    sendState();
}
void DeviceClient::setColor(uint8_t r, uint8_t g, uint8_t b, bool sendNow) {
  state.color[0] = r;
  state.color[1] = g;
  state.color[2] = b;
  if (sendNow)
    sendState();
}
void DeviceClient::setAutoBrightness(bool en, bool sendNow) {
  state.auto_brightness = en;
  if (sendNow)
    sendState();
}
void DeviceClient::setPosition(const uint8_t pos[4], bool sendNow) {
  for (int i = 0; i < 4; ++i)
    state.position[i] = pos[i];
  if (sendNow)
    sendState();
}
void DeviceClient::setAutoPosition(bool en, bool sendNow) {
  state.auto_position = en;
  if (sendNow)
    sendState();
}
