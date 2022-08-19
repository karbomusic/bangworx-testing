/*+===================================================================
  File:      localWiFi.h

  Summary:   This is the WiFi client that gets us connected.
             localUpdateServer.h depends on this header file.

  Kary Wall 2022.
===================================================================+*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <secrets.h>

// externs (from secrets.h)
extern String ssid;            // WiFi ssid
extern String password;        // WiFi password
extern String hostName;        // hostname as seen on network
extern String softwareVersion; // used for OTA updates & about page
extern String deviceFamily;    // used for OTA updates & about page
extern String description;     // used for about page
extern String globalIP;        // needed for about page
extern String softap_ssid;
extern String softap_password;

void startSoftAP()
{
    WiFi.softAP(softap_ssid.c_str(), softap_password.c_str());
    globalIP = WiFi.softAPIP().toString();
    Serial.println("SoftAP IP: " + globalIP);
}

void startWifi()
{
    if (g_isAccessPoint)
    {
        startSoftAP();
    }
    else
    {
        password = softap_password;
        ssid = softap_ssid;
    }

    if (!g_isAccessPoint)
    {
        // Connect to WiFi network
        Serial.print("SSID: ");
        Serial.println(ssid);
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
        WiFi.setHostname(hostName.c_str());
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.println("");

        // Wait for WiFi connection...
        int timeout = 45;
        Serial.print("Connecting to WiFi...");

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.println("--> " + String(ssid) + " | " + WiFi.RSSI() + " dBm " + "| Timeout: " + String(timeout));
            timeout--;
            if (timeout == 0)
            {
                Serial.println("");
                Serial.println("WiFi connection timed out, restarting...");
                Serial.println("");
                ESP.restart();
                break;
            }
        }

        // We're connected...
        WiFi.setHostname(hostName.c_str());
        WiFi.hostname(hostName.c_str());
        Serial.println("\n-------------------------------------");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("SoftAP IP: ");
        Serial.println(WiFi.softAPIP().toString());
        Serial.print("MAC address: ");
        Serial.println(WiFi.macAddress());
        Serial.print("Hostname: ");
        Serial.println(WiFi.getHostname());
        Serial.println("Device Family: " + deviceFamily);
        Serial.println("Chip ID:" + String(zUtils::getChipID()));
        Serial.println("-------------------------------------\n");
        globalIP = WiFi.localIP().toString();
    }
    else
    {
        // It's an access point (master controller), set the IP manually since we didn't connect to WiFi
        WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); // subnet FF FF FF 00
        WiFi.softAPsetHostname(hostName.c_str());
        Serial.println("\n-------------------------------------");
        Serial.println("SoftAP started");
        Serial.print("SoftAP IP: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("MAC address: ");
        Serial.println(WiFi.softAPmacAddress());
        Serial.print("Hostname: ");
        Serial.println(WiFi.softAPgetHostname());
        Serial.println("Device Family: " + deviceFamily);
        Serial.println("Chip ID:" + String(zUtils::getChipID()));
        Serial.println("-------------------------------------\n");
        globalIP = WiFi.softAPIP().toString();
    }

    // use mdns for host name resolution
    if (!MDNS.begin(hostName.c_str()))
    {
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        }
    }

    Serial.println("mDNS responder started...");
    mdns_hostname_set(hostName.c_str());

}

bool isWiFiConnected()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int getConnectedClientCount(){
    return WiFi.softAPgetStationNum();
}