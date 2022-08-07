/*+===================================================================
  File:      secrets dot h

  Summary:   Wifi SSID and Credentials.

  Kary Wall 2/26/2022.
===================================================================+*/

#include <Arduino.h>

#ifndef STASSID
#define STASSID "IOTNET2G"
#define STAPSK "creekvalley124"
#endif

String ssid = STASSID;                         // WiFi ssid
String password = STAPSK;                      // WiFi password
String softap_ssid = "BANGWORKS-SVR";          // WiFi softap ssid
String softap_password = "password";           // WiFi softap password