#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "Display.h"
#include "config.h"

const int LED_PIN = 0;
const char* host = "trains.mcrlab.co.uk";
const int httpsPort = 443;

Display screen = Display();
boolean paused = true;

void off() {
  screen.clear();   
}

void setup() {
  Serial.begin(115200);
  delay(1000); // wait before starting Up
  screen.init();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  char toDisplay[5] = "WIFI";
  screen.renderCharArray(toDisplay);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void displayDepartures(char from[], char to[]) {
 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  char toDisplay[5];
  char url[13];
    
  WiFiClientSecure client;

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  
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
}

void loop() {
  char from[4] = "nmc";
  char to[4] = "man";
  displayDepartures(from, to);
  delay(60000);
}
