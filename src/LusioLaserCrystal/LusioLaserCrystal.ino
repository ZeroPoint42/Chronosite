/*********************************************************************
  EMANATE's Chronosite! Time crystal
  emanatedesigns.com

  All code is open sorce! Sourced from Adafruit.com, FastLED.io, Github, Kyle Hadley, and my own.
  Feel free to tweak and modify colors and patterns.
  If you design custom animations and patterns please shoot me a video at info@emanatedesigns.com,
  I wanna see what you can do!!
  Maybe we can incorporate them into future versions, with credit to you of course.

  I support Adafruit 110%! Their team has helped me learn so much
  and they have some amazing products. I almost exclusivly order from them.

  This is a [modified] example for our nRF51822 based Bluefruit LE modules
  Pick one up today in the adafruit shop!
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/

#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "FastLED.h"
#include "Adafruit_NeoPixel.h"
#include "Adafruit_NeoPXL8.h"
#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif
#include "Adafruit_VL53L0X.h"

#define NUM_STRIPS_UPPER 4
#define NUM_STRIPS_LOWER 4
#define NUM_LEDS_PER_UPPER_STRIP 161
#define NUM_LEDS_PER_LOWER_STRIP 158
#define NUM_LEDS_UPPER_TOTAL (NUM_STRIPS_UPPER * NUM_LEDS_PER_UPPER_STRIP)
#define NUM_LEDS_LOWER_TOTAL (NUM_STRIPS_LOWER * NUM_LEDS_PER_LOWER_STRIP)
#define NUM_LEDS_TOTAL (NUM_LEDS_UPPER_TOTAL + NUM_LEDS_LOWER_TOTAL)
#define FPS 120 // Animation frames/second (ish)
#define LED_TYPE WS2812B
#define COLOR_ORDER NEO_GRB
#define BRIGHTNESS 127 //127 ~ 1/2 brightness [0-256]

#define HueInput 0;            // sint value for the Hue laser pot
#define BrightnessInput 1;     // int value for the Brightness laser pot
#define AnimationSpeedInput 2; // int value for the Speed laser pot
#define InnerAnimation 12;     // select the input pin for the button
#define OuterAnimation 11;     // select the input pin for the button

int sensorValue_H = 0; // variable to store the value coming from the Hue sensor
int sensorValue_B = 0; // variable to store the value coming from the Brightness sensor
int sensorValue_S = 0; // variable to store the value coming from the Speed sensor

int8_t pins[8] = {PIN_SERIAL1_RX, PIN_SERIAL1_TX, MISO, 13, 5, MOSI, A4, A3}; //Pins switched from default!! Hardware dependant!! https://learn.adafruit.com/adafruit-neopxl8-featherwing-and-library/neopxl8-arduino-library
Adafruit_NeoPXL8 PXL8(NUM_LEDS_PER_UPPER_STRIP, pins, NEO_GRB);

uint8_t red = 100;
uint8_t green = 100;
uint8_t blue = 100;
int animationStateInr = 0; //Which animation is displaying
int animationStateOut = 0; //Which animation is displaying
long previousMillis = 0;   // Will store last time button was updated

/*****************************************************************************/

/*******************************SETUP********************************************************************/
void setup(void)
{
    PXL8.begin();
    PXL8.setBrightness(BRIGHTNESS); //50% Brightness (range: 1-256)

    pinMode(InnerAnimation, INPUT_PULLUP);
    pinMode(OuterAnimation, INPUT_PULLUP);

    Serial.begin(115200);
}

uint8_t gHue = 0;     // rotating "base color" used by many of the patterns
uint8_t buf[3],       // Enough for RGB parse; expand if using sensors
    prevTime = 0L;    // For animation timing
bool stateInr = HIGH; // current state of button. HIGH == ColorPicker
bool stateOut = HIGH; // current state of button. HIGH == ColorPicker
int previous = LOW;   // previous reading         LOW == Rainbow

/******************************MAIN LOOP********************************************/

