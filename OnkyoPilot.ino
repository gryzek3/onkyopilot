/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "WifiConfiguration.h"
#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#define STAONKOYOHOSTIP ""
#endif
byte message[] = {
    0x49, 0x53, 0x43, 0x50,
    0x00, 0x00, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x08, //replace last with length
    0x01, 0x00, 0x00, 0x00,
    0x21, 0x31, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0D //49
                           //Add data + EOF here
};
byte pwrOn[] = {0x50, 0x57, 0x52, 0x30, 0x31};
byte pwrOff[] = {0x50, 0x57, 0x52, 0x30, 0x30};
byte pwrSelectNet[] = {0x53, 0x4C, 0x49, 0x32, 0x39};
byte pwrSelectTv[] = {0x53, 0x4C, 0x49, 0x32, 0x30};
byte volume30[] = {0x4D, 0x56, 0x4C, 0x34, 0x33};
byte volume50[] = {0x4D, 0x56, 0x4C, 0x37, 0x33};

const char *ssid = STASSID;
const char *password = STAPSK;

const char *host = STAONKOYOHOSTIP;
const uint16_t port = 60128;

const uint8_t ledPin= 2;
ESP8266WebServer server(80);

void setup()
{
  pinMode(ledPin, OUTPUT);
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
  digitalWrite(ledPin, HIGH);
  server.on("/net", handle_net);
  server.on("/tv", handle_tv);
  server.on("/off", handle_off);
  server.begin();
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

void loop()
{
  server.handleClient();
}

void handle_net() {
  sendDataToOnkyo(pwrSelectNet, volume30,  "Net");
  server.send(200, "text/html", "NET"); 
}
void handle_tv() {
  sendDataToOnkyo(pwrSelectTv, volume50, "TV");
  server.send(200, "text/html", "TV"); 
}
void handle_off() {
  sendDataToOnkyo(pwrOff, NULL,  "PWR OFF");
  server.send(200, "text/html", "OFF"); 
}