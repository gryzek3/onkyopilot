

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
const uint8_t ledPin = 16;        // GPIO 16 (D0)
const uint8_t pumpPin = 0;        // GPIO 0  (D3)
const uint8_t thermometerPin = 5; // GPIO 5  (D1)
uint8_t thermometer186Adr[8] = {40, 83, 73, 254, 160, 33, 1, 186};
uint8_t thermometer228Adr[8] = {40, 5, 156, 5, 161, 33, 1, 228};
DS18B20 ds(thermometerPin);
bool isPumpOn = false;
float turnOffTempDifference = 5;
float expectedTurnOffTemp = 0;
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
void pumpOn()
{
  digitalWrite(pumpPin, HIGH);
  isPumpOn = true;
  ledOn();
  Serial.println("Pump On");
  expectedTurnOffTemp = get186Temp() + turnOffTempDifference;
}
void pumpOff()
{
  digitalWrite(pumpPin, LOW);
  isPumpOn = false;
  ledOff();
  Serial.println("Pump OFF");
  expectedTurnOffTemp = 0;
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
  pinMode(ledPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  ds.select(thermometer186Adr);
  pumpOff();
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
  server.on("/pumpon", handle_PumpOn);
  server.on("/pumpoff", handle_PumpOff);
  server.begin();
  timeClient.begin();
  ledOff();
}
void loop()
{
  server.handleClient();
  TurnOffIfConditionsMet();
}
void TurnOffIfConditionsMet()
{
  if (expectedTurnOffTemp == 0)
  {
    return;
  }
  if (get186Temp() > expectedTurnOffTemp)
  {
    pumpOff();
  }
}
void handle_index()
{
  sendResponseToClient("index");
}
void handle_PumpOff()
{
  pumpOff();
  sendResponseToClient("pumpoff");
}
void handle_PumpOn()
{
  pumpOn();
  sendResponseToClient("pumpOn");
}
void sendResponseToClient(char *executedAction)
{

  String pageContent = "<ul style=\"font-size: 40Px;zoom: 200%;\"><li><a href =\"pumpon\">pump on</a></li><li><a href =\"pumpoff\">Pump OFF</a></li></ul>";
  pageContent = pageContent +
                "<br><br> PUMP IS ON : " + isPumpOn +
                "<br><br> 186 tehermometere temp : " + get186Temp();
  server.send(200, "text/html", pageContent);
}
