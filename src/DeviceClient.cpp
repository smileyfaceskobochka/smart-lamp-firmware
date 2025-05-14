#include "DeviceClient.h"
#include <esp_task_wdt.h>

DeviceClient::DeviceClient(const Network *nets, uint8_t count,
                           const String &deviceId)
    : networks(nets), netCount(count), wsConnected(false), lastReconnect(0),
      lastPing(0) {
  state.id = deviceId;
  state.power = false;
  state.color[0] = state.color[1] = state.color[2] = 0;
  state.brightness = 0;
  state.auto_brightness = false;
  for (int i = 0; i < 4; ++i)
    state.position[i] = 0;
  state.auto_position = false;
  state.distance = 0.0f;
}

void DeviceClient::begin(const char *host, uint16_t port) {
  wsHost = host;
  wsPort = port;

  esp_task_wdt_init(10, true);
  esp_task_wdt_add(nullptr);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  WiFi.onEvent([this](WiFiEvent_t ev, WiFiEventInfo_t) {
    if (ev == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
      Serial.println("[WiFi] Disconnected, reconnecting...");
      WiFi.reconnect();
    } else if (ev == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
      Serial.println("[WiFi] Connected");
    }
  });

  connectWiFi();
  connectWS();
}

void DeviceClient::connectWiFi() {
  static unsigned long lastAttempt = 0;
  const unsigned long retryInterval = 5000;
  const unsigned long timeoutConnect = 10000;

  if (WiFi.status() == WL_CONNECTED)
    return;
  unsigned long now = millis();
  if (now - lastAttempt < retryInterval)
    return;
  lastAttempt = now;

  for (uint8_t i = 0; i < netCount; ++i) {
    Serial.printf("[WiFi] Try `%s`...\n", networks[i].ssid);
    WiFi.disconnect(true);
    delay(50);
    WiFi.begin(networks[i].ssid, networks[i].password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutConnect) {
      ws.poll();
      esp_task_wdt_reset();
      delay(50);
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[WiFi] Connected to `%s`, IP=%s\n", networks[i].ssid,
                    WiFi.localIP().toString().c_str());
      return;
    }
    Serial.println("[WiFi] Failed, next");
  }
  Serial.println("[WiFi] All networks failed; will retry");
}

void DeviceClient::connectWS() {
  if (wsConnected)
    ws.close();

  String uri = String("ws://") + wsHost + ":" + wsPort + "/ws/device";

  ws.onEvent([this](WebsocketsEvent event, String) {
    if (event == WebsocketsEvent::ConnectionOpened) {
      Serial.println("[WS] Opened");
      // ArduinoJson 7: use JsonDocument
      JsonDocument doc;
      JsonObject root = doc.to<JsonObject>();
      root["type"] = "register";
      root["id"] = state.id;
      String out;
      serializeJson(doc, out);
      ws.send(out);
      wsConnected = true;
    } else if (event == WebsocketsEvent::ConnectionClosed) {
      Serial.println("[WS] Closed");
      wsConnected = false;
    } else if (event == WebsocketsEvent::GotPing) {
      Serial.println("[WS] GotPing");
    } else if (event == WebsocketsEvent::GotPong) {
      Serial.println("[WS] GotPong");
    }
  });

  ws.onMessage([this](WebsocketsMessage msg) {
    JsonDocument doc;
    auto err = deserializeJson(doc, msg.data());
    if (err) {
      Serial.println("[WS] JSON parse error");
      return;
    }
    if (doc["type"] == "control") {
      JsonObject s = doc["state"].as<JsonObject>();
      JsonVariantConst cmd = doc["command"];
      processControl(s, cmd);
      if (stateCb)
        stateCb(state);
      if (cmd.is<const char *>() && String((const char *)cmd) == "restart") {
        Serial.println("[CMD] Restarting now...");
        delay(100);
        ESP.restart();
      }
    }
  });

  ws.connect(uri);
  lastReconnect = millis();
}

void DeviceClient::loop() {
  esp_task_wdt_reset();

  connectWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    if (!wsConnected && millis() - lastReconnect > 5000) {
      Serial.println("[Loop] WS reconnect");
      connectWS();
    }
    ws.poll();
    if (millis() - lastPing > 15000) {
      ws.ping();
      lastPing = millis();
    }
  }
}

void DeviceClient::processControl(const JsonObject &s, const JsonVariantConst) {
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
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  root["type"] = "state";
  root["id"] = state.id;
  JsonObject s = root["state"].to<JsonObject>();
  s["power"] = state.power;
  JsonArray col = s["color"].to<JsonArray>();
  for (int i = 0; i < 3; ++i)
    col.add(state.color[i]);
  s["brightness"] = state.brightness;
  s["auto_brightness"] = state.auto_brightness;
  JsonArray pos = s["position"].to<JsonArray>();
  for (int i = 0; i < 4; ++i)
    pos.add(state.position[i]);
  s["auto_position"] = state.auto_position;
  s["distance"] = state.distance;
  String out;
  serializeJson(doc, out);
  ws.send(out);
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
void DeviceClient::setPosition(const uint8_t posArr[4], bool sendNow) {
  for (int i = 0; i < 4; ++i)
    state.position[i] = posArr[i];
  if (sendNow)
    sendState();
}
void DeviceClient::setAutoPosition(bool en, bool sendNow) {
  state.auto_position = en;
  if (sendNow)
    sendState();
}
void DeviceClient::setDistance(float d, bool sendNow) {
  state.distance = d;
  if (sendNow)
    sendState();
}
