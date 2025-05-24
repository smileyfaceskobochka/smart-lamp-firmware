#ifndef DEVICELIGHT_H
#define DEVICELIGHT_H

#include "DeviceClient.h"
#include <FastLED.h>

template <uint8_t LEFT_PIN, uint16_t LEFT_COUNT, uint8_t RIGHT_PIN,
          uint16_t RIGHT_COUNT, uint8_t PHOTO_PIN>
class DeviceLight {
public:
  static void begin() {
    FastLED.addLeds<WS2812B, LEFT_PIN, GRB>(ledsL, LEFT_COUNT);
    FastLED.addLeds<WS2812B, RIGHT_PIN, GRB>(ledsR, RIGHT_COUNT);
    FastLED.setBrightness(0);
    pinMode(PHOTO_PIN, INPUT);
  }

  static void update(const State &state) {
    int photoValue = analogRead(PHOTO_PIN);
    uint8_t brightness = state.auto_brightness
                             ? map(photoValue, 0, 4095, 255, 0)
                             : state.brightness;

    FastLED.setBrightness(brightness);

    if (state.power) {
      CRGB color(state.color[0], state.color[1], state.color[2]);
      fill_solid(ledsL, LEFT_COUNT, color);
      fill_solid(ledsR, RIGHT_COUNT, color);
    } else {
      FastLED.clear();
    }
    FastLED.show();
  }

private:
  static CRGB ledsL[LEFT_COUNT];
  static CRGB ledsR[RIGHT_COUNT];
};

template <uint8_t LP, uint16_t LC, uint8_t RP, uint16_t RC, uint8_t PP>
CRGB DeviceLight<LP, LC, RP, RC, PP>::ledsL[LC];

template <uint8_t LP, uint16_t LC, uint8_t RP, uint16_t RC, uint8_t PP>
CRGB DeviceLight<LP, LC, RP, RC, PP>::ledsR[RC];

#endif // DEVICELIGHT_H