#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "FS.h"

#define S1 D4
#define S2 D3
#define S3 D2
#define S4 D1

const char* ssid = "Floor2_bed1";
const char* password = "myfirsttest";


unsigned int Udpport = 4210;  // local port to listen on UDP
int value = LOW;
unsigned int Serverport = 80; // http server port
char incomingPacket[255];  // buffer for incoming packets

WiFiUDP Udp;
WiFiServer server(Serverport);

IPAddress ip2(192, 168, 4, 2);
IPAddress ip3(192, 168, 4, 3);
IPAddress ip4(192, 168, 4, 4);
IPAddress ip5(192, 168, 4, 5);
IPAddress send_udp;

void setup()
{
  int i = 0;
  int stats[4];
  EEPROM.begin(512);
  Serial.begin(115200);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(S4, OUTPUT);
  for (i = 0; i < 4; i++)
  {
    stats[i] =  EEPROM.read(i);
    Serial.println("read EEpROm success");
  }
  EEPROM.end();
  digitalWrite(S1, stats[0]);
  digitalWrite(S2, stats[1]);
  digitalWrite(S3, stats[2]);
  digitalWrite(S4, stats[3]);
  setupwifi();
  bool ok = SPIFFS.begin();
  if (ok) {
    Serial.println("ok");
    bool exist = SPIFFS.exists("/web.txt");

    if (exist)
      Serial.println("The file exists!");
  }
  server.begin();
  Serial.println("Server started");
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.softAPIP());
  Serial.println("/");


}

void loop()
{
loop1:
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(10);
    WiFiClient client = server.available();
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
  if (request.indexOf("/favicon.ico") != -1)
  {
    client.stop();
    goto loop1;
  }
  // Match the request
  action(request);
  File f = SPIFFS.open("/web.txt", "r");
  String data = f.readString();
  f.close();
  client.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE HTML>\n");
  client.println(data);
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");


  client.stop();
  server.stop();
  delay(10);
  if (request.indexOf("ID0") == -1)
  {
    Udp.begin(Udpport);
    delay(10);
    Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.softAPIP().toString().c_str(), Udpport);
    Serial.printf("Sending request %s to IP %s\n" , request.c_str(), send_udp.toString().c_str());
    Udp.beginPacket(send_udp, Udpport);
    Udp.write(request.c_str());
    Udp.endPacket();
    Udp.stop();
    delay(10);
  }
  server.begin();
  delay(10);
}

void setupwifi(void)
{
  WiFi.mode(WIFI_AP);
  Serial.println("WIFI Mode : AccessPoint Station");

  // Starting The Access Point
  WiFi.softAP(ssid, password);
  Serial.println("WIFI < " + String(ssid) + " > ... Started");

  // Wait For Few Seconds
  delay(1000);

  // Getting Server IP
  IPAddress IP = WiFi.softAPIP();

  // Printing The Server IP Address
  Serial.print("AccessPoint IP : ");
  Serial.println(IP);
}

void action(String request)
{
  EEPROM.begin(512);
  if (request.indexOf("ID0") != -1)
  {
    if (request.indexOf("ON") != -1)
    {
      value = HIGH;
    }
    else
    {
      value = LOW;
    }
    if (request.indexOf(":1") != -1)
    {
      digitalWrite(S1, value);
      EEPROM.write(0, value);
    }
    if (request.indexOf(":2") != -1)
    {
      digitalWrite(S2, value);
      EEPROM.write(1, value);
    }
    if (request.indexOf(":3") != -1)
    {
      digitalWrite(S3, value);
      EEPROM.write(2, value);
    }
    if (request.indexOf(":4") != -1)
    {
      digitalWrite(S4, value);
      EEPROM.write(3, value);
    }

  }
  EEPROM.end();
  if (request.indexOf("ID1") != -1)
    send_udp = ip2;

  if (request.indexOf("ID2") != -1)
    send_udp = ip3;

  if (request.indexOf("ID3") != -1)
    send_udp = ip4;

  if (request.indexOf("ID4") != -1)
    send_udp = ip5;

}

