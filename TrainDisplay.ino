#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "Display.h"

const char* ssid = "PLUSNET-HHQJC9";
const char* password = "f6db6dd64c";

const char* host = "trains.mcrlab.co.uk";
const int httpsPort = 443;
Display screen = Display();
  
void setup() {
  screen.init();
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  char toDisplay[5] = "Wifi";
  screen.renderCharArray(toDisplay);
  
    
}

void loop() {

  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/iot/nmc/man";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +  
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  unsigned int time_data = line.toInt();
  int minutes = (time_data % 60);
  int hours = (time_data - minutes)/ 60;
  Serial.print(hours);
  Serial.print(":");
  Serial.println(minutes);
  char toDisplay[5] = "Wifi";
  snprintf(toDisplay, sizeof(toDisplay), "%02d%02d", hours, minutes);
  Serial.println(toDisplay);
  screen.renderCharArray(toDisplay);

  delay(30000);
}
