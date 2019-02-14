#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "Display.h"
#include "config.h"

//const char* ssid = "PLUSNET-HHQJC9";
//const char* password = "f6db6dd64c";

const int DEBUG_PIN = 13;
const char* host = "trains.mcrlab.co.uk";
const int httpsPort = 443;
Display screen = Display();
boolean paused = true;
const int MOVEMENT_PIN = 14;

void off() {
  screen.clear();   
}

void setup() {
  Serial.begin(115200);
  screen.init();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  char toDisplay[5] = "WIFI";
  screen.renderCharArray(toDisplay);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  off();
}

void triggerMovement() {
  paused = false;
}

void displayDepartures(char from[], char to[]) {
 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  char toDisplay[5];
  
  WiFiClientSecure client;

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  
  char url[13] = "/iot/nmc/man";
  snprintf(url, sizeof(url), "/iot/%s/%s", from, to);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +  
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  String line = client.readStringUntil('\n');
  unsigned int time_data = line.toInt();
  int minutes = (time_data % 60);
  int hours = (time_data - minutes)/ 60;

  snprintf(toDisplay, sizeof(toDisplay), "%02d%02d", hours, minutes);
  screen.renderCharArray(toDisplay);
  delay(60000);
}

void loop() {
  char from[4] = "nmc";
  char to[4] = "man";
  displayDepartures(from, to);
}
