#include "twinklefox.h"
#include <Arduino.h>
#include <FastLED.h>
#include <OneButton.h>

FASTLED_USING_NAMESPACE;

#define DEBUG 1

const int maxBrightness = 192;
const int buttonPin1 = D1;
const int dataPin = D4;
const int clockPin = D5;

const int CYCLE_SPEED = 4;

#define PULLUP false
#define FRAMES_PER_SECOND 240

OneButton myButton(buttonPin1, PULLUP);

#define NUM_LEDS 16
CRGBArray<NUM_LEDS> leds;

int currentEffect = 0, setupEffect = -1;

typedef void (*EffectFunction)(int);

void lightsOff(int);
void rainbow(int);
void clouds(int);
void nightlight(int);

EffectFunction effectFunctions[] = {
    lightsOff,
    rainbow,
    clouds,
    nightlight,
};
const int effectCount = 4;

///////////////////////////////// EFFECTS ///////////////////////////////////

void lightsOff(int clicker) { fill_solid(leds, NUM_LEDS, CRGB::Black); }

void rainbow(int clicker) {
  fill_palette(leds, NUM_LEDS, clicker, 4, RainbowColors_p, 255, LINEARBLEND);

  // occasionally twinkle a random light (actually never)
  if (random16() < 256 && 0)
    leds[random8(NUM_LEDS)] = CRGB::White;
}

void clouds(int clicker) {
  if (setupEffect != currentEffect) {
    // set up clouds
    chooseNextColorPalette(1);
    FastLED.setBrightness(255);
    setupEffect = currentEffect;
  }

  drawTwinkles(leds, 4);
}

void nightlight(int clicker) {
  if (setupEffect != currentEffect) {
    // set up nightlight
    chooseNextColorPalette(6);
    FastLED.setBrightness(64);
    setupEffect = currentEffect;
  }
  drawTwinkles(leds, 2);
}

///////////////////////////////// MAIN LIGHT CODE ///////////////////////////

volatile int idx = 0, ledClock = 0;
void lightUpdate() {
  effectFunctions[currentEffect](idx);

  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  ledClock++;
  if (ledClock > CYCLE_SPEED) {
    idx++;
    ledClock = 0;
    if (idx > 255)
      idx = 0;
  }
}

void nextEffect() {
  currentEffect++;

  if (currentEffect == effectCount)
    currentEffect = 0;

#if DEBUG
  Serial.print("Current effect: ");
  Serial.println(currentEffect);
#endif

  lightUpdate();
}

void setup() {
#if DEBUG
  Serial.begin(9600);
#endif
  delay(1000);

  FastLED.addLeds<APA102, dataPin, clockPin, BGR>(leds, NUM_LEDS);

  FastLED.setCorrection(TypicalPixelString);
  FastLED.setTemperature(Halogen);
  FastLED.setDither(true);

  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(255);

  lightUpdate();
  myButton.attachClick(nextEffect);
}

void loop() {
  myButton.tick();
  lightUpdate();
}
