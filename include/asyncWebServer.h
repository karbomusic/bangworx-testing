/*+===================================================================
  File:      asyncWebServer dot h

  Summary:   Provides an Async HTTP server and HTTPUploader for OTA
             updates, manual only via Elegant OTA:
             https://github.com/ayushsharma82/ElegantOTA

  Kary Wall 2/20/22.
===================================================================+*/

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <htmlStrings.h>

// externs
extern String ssid;               // WiFi ssid.
extern String hostName;           // hostname as seen on network.
extern String softwareVersion;    // used in about page and your custom needs.
extern String deviceFamily;       // used in about page and your custom needs.
extern String description;        // used in about page and your custom needs.
extern String globalIP;           // used in about page.
extern const String metaRedirect; // used for restart redirect.
extern const int activityLED;

// Prototypes
void handleAbout(AsyncWebServerRequest *request);
void bangLED(int);
void handleRestart(AsyncWebServerRequest *request);
void listAllFiles();
String getControlPanelHTML();

// locals
String controlPanelHtml;
AsyncWebServer server(80);

// globals

// incoming parameters
const char *MY_PARAM = "myParam";

// Replaces placeholder with section in your web page
String processor(const String &var)
{
    // not working becasue not sure how it works yet
    if (var == "[TITLE]")
    {
        return hostName;
    }
    return String();
}

void startWebServer()
{
    //----------------------------------------------------------------------------------------------------------------------
    // HTTP Requests and Responses
    //----------------------------------------------------------------------------------------------------------------------

    Serial.println("mDNS responder started");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {request->send(200, "text/plain", "Default Page");});

    server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request)
            {handleRestart(request);});

    server.on("/about", HTTP_GET, [](AsyncWebServerRequest *request)
            {handleAbout(request); });

    server.onNotFound([](AsyncWebServerRequest *request)
            {request->send(404, "text/plain", "404 - Not found"); });


   //----------------------------------------------------------------------------------------------------------------------
   // Start ElegantOTA update server and web server
   //----------------------------------------------------------------------------------------------------------------------
    AsyncElegantOTA.begin(&server); 
    Serial.println("Update server started! Open your browser and go to http://" + globalIP + "/update");
    Serial.println("or http://" + hostName + "/update");

    server.begin(); 
    Serial.println("HTTP server started! Open your browser and go to http://" + globalIP);
    Serial.println("or http://" + hostName);

    // listAllFiles(); //List SPIFF files
}

void handleRestart(AsyncWebServerRequest *request)
{
    Serial.println("Restarting in 5 seconds...");
    request->send(200, "text/html", metaRedirect);
    delay(5000);
    ESP.restart();
}

void bangLED(int state)
{
    digitalWrite(activityLED, state);
}

// return the about page as HTML.
void handleAbout(AsyncWebServerRequest *request)
{
    bangLED(HIGH);
    String aboutResponse = "<head><style type=\"text/css\">.button:active{background-color:#cccccc;color:#111111}></style><head>"
                           "<meta http-equiv=\"refresh\" content=\"10\"/>"
                           "<body style=\"background-color:#333333;color:#dddddd;font-family:arial\"><b>[About BangWorx Server]</b><br><br>"
                           "<b>Device Family:</b> " +
                           deviceFamily + "<br>"
                                          "<b>ESP Chip Model:</b> " +
                           String(ESP.getChipModel()) + "<br>"
                                                        "<b>CPU Frequency:</b> " +
                           String(ESP.getCpuFreqMHz()) + "<br>"
                                                         "<b>Free Heap Mem:</b> " +
                           String(ESP.getFreeHeap()) + "<br>"
                                                       "<b>Flash Mem Size:</b> " +
                           String(ESP.getFlashChipSize() / 1024 / 1024) + " MB<br>"
                                                                          "<b>Chip ID: </b>" +
                           String(zUtils::getChipID()) + "<br>"
                                                         "<b>Hostname:</b> " +
                           hostName + "<br>"
                                      "<b>IPAddress:</b> " +
                           globalIP + "<br>"
                                      "<b>MAC Address:</b> " +
                           String(WiFi.macAddress()) + "<br>"
                                                       "<b>SSID: </b> " +
                           ssid + "<br>"
                                  "<b>RSSI: </b> " +
                           String(WiFi.RSSI()) + " dB<br>"
                                                 "<b>Software Version:</b> " +
                           softwareVersion + "<br>"
                                             "<b>Description:</b> " +
                           description + "<br>"
                                         "<b>Uptime:</b> " +
                           zUtils::getMidTime() + "<br>"
                                                  "<b>Temperature:</b> " +
                           g_temperature + "<br>"
                                           "<b>Update:</b> http://" +
                           hostName + ".ra.local/update<br><br>"
                                      "<button class=\"button\" style=\"width:100px;height:30px;border:0;background-color:#3c5168;color:#dddddd\" onclick=\"window.location.href='/restart'\">Restart</button></body>"
                                      "&nbsp;&nbsp;<button class=\"button\" style=\"width:100px;height:30px;border:0;background-color:#3c5168;color:#dddddd\" onclick=\"window.location.href='/update'\">Update</button></body>";
    request->send(200, "text/html", aboutResponse);
    bangLED(LOW);
}