/*+===================================================================
  Template: ESP32 Project template with builtin OTA,
            HTTP Server, WiFi connectivity and About page.
            Only manual OTA updates (/update) are supported.

            This is specifcally for the wide/skinny
            oled display as seen in readme.md.

  File:      main.cpp

  Project:   <BangWorx>

  Summary:   Project testing for fireworks controller

             Architecture: ESP32 specific.

  LED Config:You must update secrets.h with your WiFi credentials
             and the hostname you choose for this device.
             Currently using Elegant OTA.

             Pre-deloyment configuration checklist:

                1. Set DATA_PIN, NUM_ROWS and NUM_COLS - ROWS=1 = single strip.
                2. Set <title> in htmlStrings.h and <h1> header on line 302
                3. Set MAX_CURRENT in milliamps and NUM_VOLTS (must match PSU used!).
                4. Set hostName in secrets.h
                5. Set ssid and password in secrets.h
                6. Enable USE_HARDWARE_INPUT if using an analog brightness knob (GPIO35).
                7. NEW: Set NUM_LEDS in Kanimations.h

  Hardware:  If using bright knob, color switch button, temp sensor, set USE_HARDWARE_INPUT 1
             Oled is always expected to be present.

             OLED: SSD1306 128x32. SDA=21, SC=22
             Bright Knob: Use a 10k Potentiometer to control the brightness
                          of the LEDs with .1 uf cap on lugs 1(gnd) and 2 (data).

             Temp Sensor: MLX90615. SDA=21, SCL=21 (same buss as oled)
             Color Button:
                                  ____
                            3v3<-|    |->NC
                                 | () |
                       GND<-10k<-|____|->DATA

             Bright Knob: Pin 1=GND, Pin 2=DATA, Pin 3=VCC

                                    |
                                  (___)
                                  | | |
                                  G D 3
                                  N T V
                                  D A 3

  Building:  pio run -t <target> -e envName

             Examples:
                pio run -t upload, monitor -e heltec_wifi_kit_32
                pio run -t upload, monitor -e fm-dev-kit
                pio run -t upload -e heltec_wifi_kit_32 --upload-port COM6
                pio run -t upload -e fm-dev-kit
                pio run -t upload -e fm-dev-kit --upload-port COM6

             List targets: pio run --list-targets

  PINS
            USE_HARDWARE_INPUT 1|0  : Enable brignt knob, temp meter, buttons etc.
            RND_PIN 34              : Random number seed from analog pin.
            BRITE_KNOB_PIN = 35     : Brightness knob.
            DATA_PIN = 5            : Data pin for the LED strip.
            COLOR_SELECT_PIN = 16   : Color selection button.
            HEAT_SCL_PIN = 22       : I2C SCL pin for temperature sensor.
            HEAT_SDA_PIN = 21       : I2C SDA pin for temperature sensor.
            OLED SCL = 22           : ESP builtin SCA/SCL pins, don't assign in code!
            OLED SCA = 21           : ESP builtin SCA/SCL pins, don't assign in code!
            FAN_PIN = 33            : PWM controlled heat fan pin.

  Kary Wall 2/20/2022.
===================================================================+*/

#define FASTLED_INTERNAL // Quiets build noise
#include <globalConfig.h>
#include <LEDController.h>
#include <Arduino.h>
#include "SPIFFS.h"
#include <zUtils.h>
#include <localWiFi.h>
#include <asyncWebServer.h>
#include <FastLED.h>
#include <oled.h>

// Template external and globals
extern CRGB leds[];
extern int gLeds[];
#if defined(esp32dev)
extern Adafruit_SSD1306 display;
#endif
extern void startWifi();
extern void startWebServer();
extern bool isWiFiConnected();
extern String ssid;
extern String rssi;
extern String globalIP;
extern int g_lineHeight;

// Project specific externs and globals


// Prototypes
String checkSPIFFS();
void printDisplayMessage(String msg);
uint8_t getBrigtnessLimit();
void checkBriteKnob();
float celsiusToFahrenheit(float c);

// Locals
const int activityLED = 12;
unsigned long lastUpdate = 0;

float EMA_a = 0.8; // Smoothing
int EMA_S = 0;     // Smoothing


