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
#endif
#include "Adafruit_VL53L0X.h"

#define DEBUG true

#define NUM_UPPER_STRIPS 4
#define NUM_LOWER_STRIPS 4
#define NUM_OUTER_STRIPS 1
#define NUM_INNER_STRIPS (NUM_UPPER_STRIPS + NUM_LOWER_STRIPS)
#define NUM_LEDS_PER_OUTER_STRIP 4
#define NUM_LEDS_PER_INNER_STRIP 161

#define NUM_LASERS 3
#define LASER_BRIGHTNESS_XSHUT_PIN 10 // int value for the Brightness laser_L pot
#define LASER_HUE_XSHUT_PIN 11        // int value for the Hue laser_C pot
#define LASER_SPEED_XSHUT_PIN 12      // int value for the Speed laser_R pot
#define INNER_ANIM_PIN 6              // select the input pin for the button
#define OUTER_ANIM_PIN A5             // select the input pin for the button

uint32_t sensorValue_B = 0; // variable to store the value coming from the Brightness sensor
uint32_t sensorValue_H = 0; // variable to store the value coming from the Hue sensor
uint32_t sensorValue_S = 0; // variable to store the value coming from the Speed sensor

uint16_t gFps = 70;
uint8_t gHue = 0;          // rotating "base color" used by many of the patterns
uint8_t gBrightness = 200; //127 ~ 1/2 brightness [0-256]
byte gRed = 100;
byte gBlue = 100;
byte gGreen = 100;
uint32_t gFade = 32;
bool isFading = false;
CRGBPalette16 currentPalette;
TBlendType currentBlending;

// uint8_t buf[3],       // Enough for RGB parse; expand if using sensors
//     prevTime = 0L;    // For animation timing
bool canChangeInnerState = true;
bool canChangeOuterState = true;
int buttonStateInner = LOW; // current state of button. HIGH == ColorPicker
int buttonStateOuter = LOW; // current state of button. HIGH == ColorPicker

int innerAnimationIndex = 0; //Which animation is displaying
int outerAnimationIndex = 0; //Which animation is displaying
int paletteIndex = 0;
long previousMillis = 0; // Will store last time button was updated
uint32_t currentMillis = millis();
int8_t pins[8] = {PIN_SERIAL1_RX, PIN_SERIAL1_TX, MISO, 13, 5, MOSI, A4, A3}; //Pins switched from default!! Hardware dependant!! https://learn.adafruit.com/adafruit-neopxl8-featherwing-and-library/neopxl8-arduino-library
//int8_t upperPins[4] = {PIN_SERIAL1_RX, PIN_SERIAL1_TX, MISO, 13};
//int8_t lowerPins[4] = {5, MOSI, A4, A3};
Adafruit_NeoPXL8 innerLeds(NUM_LEDS_PER_INNER_STRIP, pins, NEO_GRB);
//Adafruit_NeoPXL8 upperLeds(NUM_LEDS_PER_INNER_STRIP, upperPins, NEO_GRB);
//Adafruit_NeoPXL8 lowerLeds(NUM_LEDS_PER_INNER_STRIP, lowerPins, NEO_GRB);
Adafruit_NeoPixel outerLeds = Adafruit_NeoPixel(NUM_LEDS_PER_OUTER_STRIP, 9, NEO_GRB);

Adafruit_VL53L0X loxL_brightness = Adafruit_VL53L0X();
Adafruit_VL53L0X loxC_hue = Adafruit_VL53L0X();
Adafruit_VL53L0X loxR_speed = Adafruit_VL53L0X();
int8_t laserPins[3] = {10, 11, 12};
/*****************************************************************************/

