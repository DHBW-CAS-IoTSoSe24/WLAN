/**
 * @file main.cpp
 * @author Team WLAN
 * @brief 
 * @version 0.1
 * @date 2024-09-06
 * 
 * @copyright Copyright (c) 2024
 * 
 * 
 * Module doc: https://www.waveshare.com/wiki/ESP32-C6-DEV-KIT-N8
 * 
 */

#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 1
#define DATA_PIN 8

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
}

void fill(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
}

void loop() {
  leds[0].red = 10;
  FastLED.show();
  delay(500);
  leds[0] = 0x000000;
  FastLED.show();
  delay(500);
}
