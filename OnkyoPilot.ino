

/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "WifiConfiguration.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#define STAONKOYOHOSTIP ""
#endif
byte message[] = {
    0x49, 0x53, 0x43, 0x50,
    0x00, 0x00, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x08, // replace last with length
    0x01, 0x00, 0x00, 0x00,
    0x21, 0x31, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0D // 49
                           // Add data + EOF here
};
byte pwrOn[] = {0x50, 0x57, 0x52, 0x30, 0x31};
byte pwrOff[] = {0x50, 0x57, 0x52, 0x30, 0x30};
byte pwrSelectNet[] = {0x53, 0x4C, 0x49, 0x32, 0x39};
byte pwrSelectTv[] = {0x53, 0x4C, 0x49, 0x32, 0x30};
byte volume30[] = {0x4D, 0x56, 0x4C, 0x34, 0x30};
byte volume50[] = {0x4D, 0x56, 0x4C, 0x37, 0x30};
byte volumeUp[] = {0x4D, 0x56, 0x4C, 0x55, 0x50};
byte volumeDown[] = {0x4D, 0x56, 0x4C, 0x44, 0x4F, 0x57, 0x4E};

const uint32_t sourceCode = 0xE0E0807F;
const uint32_t onCode = 0xE0E040BF;
const char *ssid = STASSID;
const char *password = STAPSK;

const char *host = STAONKOYOHOSTIP;
const uint16_t port = 60128;

const uint8_t ledPin = 2;    // GPIO 2 (D4)
const uint8_t pump1Pin = 16; // GPIO 16  (D0)
const uint8_t pump2Pin = 5;  // GPIO 5  (D1)
IRsend irsend(4);            // GPIO 4 (D2)
ESP8266WebServer server(80);

const long utcOffsetInSeconds = 3600 * 2;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 30000);
bool isWateringOngoing = false;
bool isPump1On = false;
bool isPump2On = false;
int wateringHour = 20;
int wateringStartMinute = 30;
const int waretringTimeInMinutes = 2;

const int pump1StartMinute = wateringStartMinute;
const int pump2StartMinute = wateringStartMinute + waretringTimeInMinutes;
const int pump2EndMinute = wateringStartMinute + (2 * waretringTimeInMinutes);

unsigned long wateringRuns = 0;
void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(pump1Pin, OUTPUT);
  pinMode(pump2Pin, OUTPUT);
  digitalWrite(pump1Pin, LOW);
  digitalWrite(pump2Pin, LOW);
  Serial.begin(115200);
  irsend.begin();
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
  digitalWrite(ledPin, HIGH);
  server.on("/net", handle_net);
  server.on("/", handle_index);
  server.on("/tv", handle_tv);
  server.on("/tvoff", handle_tv_off);
  server.on("/off", handle_off);
  server.on("/netoff", handle_netoff);
  server.on("/netvolup", handle_netVolumeUp);
  server.on("/netvoldown", handle_netVolumeDown);
  server.on("/source", handle_source);
  server.on("/pumpOn", handle_pumpOn);
  server.on("/pumpOff", handle_pumpOff);
  server.on("/setWateringHour", setWateringHour);
  server.begin();
  timeClient.begin();
}