void loop(void)
{
    // Get current button state.
    bool newStateInr = digitalRead(InnerAnimation);
    bool newStateOut = digitalRead(OuterAnimation);

    // Check if state changed from high to low (button press).
    EVERY_N_MILLISECONDS(20)
    {
        if (newStateInr == LOW && stateInr == HIGH)
        {
            // Check if button is still low after debounce.
            newStateInr = digitalRead(InnerAnimation);
            if (newStateInr == LOW)
            {
                animationStateInr++;
                if (animationStateInr > 9)
                    animationStateInr = 0;
                startShowInr(animationStateInr);
            }
        }
        if (newStateOut == LOW && stateOut == HIGH)
        {
            // Check if button is still low after debounce.
            newStateOut = digitalRead(OuterAnimation);
            if (newStateOut == LOW)
            {
                animationStateOut++;
                if (animationStateOut > 9)
                    animationStateOut = 0;
                startShowOut(animationStateOut);
            }
        }
        // Set the last button state to the old state.
        stateInr = newStateInr;
        stateOut = newStateOut;
    }
}

// *************************MAIN ANIMATION CYCLE***************************

void showTimeInr(int i)
{

    switch (i)
    {
    case 0: // Juggle  *[Button 1]*
        juggle();
        break;

    case 1: // Confetti *[Button 2]*
        EVERY_N_MILLISECONDS(30)
        {
            gHue++; // slowly cycle the "base color" through the rainbow
        }
        confetti();
        break;

    case 2: // Rainbow Glitter *[Button 3]*
        EVERY_N_MILLISECONDS(20)
        {
            gHue++; // slowly cycle the "base color" through the rainbow
        }
        rainbowWithGlitter();
        break;

    case 3: // Sinelon Rainbow *[Button 4]*
        EVERY_N_MILLISECONDS(20)
        {
            gHue++; // slowly cycle the "base color" through the rainbow
        }
        sinelon();
        break;

    case 4: // Palette Cycle *[UP Button5]*
        PaletteCycle();
        break;

    case 5: // FadeInOut {Color Picker} *[DOWN Button6]*
        FadeInOut(red, green, blue);
        break;

    case 6: // RunningLights {Color Picker} *[LEFT Button7]*
        RunningLights(red, green, blue, 200);
        break;

    case 7: //Sparkle {Color Picker} *[RIGHT Button8]*
        Sparkle(red, green, blue, 0);
        break;
    }
}

void showTimeOut(int i)
{

    switch (i)
    {
    case 0: // Juggle  *[Button 1]*
        juggle();
        break;

    case 1: // Confetti *[Button 2]*
        EVERY_N_MILLISECONDS(30)
        {
            gHue++; // slowly cycle the "base color" through the rainbow
        }
        confetti();
        break;

    case 2: // Rainbow Glitter *[Button 3]*
        EVERY_N_MILLISECONDS(20)
        {
            gHue++; // slowly cycle the "base color" through the rainbow
        }
        rainbowWithGlitter();
        break;

    case 3: // Sinelon Rainbow *[Button 4]*
        EVERY_N_MILLISECONDS(20)
        {
            gHue++; // slowly cycle the "base color" through the rainbow
        }
        sinelon();
        break;

    case 4: // Palette Cycle *[UP Button5]*
        PaletteCycle();
        break;

    case 5: // FadeInOut {Color Picker} *[DOWN Button6]*
        FadeInOut(red, green, blue);
        break;

    case 6: // RunningLights {Color Picker} *[LEFT Button7]*
        RunningLights(red, green, blue, 200);
        break;

    case 7: //Sparkle {Color Picker} *[RIGHT Button8]*
        Sparkle(red, green, blue, 0);
        break;
    }
}
//*******************************FUNCTIONS********************************

void skipBytes(uint8_t n)
{
    while (n--)
    {
        while (Serial.read() < 0)
            ;
    }
}

