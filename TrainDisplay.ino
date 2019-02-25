#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Display.h"
#include "config.h"

Display screen = Display();

char from[4] = "MAN";
char to[4] = "NMC";


typedef struct train {
  train * next;  // pointer to next train in the list
  char from[4]; // from CRS
  char to[4]; // to CRS
  unsigned int departure; // departure time in minutes
} train_t;

train_t * train_list;

train_t * train_create(unsigned int departure){
  train_t * t = (train_t*) calloc(1, sizeof(*t));
  t->departure = departure;
  t->next = NULL;
  return t;
}

void train_insert(train_t * nt){
  train_t **t = &train_list;
  while(*t){
    t = &(*t)->next;
  }
  nt->next = *t;
  *t = nt;
}

void train_remove_all(){
  train_t * current = train_list;
  train_t * temp_node = NULL;
  
  while (current != NULL) {
      temp_node = current;
      current = temp_node->next;
      free(temp_node);
  }
  train_list = NULL;
}


void train_list_all(){
  train_t * current = train_list;
  Serial.println(F("---------\nCurrent Trains\n---------"));
  while(current != NULL){
    
    char toDisplay[5];
    
    int minutes = (current->departure % 60);
    int hours = (current->departure - minutes) / 60;

    snprintf(toDisplay, sizeof(toDisplay), "%02d%02d", hours, minutes);

    Serial.print(current->from);
    Serial.print(" ");
    Serial.print(current->to);
    Serial.print(" ");
    Serial.println(toDisplay);
      
    current = current->next;
  }
  Serial.println(F("---------"));

}


void setup() {
  Serial.begin(9600);
  delay(1000); // wait before starting Up
  screen.init();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  waitForWifi();
}

void waitForWifi(){
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }  
}

void fetchDepartures(char from[], char to[]){
  char json[43];
  char status[32] = {0};    
  WiFiClientSecure client;
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2*JSON_OBJECT_SIZE(3);
  
  if (!client.connect(host, httpsPort)) {
    return;
  }
  
  snprintf(json, sizeof(json), "{\"from\":[\"%s\"], \"to\":[\"%s\"], \"limit\":2 }", from, to);

  client.println(F("POST /spread HTTP/1.1"));
  client.println(F("Host: trains.mcrlab.co.uk"));
  client.println(F("Content-Type: application/json"));
  client.println("Content-Length: 42");
  client.println();
  client.println(json);

  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.println(F("Unexpected response: "));
    Serial.println(status);
    Serial.println(F("-----"));
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  DynamicJsonBuffer jsonBuffer(capacity);

  JsonArray& root = jsonBuffer.parseArray(client);

  if (!root.success()) {
    Serial.println(F("Parsing failed!"));
    return;
  } 
  
  int numberOfTrains = root.size();
  for(int i  = 0; i < numberOfTrains; i++){
    JsonObject &train = root[i]; 
    const char * origin = train["o"]; 
    const char * destination = train["d"];
    const char * departure_time_string = train["t"];

    int departure_time = atoi(departure_time_string);
    train_t * t = train_create(departure_time);
    strlcpy(t->from, origin, sizeof(t->from));
    strlcpy(t->to, destination, sizeof(t->to)); 
    train_insert(t);
  }
  
}

void loop() {
  waitForWifi();
  train_remove_all();
  fetchDepartures(from, to);  
  train_list_all();
  delay(60000);
}
