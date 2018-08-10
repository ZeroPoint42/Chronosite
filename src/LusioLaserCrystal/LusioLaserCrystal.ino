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
#define NUM_STRIPS (NUM_UPPER_STRIPS + NUM_LOWER_STRIPS)
#define NUM_OUTER_LEDS 4
#define NUM_LEDS_PER_STRIP 33// 161

#define FPS 70        // Animation frames/second (ish)
#define BRIGHTNESS 127 //127 ~ 1/2 brightness [0-256]

#define HUE_PIN 0         // sint value for the Hue laser pot
#define BRIGHTNESS_PIN 1  // int value for the Brightness laser pot
#define SPEED_PIN 2       // int value for the Speed laser pot
#define INNER_ANIM_PIN 12 // select the input pin for the button
#define OUTER_ANIM_PIN 11 // select the input pin for the button

int sensorValue_H = 0; // variable to store the value coming from the Hue sensor
int sensorValue_B = 0; // variable to store the value coming from the Brightness sensor
int sensorValue_S = 0; // variable to store the value coming from the Speed sensor

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
byte gRed = 100;
byte gBlue = 100;
byte gGreen = 100;
uint32_t gFade = 0;
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
Adafruit_NeoPXL8 innerLeds(NUM_LEDS_PER_STRIP, pins, NEO_GRB);
Adafruit_NeoPixel outerLeds = Adafruit_NeoPixel(NUM_OUTER_LEDS, 9, NEO_GRB);

/*****************************************************************************/

/*******************************SETUP********************************************************************/
void setup(void)
{
    innerLeds.begin();
    innerLeds.setBrightness(BRIGHTNESS); //50% Brightness (range: 1-256)

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
    //   for(uint8_t r=0; r<8; r++) { // For each row...
    //     for(int p=0; p<NUM_LED; p++) { // For each pixel of row...
    //       leds.setPixelColor(r * NUM_LED + p, rain(r, p));
    //     }
    //   }
    //   leds.show();
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
    EVERY_N_SECONDS(10)
    {
        innerLeds.clear();
        innerAnimationIndex++;
    }
#endif

    showInnerAnimation();
    showOuterAnimation();

    innerLeds.show();

    FastLED.delay(1000 / FPS);
}

// *************************MAIN ANIMATION CYCLE***************************

void showInnerAnimation()
{
    switch (innerAnimationIndex)
    {
        case 0: 
            bpm();
            break;
        case 1: 
            rainbow(gHue, 7);
            break;
        case 2:
            confetti();
            break;
        case 3: 
            sinelon();
            break;
        case 4: 
            juggle();
            break;
        case 5:
            paletteCycle();
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
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
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

void rainbow(uint8_t initialHue, uint8_t deltaHue)
{
    CHSV hsv;
    hsv.hue = initialHue;
    hsv.val = 255;
    hsv.sat = 240;

     // FastLED's built-in rainbow generator
    for (uint8_t i = 0; i < NUM_STRIPS; i++)
    {
        for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) 
        {
            setPixelColor(i, j, hsv);
            hsv.hue += deltaHue;
        }
    }
}

void rainbowWithGlitter(uint8_t initialHue, uint8_t deltaHue)
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow(initialHue, deltaHue);
    addGlitter(10);
}

void addGlitter(fract8 chanceOfGlitter)
{
    //add some sparkle
    if (random8() < chanceOfGlitter)
    {
        for (uint8_t i = 0; i < NUM_STRIPS; i++)
        {
            setPixelColor(i, random16(NUM_LEDS_PER_STRIP), CRGB::White);
        }
    }
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 60;
    CRGBPalette16 palette = OceanColors_p;
    //CRGBPalette16 palette2 = LavaColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (uint8_t i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_STRIP; j++)
        {
            CRGB color = ColorFromPalette(palette, gHue + (j * 2), beat - gHue + (j * 10));
            setPixelColor(i, j, color);
        }
    }
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    for (uint8_t i = 0; i < NUM_STRIPS; i++)
    {
        fadeAll(4);
        int pos = beatsin16(13, 0, NUM_LEDS_PER_STRIP - 1);
        CRGB rgb = getCRGB(i, pos);
        rgb += CHSV(gHue, 255, 192);

        setPixelColor(i, pos, rgb);
    }
}

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    for (uint8_t i = 0; i < NUM_STRIPS; i++)
    {
        fadeAll(4);
        byte dothue = 0;
        for (int j = 0; j < 8; j++)
        {
            int pos = beatsin16(j + 6, 0, NUM_LEDS_PER_STRIP - 1);
            CRGB rgb = getCRGB(i, pos);
            rgb |= CHSV(dothue, 200, 255);
            dothue += 32;

            setPixelColor(i, pos, rgb);
        }
    }
}