void fadeall()
{
    //fades smoothly on/off
    for (uint8_t x = 0; x < NUM_STRIPS; x++)
    {
        for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
        {
            leds[x][i].nscale8(180);
        }
    }
}

void LaserPots()
{
    VL53L0X_RangingMeasurementData_t measure;

    Serial.print("Reading a measurement... ");
    loxL.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

    if (measure.RangeStatus != 4)
    { // phase failures have incorrect data
        Serial.print("Distance (mm): ");
        Serial.println(measure.RangeMilliMeter);
    }
    else
    {
        Serial.println(" out of range ");
    }
}

//******************************ANIMATIONS**********************************

void rainbow()
{ // FastLED's built-in rainbow generator
    for (uint8_t x = 0; x < NUM_STRIPS; x++)
    {
        fill_rainbow(leds[x], NUM_LEDS_PER_STRIP, gHue, 7);
    }
    FastLED.show();
}

void rainbowWithGlitter()
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter)
{ //add some sparkle
    if (random8() < chanceOfGlitter)
    {
        for (uint8_t x = 0; x < NUM_STRIPS; x++)
        {
            leds[x][random16(NUM_LEDS_PER_STRIP)] += CRGB::White;

            FastLED.show();
        }
    }
}

void bpm()
{ // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 60;
    CRGBPalette16 palette = OceanColors_p;
    //CRGBPalette16 palette2 = LavaColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (uint8_t x = 0; x < NUM_STRIPS; x++)
    {
        for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
        { //9948
            leds[x][i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        }
    }
    FastLED.show();
}

void sinelon()
{ // a colored dot sweeping back and forth, with fading trails
    for (uint8_t x = 0; x < NUM_STRIPS; x++)
    {
        fadeToBlackBy(leds[x], NUM_LEDS_PER_STRIP, 20);
        int pos = beatsin16(13, 0, NUM_LEDS_PER_STRIP - 1);
        leds[x][pos] += CHSV(gHue, 255, 192);
    }
    FastLED.show();
}

void juggle()
{ // eight colored dots, weaving in and out of sync with each other
    for (uint8_t x = 0; x < NUM_STRIPS; x++)
    {
        fadeToBlackBy(leds[x], NUM_LEDS_PER_STRIP, 30);
        byte dothue = 0;
        for (int i = 0; i < 8; i++)
        {
            leds[x][beatsin16(i + 6, 0, NUM_LEDS_PER_STRIP - 1)] |= CHSV(dothue, 200, 255);
            dothue += 32;
        }
    }
    FastLED.show();
}

void confetti()
{ // random colored speckles that blink in and fade smoothly
    for (uint8_t x = 0; x < NUM_STRIPS; x++)
    {
        fadeToBlackBy(leds[x], NUM_LEDS_PER_STRIP, 10);
        int pos = random16(NUM_LEDS_PER_STRIP);
        leds[x][pos] += CHSV(gHue + random8(64), 255, 255);
    }
    FastLED.show();
}

// void theaterChase(byte red, byte green, byte blue, int SpeedDelay)
// { //3 on, 3 off chasing lights
//     if (currentMillis - previousMillis > SpeedDelay)
//     { // Time to update
//         previousMillis = currentMillis;
//         for (uint8_t x = 0; x < NUM_STRIPS; x++)
//         {
//             for (int q = 0; q < 3; q++)
//             {
//                 for (int i = 0; i < NUM_LEDS_PER_STRIP; i = i + 3)
//                 {
//                     leds[x][i + q] = CRGB(red, green, blue); //turn every third pixel on
//                 }
//                 FastLED.show();

//                 for (int i = 0; i < NUM_LEDS_PER_STRIP; i = i + 3)
//                 {
//                     leds[x][i + q] = CRGB(0, 0, 0); //turn every third pixel off
//                 }
//             }
//         }
//     }
// }

void FadeInOut(byte red, byte green, byte blue)
{
    float r, g, b;
    for (int k = 0; k < 65; k = k + 1)
    { //64 ~ 1/4 brightness
        r = (k / 256.0) * red;
        g = (k / 256.0) * green;
        b = (k / 256.0) * blue;
        setAll(r, g, b);
        showStrip();
    }
    for (int k = 64; k >= 0; k = k - 2)
    {
        r = (k / 256.0) * red;
        g = (k / 256.0) * green;
        b = (k / 256.0) * blue;
        setAll(r, g, b);
        showStrip();
    }
}

void Sparkle(byte red, byte green, byte blue, int SpeedDelay)
{
    int Pixel = random(NUM_LEDS_PER_STRIP);
    setPixel(Pixel, red, green, blue);
    showStrip();
    delay(SpeedDelay);
    setPixel(Pixel, 0, 0, 0);
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay)
{ 
    //sin wave running lights
    int Position = 0;
    unsigned long currentMillis = millis();
    // Time to update
    //if (currentMillis - previousMillis > WaveDelay) {
    //previousMillis = currentMillis;

    for (int i = 0; i < NUM_LEDS_PER_STRIP * 2; i++)
    {
        Position++; // = 0; //Position + Rate;
        for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
        {
            // sine wave, 3 offset waves make a rainbow!
            //float level = sin(i+Position) * 127 + 128;
            //setPixel(i,level,0,0);
            //float level = sin(i+Position) * 127 + 128;
            setPixel(i, ((sin(i + Position) * 127 + 128) / 255) * red,
                     ((sin(i + Position) * 127 + 128) / 255) * green,
                     ((sin(i + Position) * 127 + 128) / 255) * blue);
        }
        EVERY_N_MILLISECONDS(100)
        {
            showStrip();
        }
    }
}

void PaletteCycle()
{
    //ChangePalettePeriodically();

    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */

    FillLEDsFromPaletteColors(startIndex);

    FastLED.show();
}

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 30;

    if (lastSecond != secondHand)
    {
        lastSecond = secondHand;
        if (secondHand == 0)
        {
            currentPalette = OceanColors_p;
            currentBlending = LINEARBLEND;
        }
        if (secondHand == 10)
        {
            currentPalette = LavaColors_p;
            currentBlending = LINEARBLEND;
        }
        if (secondHand == 20)
        {
            SetupPurpleAndGreenPalette();
            currentBlending = LINEARBLEND;
        }
    }
}

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
    currentPalette = OceanColors_p;
    for (uint8_t x = 0; x < NUM_STRIPS; x++)
    {
        for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
        {
            leds[x][i] = ColorFromPalette(currentPalette, colorIndex, BRIGHTNESS, currentBlending);
            colorIndex += 1;
        }
    }
}

