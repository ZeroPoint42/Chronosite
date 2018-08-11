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

#define DEBUG true

#define NUM_UPPER_STRIPS 4
#define NUM_LOWER_STRIPS 4
#define NUM_OUTER_STRIPS 1
#define NUM_INNER_STRIPS (NUM_UPPER_STRIPS + NUM_LOWER_STRIPS)
#define NUM_LEDS_PER_OUTER_STRIP 4
#define NUM_LEDS_PER_INNER_STRIP 161

#define HUE_PIN 0         // sint value for the Hue laser pot
#define BRIGHTNESS_PIN 1  // int value for the Brightness laser pot
#define SPEED_PIN 2       // int value for the Speed laser pot
#define INNER_ANIM_PIN 12 // select the input pin for the button
#define OUTER_ANIM_PIN 11 // select the input pin for the button

int sensorValue_H = 0; // variable to store the value coming from the Hue sensor
int sensorValue_B = 0; // variable to store the value coming from the Brightness sensor
int sensorValue_S = 0; // variable to store the value coming from the Speed sensor

uint16_t gFps = 70;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t gBrightness = 127; //127 ~ 1/2 brightness [0-256]
byte gRed = 100;
byte gBlue = 100;
byte gGreen = 100;
uint32_t gFade = 4;
bool isFading = false;
CRGBPalette16 currentPalette;
TBlendType currentBlending;

// uint8_t buf[3],       // Enough for RGB parse; expand if using sensors
//     prevTime = 0L;    // For animation timing

bool buttonStateInner = HIGH; // current state of button. HIGH == ColorPicker
bool buttonStateOuter = HIGH; // current state of button. HIGH == ColorPicker

int innerAnimationIndex = 0; //Which animation is displaying
int outerAnimationIndex = 0; //Which animation is displaying
int paletteIndex = 0;
//long previousMillis = 0;   // Will store last time button was updated

int8_t pins[8] = {PIN_SERIAL1_RX, PIN_SERIAL1_TX, MISO, 13, 5, MOSI, A4, A3}; //Pins switched from default!! Hardware dependant!! https://learn.adafruit.com/adafruit-neopxl8-featherwing-and-library/neopxl8-arduino-library
Adafruit_NeoPXL8 innerLeds(NUM_LEDS_PER_INNER_STRIP, pins, NEO_GRB);
Adafruit_NeoPixel outerLeds = Adafruit_NeoPixel(NUM_LEDS_PER_OUTER_STRIP, 9, NEO_GRB);

/*****************************************************************************/

/*******************************SETUP********************************************************************/
void setup(void)
{
    innerLeds.begin();
    innerLeds.setBrightness(gBrightness); //50% Brightness (range: 1-256)

    pinMode(INNER_ANIM_PIN, INPUT_PULLUP);
    pinMode(OUTER_ANIM_PIN, INPUT_PULLUP);

#ifdef DEBUG
    innerAnimationIndex = 5;
    paletteIndex = 2;
    changePalette();
    innerLeds.setBrightness(25);
#endif

    Serial.begin(9600);
}

/******************************MAIN LOOP********************************************/

void loop(void)
{
    // Get current button state.
    bool newStateInr = digitalRead(INNER_ANIM_PIN);
    bool newStateOut = digitalRead(OUTER_ANIM_PIN);

    // Check if state changed from high to low (button press).
    EVERY_N_MILLISECONDS(20)
    {
        if (newStateInr == LOW && buttonStateInner == HIGH)
        {
            // Check if button is still low after debounce.
            newStateInr = digitalRead(INNER_ANIM_PIN);
            if (newStateInr == LOW)
            {
                innerLeds.clear();
                innerAnimationIndex++;
            }
        }
        if (newStateOut == LOW && buttonStateOuter == HIGH)
        {
            // Check if button is still low after debounce.
            newStateOut = digitalRead(OUTER_ANIM_PIN);
            if (newStateOut == LOW)
            {
                outerLeds.clear();
                outerAnimationIndex++;
            }
        }
        // Set the last button state to the old state.
        buttonStateInner = newStateInr;
        buttonStateOuter = newStateOut;
    }

    // Shift the hue over time.
    EVERY_N_MILLISECONDS(60)
    {
        gHue++;
    }

#ifdef DEBUG
    // For easier pattern testing.
    EVERY_N_SECONDS(30)
    {
        innerLeds.clear();
        innerAnimationIndex++;
    }
#endif

    showInnerAnimation();
    showOuterAnimation();

    innerLeds.show();

    FastLED.delay(1000 / gFps);
}

