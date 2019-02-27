#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Display.h"
#include "config.h"

Display screen = Display();

typedef struct train {
  train * next;  // pointer to next train in the list
  char from[4]; // from CRS
  char to[4]; // to CRS
  unsigned int scheduled; // departure time in minutes
  unsigned int estimated;
} train_t;

train_t * train_list;

train_t * train_create(unsigned int scheduled, unsigned int estimated){
  train_t * t = (train_t*) calloc(1, sizeof(*t));
  t->scheduled = scheduled;
  t->estimated = estimated;
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

void format_time(char *buff, unsigned int departure_time){
    if(departure_time >= 0) {
      int minutes = (departure_time % 60);
      int hours = (departure_time - minutes) / 60;
      sprintf(buff, "%02d%02d", hours, minutes);
    } else if(departure_time == -1) {
      sprintf(buff, " %s", "DLY");
    } else {
      sprintf(buff, " %s", "CAN");
    }
}

void train_list_all(){
  train_t * current = train_list;
  Serial.println(F("---------\nCurrent Trains\n---------"));
  char toDisplay[20] = {0};

  while(current != NULL){
    char buff[5] = {0};
    char train[10] = {0};

    format_time(buff, current->scheduled);
    sprintf(train, "%s %s", current->from, buff);
    strcat(toDisplay, train);
    Serial.println(train);
    current = current->next;
  }
  Serial.println(F("---------"));
  screen.renderCharArray(toDisplay);
}


void setup() {
  Serial.begin(9600);
  delay(1000); // wait before starting Up
  screen.init();
  screen.renderCharArray("MY TRAINDISPLAY ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  waitForWifi();
}

void waitForWifi(){
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }  
}

void fetchDepartures(){
  char json[46] = "{\"from\":[\"NMC\",\"NMN\"],\"to\":[\"MAN\"],\"limit\":2}";
  char status[32] = {0};    
  WiFiClientSecure client;
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2*JSON_OBJECT_SIZE(4);
  
  

  if (!client.connect(host, httpsPort)) {
    return;
  }
  
  client.println(F("POST /spread HTTP/1.1"));
  client.println(F("Host: trains.mcrlab.co.uk"));
  client.println(F("Content-Type: application/json"));
  client.println("Content-Length: 45");
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
    const char * scheduled_time_string = train["s"];
    const char * estimated_time_string = train["e"];

    int scheduled_time = atoi(scheduled_time_string);
    int estimated_time = atoi(estimated_time_string);
    
    train_t * t = train_create(scheduled_time, estimated_time);
    strlcpy(t->from, origin, sizeof(t->from));
    strlcpy(t->to, destination, sizeof(t->to)); 
    train_insert(t);
  }
  
}

void loop() {
  waitForWifi();
  train_remove_all();
  fetchDepartures();  
  train_list_all();
  delay(60000);
}
