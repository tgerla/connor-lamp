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

int currentEffect = 3, setupEffect = -1;

typedef void (*EffectFunction)(int);

void lightsOff(int);
void rainbow(int);
void clouds(int);
void eyes(int);
void nightlight(int);
void purples(int);

EffectFunction effectFunctions[] = {
    lightsOff, rainbow, clouds, eyes, nightlight, purples,
};
const int effectCount = 6;

///////////////////////////////// EFFECTS ///////////////////////////////////

void lightsOff(int clicker) { fill_solid(leds, NUM_LEDS, CRGB::Black); }

void rainbow(int clicker) {
  fill_palette(leds, NUM_LEDS, clicker, 4, RainbowColors_p, 255, LINEARBLEND);

  // occasionally twinkle a random light (actually never)
  if (random16() < 256 && 0)
    leds[random8(NUM_LEDS)] = CRGB::White;
}

// clang-format off
DEFINE_GRADIENT_PALETTE(clouds_gp){
  0,  255,   255,   255,  
  64,  0,  0,  255,
  128, 255, 255,  255,
  192, 0, 0, 255,
  255, 255, 255, 255
  };
CRGBPalette16 cloudsPalette = clouds_gp;
// clang-format on

void clouds(int clicker) {
  if (setupEffect != currentEffect) {
    // set up clouds
    FastLED.setBrightness(255);
    setupEffect = currentEffect;
  }

  drawTwinkles(leds, 3, cloudsPalette, CRGB::Blue, 5);
}

int eyesCounter;
int fadeRate;
int darkCounter;
int eyesLit;
int eyesHue;

void eyes(int clicker) {
  if (setupEffect != currentEffect) {
    eyesCounter = 0;
    fadeRate = 0;
    darkCounter = 0;

    fill_solid(leds, NUM_LEDS, CHSV(0, 0, 0));
    setupEffect = currentEffect;
  }

  // initial state
  if (eyesCounter == 0 && darkCounter == 0) {
    // make sure that the eyes are always lit up on the same section of strip
    //     position on strip + first or second strip offset
    eyesLit = random8(7) + (random8(2) * 8);

    darkCounter = random8();
    eyesCounter = 1024; // random16(255, 1024);
    fadeRate = eyesCounter / 256;

    //    leds[eyesLit] = eyeColors[random8(7)];
    //    fill_rainbow(&(leds[eyesLit]), 1 /*led count*/, random8());
    eyesHue = random8();
    leds[eyesLit].setHSV(eyesHue, 255, 255);
    Serial.println(eyesHue);
    Serial.println(eyesLit);

    FastLED.show();
  } else if (eyesCounter == 0 && darkCounter > 0) { // waiting for dark counter
    darkCounter--;
  } else if (eyesCounter > 0 && darkCounter > 0) { // dimming eyes
    eyesCounter--;

    //    if (!mod8(eyesCounter, fadeRate)) {
    //  leds[eyesLit].fadeToBlackBy(2);
    nblend(leds[eyesLit], CRGB::Black, 1);
    // leds[eyesLit].setHSV(eyesHue, 255, eyesCounter / fadeRate);
    // fadeToBlackBy(leds, NUM_LEDS, 1);
    //    }
  }
}

// clang-format off
DEFINE_GRADIENT_PALETTE(nightlight_gp){
    0,  255, 0,   0,
    192, 255, 0, 166,
    255, 255, 0, 166,
};
CRGBPalette16 nightlightPal = nightlight_gp;
// clang-format on

void nightlight(int clicker) {
  if (setupEffect != currentEffect) {
    // set up nightlight
    FastLED.setBrightness(32);
    setupEffect = currentEffect;
  }
  drawTwinkles(leds, 1, nightlightPal, CRGB(8, 0, 0), 1);
}

DEFINE_GRADIENT_PALETTE(Pink_Purple_gp){
    0,   19,  2,   39,  25,  26,  4,   45,  51,  33,  6,   52,  76,  68,  62,
    125, 102, 118, 187, 240, 109, 163, 215, 247, 114, 217, 244, 255, 122, 159,
    149, 221, 149, 113, 78,  188, 183, 128, 57,  155, 255, 146, 40,  123};
CRGBPalette16 purplesPal = Pink_Purple_gp;

void purples(int clicker) {
  if (setupEffect != currentEffect) {
    // set up nightlight
    FastLED.setBrightness(255);
    setupEffect = currentEffect;
  }
  drawTwinkles(leds, 2, purplesPal, CRGB::MediumPurple, 5);
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
  // FastLED.setTemperature(Halogen);
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