void confetti()
{
    // random colored speckles that blink in and fade smoothly
    for (uint8_t i = 0; i < NUM_STRIPS; i++)
    {
        fadeAll(4);
        int pos = random16(NUM_LEDS_PER_STRIP);
        CHSV hsv = CHSV(gHue + random8(64), 255, 255);

        addPixelColor(i, pos, hsv);
    }
}

// void theaterChase(byte red, byte green, byte blue, int SpeedDelay)
// { //3 on, 3 off chasing lights
//     if (currentMillis - previousMillis > SpeedDelay)
//     { // Time to update
//         previousMillis = currentMillis;
//         for (uint8_t i = 0; i < NUM_STRIPS; i++)
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

void fadeInOut(byte red, byte green, byte blue)
{
    float r, g, b;
    if (!isFading)
    { 
        if (gFade < 65)
        {
            gFade++;
        }
        else
        {
            gFade = 64;
            isFading = true;
        }
    }
    
    if (isFading)
    {
        if (gFade >= 0)
        {
            gFade -= 2;
        }
        else
        {
            gFade = 0;
            isFading = false;
        }
    }

    //64 ~ 1/4 brightness
    r = (gFade / 256.0) * red;
    g = (gFade / 256.0) * green;
    b = (gFade / 256.0) * blue;
    allColor(r, g, b);
}

void Sparkle(byte red, byte green, byte blue, int SpeedDelay)
{
    // int Pixel = random(NUM_LEDS_PER_STRIP);
    // setPixel(Pixel, red, green, blue);
    // showStrip();
    // delay(SpeedDelay);
    // setPixel(Pixel, 0, 0, 0);
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay)
{
    // //sin wave running lights
    // int Position = 0;
    // unsigned long currentMillis = millis();
    // // Time to update
    // //if (currentMillis - previousMillis > WaveDelay) {
    // //previousMillis = currentMillis;

    // for (int i = 0; i < NUM_LEDS_PER_STRIP * 2; i++)
    // {
    //     Position++; // = 0; //Position + Rate;
    //     for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
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

void paletteCycle()
{
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */

    FillLEDsFromPaletteColors(startIndex);
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

void doThingsToDifferentStrips()
{  
    for (uint8_t i = 0; i < NUM_UPPER_STRIPS; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_STRIP; j++)
        {
            CRGB rgb = CRGB(0, 0, 255);
            setPixelColor(i, j, rgb);
        }
    }
    for (uint8_t i = NUM_UPPER_STRIPS; i < NUM_TOTAL_STRIPS; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_STRIP; j++)
        {
            CRGB rgb = CRGB(0, 0, 255);
            setPixelColor(i, j, rgb);
        }
    }
}

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
    currentPalette = OceanColors_p;
    for (uint8_t i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_STRIP; j++)
        {
            CRGB rgb = ColorFromPalette(currentPalette, colorIndex, BRIGHTNESS, currentBlending);
            colorIndex += 1;

            setPixelColor(i, j, rgb);
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
void allColor(byte red, byte green, byte blue)
{
    for (uint8_t i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_STRIP; j++)
        {
            setPixelColor(i, j, CRGB(red, green, blue));
        }
    }
}

void allColor(CRGB color)
{
    allColor(color.red, color.green, color.blue);
}

void fadeAll(int factor)
{
    if (factor < 1) factor = 1;

    for (uint8_t i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < NUM_LEDS_PER_STRIP; j++)
        {
            uint32_t colorInt = innerLeds.getPixelColor((i * NUM_LEDS_PER_STRIP) + j);

            byte red = (colorInt >> 16) & 255;
            byte green = (colorInt >> 8) & 255;
            byte blue = colorInt & 255;

            CRGB rgb = CRGB(red, green, blue);
            rgb.fadeToBlackBy(factor);

            setPixelColor(i, j, rgb);
        }
    }
}

void setPixelColor(int row, int pos, CHSV hsv)
{   
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);
    
    setPixelColor(row, pos, rgb);
}

void setPixelColor(int row, int pos, CRGB color)
{
    innerLeds.setPixelColor((row * NUM_LEDS_PER_STRIP) + pos, color.red, color.blue, color.green);
}

void addPixelColor(int row, int pos, CHSV hsv)
{
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);

    addPixelColor(row, pos, rgb);
}

void addPixelColor(int row, int pos, CRGB rgb)
{
    rgb += getCRGB(row, pos);

    setPixelColor(row, pos, rgb);
}

CRGB getCRGB(int row, int pos)
{
    uint32_t colorInt = innerLeds.getPixelColor((row * NUM_LEDS_PER_STRIP) + pos);

    byte red = (colorInt >> 16) & 255;
    byte green = (colorInt >> 8) & 255;
    byte blue = colorInt & 255;

    return CRGB(red, green, blue);
}

void setOuterPixelColor(int pos, CRGB color)
{
    outerLeds.setPixelColor(pos, color.red, color.blue, color.green);
}