void sendCommand(byte *command, WiFiClient *client)
{
  int startIndex = 18;
  int endIndex = 23;
  for (int i = startIndex; i < endIndex; i++)
  {
    message[i] = command[i - startIndex];
  }
  // This will send a string to the server
  if (client->connected())
  {
    client->write(message, sizeof(message));
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (client->available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(">>> Client Timeout !");
      client->stop();
      delay(6000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  // not testing 'client.connected()' since we do not need to send data here
  while (client->available())
  {
    char ch = static_cast<char>(client->read());
  }
}

void sendDataToOnkyo(byte *fistCommand, byte *secondCommand, const char *text)
{
  digitalWrite(ledPin, LOW);

  Serial.print(text);

  Serial.println("sending command");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port))
  {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  sendCommand(fistCommand, &client);
  if (secondCommand != NULL)
  {
    sendCommand(secondCommand, &client);
  }

  // Close the connection
  client.stop();
  delay(1000);
  digitalWrite(ledPin, HIGH);
}
void watering()
{
  timeClient.update();
  int hour = timeClient.getHours();
  int minute = timeClient.getMinutes();

  if (hour != wateringHour)
  {
    return;
  }
  if (minute >= pump1StartMinute && minute <= pump2StartMinute && isWateringOngoing == false)
  {
    isWateringOngoing = true;
    isPump1On = true;
    digitalWrite(pump1Pin, HIGH);
    Serial.println("Watering pump 1 ON");
    return;
  }
  if (minute >= pump2StartMinute && minute <= pump2EndMinute && isPump1On)
  {
    isPump1On = false;
    digitalWrite(pump1Pin, LOW);
    delay(500);
    digitalWrite(pump2Pin, HIGH);
    Serial.println("Watering pump 1 OFF");
    Serial.println("Watering pump 2 ON");
    isPump2On = true;
    return;
  }
  if (minute >= pump2EndMinute && isPump2On)
  {
    isPump2On = false;
    digitalWrite(pump2Pin, LOW);
    Serial.println("Watering pump 2 OFF");
    delay(500);
    isWateringOngoing == false;
    wateringRuns += 1;
  }
}
void loop()
{
  server.handleClient();
  watering();
}

void handle_source()
{
  irsend.sendNEC(sourceCode, 32);
  sendResponseToClient("source");
}
void handle_index()
{
  sendResponseToClient("index");
}
void handle_net()
{
  sendDataToOnkyo(pwrSelectNet, volume30, "Net");
  sendResponseToClient("NET");
}
void handle_tv()
{
  irsend.sendNEC(onCode, 32);
  sendDataToOnkyo(pwrSelectTv, volume50, "TV");
  sendResponseToClient("TV on");
}

void handle_off()
{
  irsend.sendNEC(onCode, 32);
  sendDataToOnkyo(pwrOff, NULL, "PWR OFF");
  sendResponseToClient("OFF ALL");
}
void handle_netoff()
{
  sendDataToOnkyo(pwrOff, NULL, "PWR OFF");
  sendResponseToClient("NET OFF");
}
void handle_netVolumeUp()
{
  sendDataToOnkyo(volumeUp, NULL, "Volume UP");
  sendResponseToClient("NET Volume UP");
}
void handle_netVolumeDown()
{
  sendDataToOnkyo(volumeDown, NULL, "Volume DOWN");
  sendResponseToClient("NET Volume Down");
}
void handle_tv_off()
{
  irsend.sendNEC(onCode, 32);
  sendResponseToClient("TV OFF");
}
void handle_pump(int state)
{
  String pumpId = server.arg("pumpId");
  digitalWrite(ledPin, HIGH);
  if (pumpId == "1")
  {
    digitalWrite(pump1Pin, state);
  }
  else if (pumpId == "2")
  {
    digitalWrite(pump2Pin, state);
  }
  Serial.print("Watering pump ");
  Serial.print(pumpId);
  Serial.print(" state ");
  Serial.print(state ? "ON" : "OFF");
  Serial.println("");
  delay(500);
  digitalWrite(ledPin, LOW);
  sendResponseToClient("Pump");
}
void handle_pumpOff()
{
  handle_pump(LOW);
}
void handle_pumpOn()
{
  handle_pump(HIGH);
}
void setWateringHour()
{
  String hour = server.arg("hour");
  String minute = server.arg("minute");

  wateringHour = hour.toInt();

  wateringStartMinute = minute.toInt();

  sendResponseToClient("setWateringHour");
}
void sendResponseToClient(char *executedAction)
{
  timeClient.update();
  int hour = timeClient.getHours();
  int minute = timeClient.getMinutes();
  String pageContent = "<ul style=\"font-size: 40Px;zoom: 200%;\"><li><a href =\"net\">NET</a></li><li><a href =\"tv\">TV</a></li><li><a href =\"off\">OFF</a></li><li><a href =\"tvoff\">TV OFF/ON</a></li><li><a href =\"source\">Change SOURCE</a></li><li> </li><li><a href =\"pumpOn?pumpId=1\">PUMP 1 ON</a></li><li><a href =\"pumpOn?pumpId=2\">PUMP 2 ON</a></li><li><a href =\"pumpOff?pumpId=1\">PUMP 1 OFF</a><li><a href =\"pumpOff?pumpId=2\">PUMP 2 OFF</a></li></ul>";
  pageContent = pageContent +
                "<br><br> isWateringOngoing: " + isWateringOngoing +
                "<br>isPump1On: " + isPump1On +
                "<br>isPump2On: " + isPump2On +
                "<br>wateringHour: " + wateringHour + " " + wateringStartMinute +
                "<br>waretringTimeInMinutes: " + waretringTimeInMinutes +
                "<br>pump1StartMinute: " + pump1StartMinute +
                "<br>pump2StartMinute: " + pump2StartMinute +
                "<br>pump2EndMinute: " + pump2EndMinute +
                "<br>hour: " + hour + ":" + minute +
                "<br>wateringRuns: " + wateringRuns;
  server.send(200, "text/html", pageContent);
}