void setup()
{
    /*--------------------------------------------------------------------
     Boot, OLED and I/O initialization.
    ---------------------------------------------------------------------*/
    Serial.begin(115200);
    Serial.println();
    Serial.println("Booting...");

#if defined(heltec_wifi_kit_32)
    /*--------------------------------------------------------------------
     Heltec OLED display initialization.
    ---------------------------------------------------------------------*/
    g_OLED.begin();
    g_OLED.clear();
    g_OLED.setFont(u8g2_font_profont15_tf);
    g_lineHeight = g_OLED.getFontAscent() - g_OLED.getFontDescent();
    g_OLED.clearBuffer();
    g_OLED.setCursor(0, g_lineHeight);
    g_OLED.printf("Connecting to WiFi...");
    g_OLED.sendBuffer();
#else
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
#endif

    /*--------------------------------------------------------------------
    Update oled every second
    ---------------------------------------------------------------------*/
#if defined(heltec_wifi_kit_32)
        g_OLED.clearBuffer();
        g_OLED.setCursor(0, g_lineHeight);
        g_OLED.printf("IP: %s", globalIP.c_str());
        g_OLED.setCursor(0, g_lineHeight * 2);
        g_OLED.printf("%s", hostName.c_str());
        g_OLED.setCursor(0, g_lineHeight * 3);
        g_OLED.printf("SSID: %s", ssid.c_str());
        g_OLED.setCursor(0, g_lineHeight * 4);
        g_OLED.printf("Connected: %s", String(isWiFiConnected()).c_str());
        g_OLED.sendBuffer();
        lastUpdate = millis();
#else
    printDisplayMessage("Boot...");
#endif

    zUtils::getChipInfo();
    pinMode(activityLED, OUTPUT);
    digitalWrite(activityLED, LOW);

    /*--------------------------------------------------------------------
     Start WiFi & OTA HTTP update server
    ---------------------------------------------------------------------*/
    printDisplayMessage("Wifi...");
    startWifi();
    printDisplayMessage("Server...");
    startWebServer();

    /*--------------------------------------------------------------------
     Project specific setup code
    ---------------------------------------------------------------------*/

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(NUM_VOLTS, MAX_CURRENT);
    FastLED.setBrightness(180);
    FastLED.setCorrection(Halogen);
    pinMode(RND_PIN, INPUT);
    randomSeed(analogRead(RND_PIN));
    FastLED.clear();
    FastLED.show();

    // pot smoothing
    EMA_S = analogRead(BRITE_KNOB_PIN);

}

void printDisplayMessage(String msg)
{
    #if defined(heltec_wifi_kit_32)
        g_OLED.clearBuffer();
        g_OLED.setCursor(0, g_lineHeight);
        g_OLED.printf(msg.c_str());
        g_OLED.sendBuffer();
    #else
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println(msg);
        display.display();
    #endif
}

void printDefaultStatusMessage()
{
    #if defined(heltec_wifi_kit_32)
        if (millis() - lastUpdate > 1000){
            g_OLED.clearBuffer();
            g_OLED.setCursor(0, g_lineHeight);
            g_OLED.printf("IP: %s", globalIP.c_str());
            g_OLED.setCursor(0, g_lineHeight * 2);
            g_OLED.printf("%s", hostName.c_str());
            g_OLED.setCursor(0, g_lineHeight * 3);
            g_OLED.printf("SSID: %s", ssid.c_str());
            g_OLED.setCursor(0, g_lineHeight * 4);
            g_OLED.printf("Connected: %s", String(isWiFiConnected()).c_str());
            g_OLED.sendBuffer();
            lastUpdate = millis();
        }
    #endif
}
void loop()
{
    printDefaultStatusMessage();

    /*--------------------------------------------------------------------
     Project specific loop code
     ---------------------------------------------------------------------*/

    delay(100); //  inhale
}

/*--------------------------------------------------------------------
     Project specific utility code (otherwise use zUtils.h)
---------------------------------------------------------------------*/

String checkSPIFFS()
{
    // Mount SPIFFS if we are using it
    if (!SPIFFS.begin(true))
    {
        return "An Error has occurred while mounting SPIFFS";
    }
    else
    {
        return "SPIFFS mounted OK.";
    }
}