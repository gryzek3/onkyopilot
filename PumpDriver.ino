

/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "WifiConfiguration.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#define STAONKOYOHOSTIP ""
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

const char *host = STAONKOYOHOSTIP;
const uint16_t port = 60128;

const uint8_t ledPin = 16;  // GPIO 16 (D0)
const uint8_t pump1Pin = 0; // GPIO 0  (D3)
const uint8_t pump2Pin = 5; // GPIO 5  (D1)
ESP8266WebServer server(80);

const int utcOffsetInSeconds = 3600 * 2;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 45000);

void ledOn()
{
  digitalWrite(ledPin, LOW);
}
void ledOff()
{
  digitalWrite(ledPin, HIGH);
}
void setup()
{
  pinMode(ledPin, OUTPUT);
  ledOn();
  Serial.begin(115200);
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handle_index);
  server.begin();
  timeClient.begin();
  ledOff();
}
void loop()
{
  server.handleClient();
  watering();
}

void handle_index()
{
  sendResponseToClient("index");
}
void sendResponseToClient(char *executedAction)
{

  String pageContent = "<ul style=\"font-size: 40Px;zoom: 200%;\"><li><a href =\"net\">NET</a></li><li><a href =\"tv\">TV</a></li><li><a href =\"off\">OFF</a></li><li><a href =\"tvoff\">TV OFF/ON</a></li><li><a href =\"source\">Change SOURCE</a></li><li> </li><li><a href =\"pumpOn?pumpId=1\">PUMP 1 ON</a></li><li><a href =\"pumpOn?pumpId=2\">PUMP 2 ON</a></li><li><a href =\"pumpOff?pumpId=1\">PUMP 1 OFF</a><li><a href =\"pumpOff?pumpId=2\">PUMP 2 OFF</a></li></ul>";
  pageContent = pageContent +
                "<br><br>wateringHour: ";
  server.send(200, "text/html", pageContent);
}