// *************************MAIN ANIMATION CYCLE***************************

void showInnerAnimation()
{
    switch (innerAnimationIndex)
    {
        case 0: 
            bpm(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
            break;
        case 1: 
            rainbow(gHue, 7, NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
            break;
        case 2:
            confetti(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
            break;
        case 3: 
            sinelon(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
            break;
        case 4: 
            juggle(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
            break;
        case 5:
            paletteCycle(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
            break;
        // case 6:
        //     RunningLights(red, green, blue, 200);
        //     break;
        // case 7:
        //     Sparkle(red, green, blue, 0);
        //     break;
        // case 8:
            // fadeInOut(gRed, gGreen, gBlue);
        //     break;
        default:
            // We've exceeded our allowed values, reset the index to 0.
            innerAnimationIndex = 0;
            paletteIndex++;
            changePalette();
            break;
    }
}

void showOuterAnimation()
{
    switch (outerAnimationIndex)
    {
        case 0: 
            bpm(NUM_OUTER_STRIPS, NUM_LEDS_PER_OUTER_STRIP, false);
            break;
        case 1: 
            rainbow(gHue, 7, NUM_OUTER_STRIPS, NUM_LEDS_PER_OUTER_STRIP, false);
            break;
        case 2:
            confetti(NUM_OUTER_STRIPS, NUM_LEDS_PER_OUTER_STRIP, false);
            break;
        case 3: 
            sinelon(NUM_OUTER_STRIPS, NUM_LEDS_PER_OUTER_STRIP, false);
            break;
        case 4: 
            juggle(NUM_OUTER_STRIPS, NUM_LEDS_PER_OUTER_STRIP, false);
            break;
        case 5:
            paletteCycle(NUM_OUTER_STRIPS, NUM_LEDS_PER_OUTER_STRIP, false);
            break;
        case 6:
            break;
        case 7:
            break;
        default:
            // We've exceeded our allowed values, reset the index to 0.
            outerAnimationIndex = 0;
            break;
    }
}
//*******************************FUNCTIONS********************************

void skipBytes(uint8_t n)
{
    // while (n--)
    // {
    //     while (Serial.read() < 0)
    //         ;
    // }
}

void LaserPots()
{
    // VL53L0X_RangingMeasurementData_t measure;

    // Serial.println("Reading a measurement... ");
    // loxL.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

    // if (measure.RangeStatus != 4)
    // {
    //     // phase failures have incorrect data
    //     Serial.print("Distance (mm): ");
    //     Serial.println(measure.RangeMilliMeter);
    // }
    // else
    // {
    //     Serial.println(" out of range ");
    // }
}

//******************************ANIMATIONS**********************************

void rainbow(uint8_t initialHue, uint8_t deltaHue, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    CHSV hsv;
    hsv.hue = initialHue;
    hsv.val = 255;
    hsv.sat = 240;

     // FastLED's built-in rainbow generator
    for (uint8_t i = 0; i < numStrips; i++)
    {
        for(int j = 0; j < numLedsPerStrip; j++) 
        {
            setPixelColor(i, j, hsv, isInner);
            hsv.hue += deltaHue;
        }
    }
}

void rainbowWithGlitter(uint8_t initialHue, uint8_t deltaHue, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow(initialHue, deltaHue, numStrips, numLedsPerStrip, isInner);
    addGlitter(10, numStrips, numLedsPerStrip, isInner);
}

void addGlitter(fract8 chanceOfGlitter, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    //add some sparkle
    if (random8() < chanceOfGlitter)
    {
        for (uint8_t i = 0; i < numStrips; i++)
        {
            setPixelColor(i, random16(numLedsPerStrip), CRGB::White, isInner);
        }
    }
}

void bpm(uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 60;
    CRGBPalette16 palette = OceanColors_p;
    //CRGBPalette16 palette2 = LavaColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (uint8_t i = 0; i < NUM_INNER_STRIPS; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_INNER_STRIP; j++)
        {
            CRGB color = ColorFromPalette(palette, gHue + (j * 2), beat - gHue + (j * 10));
            setPixelColor(i, j, color, isInner);
        }
    }
}

void sinelon(uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    // a colored dot sweeping back and forth, with fading trails
    for (uint8_t i = 0; i < numStrips; i++)
    {
        fadeAll(gFade, numStrips, numLedsPerStrip, isInner);
        int pos = beatsin16(13, 0, numLedsPerStrip - 1);
        CRGB rgb = getCRGB(i, pos, isInner);
        rgb += CHSV(gHue, 255, 192);

        setPixelColor(i, pos, rgb, isInner);
    }
}

void juggle(uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    // eight colored dots, weaving in and out of sync with each other
    for (uint8_t i = 0; i < numStrips; i++)
    {
        fadeAll(gFade, numStrips, numLedsPerStrip, isInner);
        byte dothue = 0;
        for (int j = 0; j < 8; j++)
        {
            int pos = beatsin16(j + 6, 0, numLedsPerStrip - 1);
            CRGB rgb = getCRGB(i, pos, isInner);
            rgb |= CHSV(dothue, 200, 255);
            dothue += 32;

            setPixelColor(i, pos, rgb, isInner);
        }
    }
}

void confetti(uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    // random colored speckles that blink in and fade smoothly
    for (uint8_t i = 0; i < numStrips; i++)
    {
        fadeAll(gFade, numStrips, numLedsPerStrip, isInner);
        int pos = random16(numLedsPerStrip);
        CHSV hsv = CHSV(gHue + random8(64), 255, 255);

        addPixelColor(i, pos, hsv, isInner);
    }
}

// void theaterChase(byte red, byte green, byte blue, int SpeedDelay)
// { //3 on, 3 off chasing lights
//     if (currentMillis - previousMillis > SpeedDelay)
//     { // Time to update
//         previousMillis = currentMillis;
//         for (uint8_t i = 0; i < NUM_INNER_STRIPS; i++)
//         {
//             for (int q = 0; q < 3; q++)
//             {
//                 for (int i = 0; i < NUM_LEDS_PER_INNER_STRIP; i = i + 3)
//                 {
//                     leds[x][i + q] = CRGB(red, green, blue); //turn every third pixel on
//                 }
//                 FastLED.show();

//                 for (int i = 0; i < NUM_LEDS_PER_INNER_STRIP; i = i + 3)
//                 {
//                     leds[x][i + q] = CRGB(0, 0, 0); //turn every third pixel off
//                 }
//             }
//         }
//     }
// }

void fadeInOut(byte red, byte green, byte blue)
{
    // float r, g, b;
    // if (!isFading)
    // { 
    //     if (gFade < 65)
    //     {
    //         gFade++;
    //     }
    //     else
    //     {
    //         gFade = 64;
    //         isFading = true;
    //     }
    // }
    
    // if (isFading)
    // {
    //     if (gFade >= 0)
    //     {
    //         gFade -= 2;
    //     }
    //     else
    //     {
    //         gFade = 0;
    //         isFading = false;
    //     }
    // }

    // //64 ~ 1/4 brightness
    // r = (gFade / 256.0) * red;
    // g = (gFade / 256.0) * green;
    // b = (gFade / 256.0) * blue;
    // allColor(r, g, b);
}

void Sparkle(byte red, byte green, byte blue, int SpeedDelay, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    // int Pixel = random(NUM_LEDS_PER_INNER_STRIP);
    // setPixel(Pixel, red, green, blue);
    // showStrip();
    // delay(SpeedDelay);
    // setPixel(Pixel, 0, 0, 0);
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    // //sin wave running lights
    // int Position = 0;
    // unsigned long currentMillis = millis();
    // // Time to update
    // //if (currentMillis - previousMillis > WaveDelay) {
    // //previousMillis = currentMillis;

    // for (int i = 0; i < NUM_LEDS_PER_INNER_STRIP * 2; i++)
    // {
    //     Position++; // = 0; //Position + Rate;
    //     for (int i = 0; i < NUM_LEDS_PER_INNER_STRIP; i++)
    //     {
    //         // sine wave, 3 offset waves make a rainbow!
    //         //float level = sin(i+Position) * 127 + 128;
    //         //setPixel(i,level,0,0);
    //         //float level = sin(i+Position) * 127 + 128;
    //         setPixel(i, ((sin(i + Position) * 127 + 128) / 255) * red,
    //                  ((sin(i + Position) * 127 + 128) / 255) * green,
    //                  ((sin(i + Position) * 127 + 128) / 255) * blue);
    //     }
    //     EVERY_N_MILLISECONDS(100)
    //     {
    //         showStrip();
    //     }
    // }
}

void paletteCycle(uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */

    FillLEDsFromPaletteColors(startIndex, numStrips, numLedsPerStrip, isInner);
}

void changePalette()
{
    switch (paletteIndex)
    {
        case 0:
            currentPalette = OceanColors_p;
            currentBlending = LINEARBLEND;
            break;
        case 1:
            currentPalette = LavaColors_p;
            currentBlending = LINEARBLEND;
            break;
        case 2:
            SetupPurpleAndGreenPalette();
            currentBlending = LINEARBLEND;
            break;
        default:
            paletteIndex = 0;
            break;
    }
}

// void doThingsToDifferentStrips(bool isInner)
// {  
//     for (uint8_t i = 0; i < NUM_UPPER_STRIPS; i++)
//     {
//         for (int j = 0; j < NUM_LEDS_PER_INNER_STRIP; j++)
//         {
//             CRGB rgb = CRGB(0, 0, 255);
//             setPixelColor(i, j, rgb, isInner);
//         }
//     }
//     for (uint8_t i = NUM_UPPER_STRIPS; i < NUM_INNER_STRIPS; i++)
//     {
//         for (int j = 0; j < NUM_LEDS_PER_INNER_STRIP; j++)
//         {
//             CRGB rgb = CRGB(0, 0, 255);
//             setPixelColor(i, j, rgb, isInner);
//         }
//     }
// }

void FillLEDsFromPaletteColors(uint8_t colorIndex, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    currentPalette = OceanColors_p;
    for (uint8_t i = 0; i < numStrips; i++)
    {
        for (int j = 0; j < numLedsPerStrip; j++)
        {
            CRGB rgb = ColorFromPalette(currentPalette, colorIndex, gBrightness, currentBlending);
            colorIndex += 1;

            setPixelColor(i, j, rgb, isInner);
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

// Changes all LEDS to given color
void allColor(byte red, byte green, byte blue, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    for (uint8_t i = 0; i < numStrips; i++)
    {
        for (int j = 0; j < numLedsPerStrip; j++)
        {
            setPixelColor(i, j, CRGB(red, green, blue), isInner);
        }
    }
}

void allColor(CRGB color, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    allColor(color.red, color.green, color.blue, numStrips, numLedsPerStrip, isInner);
}

void fadeAll(int factor, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    if (factor < 1) factor = 1;

    for (uint8_t i = 0; i < numStrips; i++)
    {
        for (int j = 0; j < numLedsPerStrip; j++)
        {
            uint32_t colorInt = innerLeds.getPixelColor((i * numLedsPerStrip) + j);

            byte red = (colorInt >> 16) & 255;
            byte green = (colorInt >> 8) & 255;
            byte blue = colorInt & 255;

            CRGB rgb = CRGB(red, green, blue);
            rgb.fadeToBlackBy(factor);

            setPixelColor(i, j, rgb, isInner);
        }
    }
}

void setPixelColor(int row, int pos, CHSV hsv, bool isInner)
{   
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);
    
    setPixelColor(row, pos, rgb, isInner);
}

void setPixelColor(int row, int pos, CRGB color, bool isInner)
{
    if (isInner)
    {
        innerLeds.setPixelColor((row * NUM_LEDS_PER_INNER_STRIP) + pos, color.red, color.green, color.blue);
    }
    else
    {
        outerLeds.setPixelColor(pos, color.red, color.green, color.blue);
    }
}

void addPixelColor(int row, int pos, CHSV hsv, bool isInner)
{
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);

    addPixelColor(row, pos, rgb, isInner);
}

void addPixelColor(int row, int pos, CRGB rgb, bool isInner)
{
    rgb += getCRGB(row, pos, isInner);

    setPixelColor(row, pos, rgb, isInner);
}

CRGB getCRGB(int row, int pos, bool isInner)
{
    uint32_t colorInt;
    if (isInner)
    {
        colorInt = innerLeds.getPixelColor((row * NUM_LEDS_PER_INNER_STRIP) + pos);
    }
    else
    {
        colorInt = outerLeds.getPixelColor(pos);
    }

    byte red = (colorInt >> 16) & 255;
    byte green = (colorInt >> 8) & 255;
    byte blue = colorInt & 255;

    return CRGB(red, green, blue);
}