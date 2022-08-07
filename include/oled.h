/*+===================================================================
  File:      oled dot h

  Summary:   Used for OLED display.

  Kary Wall 1/20/2022.
===================================================================+*/

#if defined(heltec_wifi_kit_32)
    #include <U8g2lib.h>
    #define OLED_CLOCK 15 // Pins for the OLED display
    #define OLED_DATA 4
    #define OLED_RESET 16
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C g_OLED(U8G2_R2, OLED_RESET, OLED_CLOCK, OLED_DATA);
#else
    #include <Wire.h>
    #include <Adafruit_I2CDevice.h>
    #include <Adafruit_SSD1306.h>
    #include <Adafruit_GFX.h>
    #define OLED_WIDTH 128
    #define OLED_HEIGHT 32
    #define OLED_ADDR 0x3C
    Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);
#endif


int g_lineHeight = 0;
