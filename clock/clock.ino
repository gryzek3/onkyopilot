

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
#include <LiquidCrystal.h>
LiquidCrystal lcd(16, 5, 4, 0, 2, 14);

#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
const uint8_t thermometerPin = 12; // GPIO 12  (D7)
uint8_t thermometer186Adr[8] = {40, 83, 73, 254, 160, 33, 1, 186};
uint8_t thermometer228Adr[8] = {40, 5, 156, 5, 161, 33, 1, 228};
DS18B20 ds(thermometerPin);
ESP8266WebServer server(80);

const int utcOffsetInSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 45000);
float get186Temp()
{
  float result = ds.getTempC();
  Serial.print("Temperature on 186: ");
  Serial.println(result);
  return result;
}
void setup()
{
  ds.select(thermometer228Adr);
  lcd.begin(16, 2); // Deklaracja typu
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
}
void loop()
{

  server.handleClient();
  String temp = "temp: ";
  temp = temp + get186Temp();
  lcd.setCursor(0, 0); // Ustawienie kursora
  lcd.print(temp);     // Wyświetlenie tekstu
  lcd.setCursor(0, 1); // Ustawienie kursora
  timeClient.update();
  String time = timeClient.getFormattedTime();
  lcd.print(time); // Wyświetlenie tekstu
}

void handle_index()
{
  sendResponseToClient();
}

void handle_temperature()
{
  String temp = "temp: ";
  temp = temp + get186Temp();
  server.send(200, "text/html", temp);
}

void sendResponseToClient()
{
  String pageContent =
      "<br><br> 186 tehermometere temp : ";
  pageContent = pageContent + get186Temp();
  server.send(200, "text/html", pageContent);
}