/*******************************SETUP********************************************************************/
void setup(void)
{
    pinMode(INNER_ANIM_PIN, INPUT_PULLUP);
    pinMode(OUTER_ANIM_PIN, INPUT_PULLUP);

    innerLeds.begin();
    innerLeds.setBrightness(gBrightness); //78% Brightness (range: 1-256)

    outerLeds.begin();
    outerLeds.setBrightness(gBrightness); //78% Brightness (range: 1-256)

    //upperLeds.begin();
    //upperLeds.setBrightness(gBrightness); //78% Brightness (range: 1-256)

    //lowerLeds.begin();
    //lowerLeds.setBrightness(gBrightness); //78% Brightness (range: 1-256)

    Serial.begin(115200);

    // wait until serial port opens for native USB devices
    while (!Serial)
    {
        delay(1);
    }
    Serial.println("Started Serial");

    pinMode(LASER_BRIGHTNESS_XSHUT_PIN, OUTPUT);
    pinMode(LASER_HUE_XSHUT_PIN, OUTPUT);
    pinMode(LASER_SPEED_XSHUT_PIN, OUTPUT);

    //Reset the lasers
    for (uint8_t i = 0; i < NUM_LASERS; i++)
    {
        digitalWrite(laserPins[i], LOW);
    }
    delay(10);
    for (uint8_t i = 0; i < NUM_LASERS; i++)
    {
        digitalWrite(laserPins[i], HIGH);
    }

    //Turn on one laser ata time
    //Left ON
    digitalWrite(LASER_HUE_XSHUT_PIN, LOW);
    digitalWrite(LASER_SPEED_XSHUT_PIN, LOW);
    delay(10);
    Serial.println("Adafruit VL53L0X_L test");
    if (!loxL_brightness.begin(0x30))
    {
        Serial.println(F("Failed to boot VL53L0X_LEFT"));
        while (1)
            ;
    }
    else
        (Serial.println("Adafruit VL53L0X_L CONNECTED"));

    //Center ON
    digitalWrite(LASER_HUE_XSHUT_PIN, HIGH);
    delay(10);
    Serial.println("Adafruit VL53L0X_C test");
    if (!loxC_hue.begin(0x31))
    {
        Serial.println(F("Failed to boot VL53L0X_CENTER"));
        while (1)
            ;
    }
    else
        (Serial.println("Adafruit VL53L0X_C CONNECTED"));

    //Right ON
    digitalWrite(LASER_SPEED_XSHUT_PIN, HIGH);
    delay(10);
    Serial.println("Adafruit VL53L0X_R test");
    if (!loxR_speed.begin(0x32))
    {
        Serial.println(F("Failed to boot VL53L0X_RIGHT"));
        while (1)
            ;
    }
    else
        (Serial.println("Adafruit VL53L0X_R CONNECTED"));

    // I2C Scanner
    // Written by Nick Gammon
    // Date: 20th April 2011
    Serial.println();
    Serial.println("I2C scanner. Scanning ...");
    byte count = 0;

    Wire.begin();
    for (byte i = 8; i < 120; i++)
    {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("Found address: ");
            Serial.print(i, DEC);
            Serial.print(" (0x");
            Serial.print(i, HEX);
            Serial.println(")");
            count++;
            delay(1); // maybe unneeded?
        }             // end of good response
    }                 // end of for loop
    Serial.println("Done.");
    Serial.print("Found ");
    Serial.print(count, DEC);
    Serial.println(" device(s).");
    //--------------Scan complete-----------------
}

/******************************MAIN LOOP********************************************/