void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV(HUE_PURPLE, 64, 64);
    CRGB green = CHSV(HUE_GREEN, 64, 64);
    CRGB black = CRGB::Black;

    currentPalette = CRGBPalette16(
        green, green, black, black,
        purple, purple, black, black,
        green, green, black, black,
        purple, purple, black, black);
}
void SetupTotallyRandomPalette()
{
    for (int i = 0; i < 16; i++)
    {
        currentPalette[i] = CHSV(random8(), 64, random8());
    }
}
//**************************FastLED_Helpers*************************************

void showStrip()
{
    // FastLED
    FastLED.show();
}

void setPixel(int Pixel, byte red, byte green, byte blue)
{
    for (uint8_t x = 0; x < NUM_STRIPS; x++)
    {
        // FastLED
        leds[x][Pixel].r = red;
        leds[x][Pixel].g = green;
        leds[x][Pixel].b = blue;
    }
}

void setAll(byte red, byte green, byte blue)
{
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
    {
        setPixel(i, red, green, blue);
    }
    showStrip();
}

// Changes all LEDS to given color
void allColor(byte red, byte green, byte blue)
{
    for (uint8_t x = 0; x < NUM_STRIPS; x++)
    {
        for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
        {
            leds[x][i] = CRGB(red, green, blue);
        }
        FastLED.show();
    }
}
