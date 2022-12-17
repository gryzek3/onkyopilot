

/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "WifiConfiguration.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DS18B20.h>

#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
const uint8_t touchPin = 16;      // GPIO 16 (D0)
const uint8_t touchPin2 = 4;      // GPIO 4 (D2)
const uint8_t lightPin = 0;       // GPIO 0  (D3)
const uint8_t thermometerPin = 5; // GPIO 5  (D1)
bool isLightOn = false;
uint8_t thermometer186Adr[8] = {40, 83, 73, 254, 160, 33, 1, 186};
uint8_t thermometer228Adr[8] = {40, 5, 156, 5, 161, 33, 1, 228};
DS18B20 ds(thermometerPin);
ESP8266WebServer server(80);

const int utcOffsetInSeconds = 3600 * 2;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 45000);

void switchLight()
{
  isLightOn = !isLightOn;

  digitalWrite(lightPin, isLightOn ? HIGH : LOW);
  Serial.println(isLightOn);
  delay(1500);
}

float get186Temp()
{
  float result = ds.getTempC();
  Serial.print("Temperature on 186: ");
  Serial.println(result);
  return result;
}
void setup()
{
  pinMode(touchPin, INPUT);
  pinMode(touchPin2, INPUT);
  pinMode(lightPin, OUTPUT);
  ds.select(thermometer186Adr);
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
  server.on("/light", handle_light);
  server.on("/temperature", handle_temperature);
  server.begin();
  timeClient.begin();
}
void loop()
{
  if (digitalRead(touchPin) == HIGH || digitalRead(touchPin2) == HIGH)
  {
    switchLight();
  }
  server.handleClient();
}

void handle_index()
{
  switchLight();
  sendResponseToClient();
}
void handle_light()
{
  switchLight();
  server.send(200, "text/html", getLightStatus());
}
void handle_temperature()
{
  String temp = "temp: ";
  temp = temp + get186Temp();
  server.send(200, "text/html", temp);
}
String getLightStatus()
{
  return isLightOn ? "ON" : "OFF";
}
void sendResponseToClient()
{
  String pageContent =
      "<br><br> 186 tehermometere temp : ";
  pageContent = pageContent + get186Temp();
  pageContent = pageContent + "<br><br> Light: " + getLightStatus();
  server.send(200, "text/html", pageContent);
}