void loop(void)
{
    //LaserPots();
    // Get current button state.
    buttonStateInner = digitalRead(INNER_ANIM_PIN);
    buttonStateOuter = digitalRead(OUTER_ANIM_PIN);

    // Check if state changed from high to low (button press).
    if (buttonStateInner == LOW && canChangeInnerState)
    {
        Serial.println("Pressing inner button...");
        innerLeds.clear();
        innerAnimationIndex++;
        canChangeInnerState = false;
    }
    else if (buttonStateInner == HIGH && !canChangeInnerState)
    {
        Serial.println("Changing inner button state...");
        canChangeInnerState = true;
    }

    if (buttonStateOuter == LOW && canChangeOuterState)
    {
        Serial.println("Pressing outer button...");
        outerLeds.clear();
        outerAnimationIndex++;
        if (gFade == 0)
        {
            gFade = 255;
        }
        else if (gFade == 255)
        {
            gFade = 127;
        }
        else if (gFade == 127)
        {
            gFade = 32;
        }
        else if (gFade == 32)
        {
            gFade = 0;
        }
        canChangeOuterState = false;
    }
    else if (buttonStateOuter == HIGH && !canChangeOuterState)
    {
        Serial.println("Changing outer button state...");
        canChangeOuterState = true;
    }

    // Shift the hue over time.
    EVERY_N_MILLISECONDS(60)
    {
        gHue++;
    }

    /*ifdef DEBUG
      // For easier pattern testing.
      EVERY_N_SECONDS(30)
      {
          innerLeds.clear();
          innerAnimationIndex++;
      }
    #endif*/

    showInnerAnimation();
    showOuterAnimation();

    innerLeds.show();
    outerLeds.show();

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
        bpm2(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
        break;
    case 2:
        rainbow(gHue, 7, NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
        break;
    case 3:
        confetti(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
        break;
    case 4:
        sinelon(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
        break;
    case 5:
        juggle(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
        break;
    case 6:
        paletteCycle(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
        break;
    case 7:
        // Fire - (NUM_INNER STRIPS, isInner, Cooling rate, Sparking rate, speed delay
        fire(NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true, 55, 120, 15);
        //Fire only shows on strip 2 but in red, green, and blue instead of the red, orange, yellow heat palette from the func.
        //Small amount of red at the begining of strip5 as well. think the button adjustable
        break;
    case 8:
        // SnowSparkle - Color (red, green, blue), sparkle delay, speed delay
        snowSparkle(16, 16, 16, 20, random(100, 1000), NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
        //No idea why but the sparkle only happens on strips 1,4,& 5 ?!
        break;
    case 9:
        // meteorRain - Color (red, green, blue), meteor size, trail decay, random trail decay (true/false), speed delay
        meteorRain(255, 255, 255, 10, 64, true, 30);
        break;
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
        sparkle(0, 40, 80, 100, NUM_OUTER_STRIPS, NUM_LEDS_PER_OUTER_STRIP, false);
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
    VL53L0X_RangingMeasurementData_t measure;
    EVERY_N_MILLISECONDS(100)
    {

        Serial.println("Reading a measurement... ");

        loxL_brightness.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

        if (measure.RangeStatus != 4)
        { // phase failures have incorrect data
            Serial.print("LEFT_Distance (mm): ");
            Serial.println(measure.RangeMilliMeter);
        }
        else
        {
            Serial.println(" out of range ");
        }

        loxC_hue.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

        if (measure.RangeStatus != 4)
        { // phase failures have incorrect data
            Serial.print("CENTER_Distance (mm): ");
            Serial.println(measure.RangeMilliMeter);
        }
        else
        {
            Serial.println(" out of range ");
        }

        loxR_speed.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

        if (measure.RangeStatus != 4)
        { // phase failures have incorrect data
            Serial.print("RIGHT_Distance (mm): ");
            Serial.println(measure.RangeMilliMeter);
        }
        else
        {
            Serial.println(" out of range ");
        }

        //return(loxL_brightness.RangeMilliMeter, loxC_hue.RangeMilliMeter, loxR_speed.RangeMilliMeter);
    }
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
        for (int j = 0; j < numLedsPerStrip; j++)
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

void bpm2(uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 100;
    CRGBPalette16 palette = LavaColors_p;

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
    fadeAll(gFade, numStrips, numLedsPerStrip, isInner);
    // a colored dot sweeping back and forth, with fading trails
    for (uint8_t i = 0; i < numStrips; i++)
    {
        int pos = beatsin16(13, 0, numLedsPerStrip - 1);
        CRGB rgb = getCRGB(i, pos, isInner);
        rgb += CHSV(gHue, 255, 192);

        setPixelColor(i, pos, rgb, isInner);
    }
}

void juggle(uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    fadeAll(gFade, numStrips, numLedsPerStrip, isInner);
    // eight colored dots, weaving in and out of sync with each other
    for (uint8_t i = 0; i < numStrips; i++)
    {
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
    fadeAll(gFade, numStrips, numLedsPerStrip, isInner);
    // random colored speckles that blink in and fade smoothly
    for (uint8_t i = 0; i < numStrips; i++)
    {
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

void sparkle(byte red, byte green, byte blue, int speedDelay, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    for (uint8_t i = 0; i < numStrips; i++)
    {
        int8_t pos = random(numLedsPerStrip - 1);
        CRGB rgb = CRGB(red, green, blue);
        setPixelColor(i, pos, rgb, isInner);

        EVERY_N_MILLISECONDS(speedDelay)
        {
            CRGB rgb = CRGB(0, 0, 0);
            setPixelColor(i, pos, rgb, isInner);
        }
    }
}

void snowSparkle(byte red, byte green, byte blue, int sparkleDelay, int speedDelay, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    allColor(red, green, blue, numStrips, numLedsPerStrip, isInner);

    fadeAll(gFade, numStrips, numLedsPerStrip, isInner);
    for (uint8_t i = 0; i < numStrips; i++)
    {
        int8_t pos = random(numLedsPerStrip - 1);

        EVERY_N_MILLISECONDS(sparkleDelay)
        {
            setPixelColor(i, pos, CRGB(255, 255, 255), isInner);
        }

        EVERY_N_MILLISECONDS(speedDelay)
        {
            setPixelColor(i, pos, CRGB(red, green, blue), isInner);
        }
    }
}

void runningLights(byte red, byte green, byte blue, int WaveDelay, uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner)
{
    //sin wave running lights
    int Position = 0;
    unsigned long currentMillis = millis();
    // Time to update
    if (currentMillis - previousMillis > WaveDelay)
    {
        previousMillis = currentMillis;
        //EVERY_N_MILLISECONDS(WaveDelay){

        for (int8_t n = 0; n < numStrips; n++)
        {
            for (int i = 0; i < numLedsPerStrip * 2; i++)
            {
                Position++; // = 0; //Position + Rate;
                for (int i = 0; i < numLedsPerStrip; i++)
                {
                    // sine wave, 3 offset waves make a rainbow!
                    //float level = sin(i + Position) * 127 + 128;
                    //CRGB rgb = CRGB(level, 0, 0);
                    //innerLeds.setPixelColor(n, i, rgb, isInner);
                    //float level = sin(i + Position) * 127 + 128;
                    CRGB rgb = CRGB(((sin(i + Position) * 127 + 128) / 255) * red,
                                    ((sin(i + Position) * 127 + 128) / 255) * green,
                                    ((sin(i + Position) * 127 + 128) / 255) * blue);
                    innerLeds.setPixelColor(n, i, rgb, isInner);
                }
            }
        }
    }
}

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay)
{
    if ((!meteorRandomDecay) || (random(10) > 5))
    {
        fadeAll(gFade, NUM_INNER_STRIPS, NUM_LEDS_PER_INNER_STRIP, true);
    }

    for (uint8_t i = NUM_UPPER_STRIPS; i < NUM_INNER_STRIPS; i++)
    {     
        for (int j = 0; j < NUM_LEDS_PER_INNER_STRIP * 2; j++)
        {
            // draw meteor
            for (int k = 0; k < meteorSize; k++)
            {
                if ((j - k < NUM_LEDS_PER_INNER_STRIP) && (j - k >= 0))
                {
                    CRGB rgb = CRGB(red, green, blue);

                    //if (currentMillis - previousMillis > SpeedDelay)     { // Time to update
                    //previousMillis = currentMillis;
                    EVERY_N_MILLISECONDS(SpeedDelay)
                    {
                        setPixelColor(i, j - k, rgb, true);
                    }
                }
            }
        }
    }
}

void fire(uint8_t numStrips, uint8_t numLedsPerStrip, bool isInner, int Cooling, int Sparking, int SpeedDelay)
{
    byte heat[numLedsPerStrip];
    int cooldown;

    // Step 0. Fade everything just a little bit
    fadeAll(gFade, numStrips, numLedsPerStrip, isInner);

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < numLedsPerStrip; i++)
    {
        cooldown = random(0, ((Cooling * 10) / numLedsPerStrip) + 2);

        if (cooldown > heat[i])
        {
            heat[i] = 0;
        }
        else
        {
            heat[i] = heat[i] - cooldown;
        }
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = numLedsPerStrip - 1; k >= 2; k--)
    {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' near the bottom
    if (random(255) < Sparking)
    {
        int y = random(7);
        heat[y] = heat[y] + random(160, 255);
        //heat[y] = random(160,255);
    }

    // Step 4.  Convert heat to LED colors
    for (int i = 0; i < numStrips; i++)
    {
        for (int j = 0; j < numLedsPerStrip; j++)
        {
            CRGB heatColor = getHeatRGB(heat[j]);
            setPixelColor(i, j, heatColor, isInner);
        }
    }
}

CRGB getHeatRGB(byte temperature)
{
    // Scale 'heat' down from 0-255 to 0-191
    byte temp = round((temperature / 255.0) * 191);

    // calculate ramp up from
    byte heatRamp = temp & 0x3F; // 0..63
    heatRamp <<= 2;              // scale up to 0..252

    // figure out which third of the spectrum we're in:
    if (temp > 128)
    { 
        // hottest
        return CRGB(255, 255, heatRamp);
    }
    else if (temp > 64)
    { 
        // middle
        return CRGB(255, heatRamp, 0);
    }
    else
    { 
        // coolest
        return CRGB(heatRamp, 0, 0);
    }
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
    if (factor == 0)
        return;

    if (factor < 1)
        factor = 1;

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