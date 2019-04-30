#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Display.h"
#include "config.h"
#include "TrainService.h"
#include "Train.h"

Display screen = Display();

unsigned long networkRequestInterval = 60000;
unsigned long lastNetworkRequest = -networkRequestInterval;

unsigned long renderInterval = 2000;
unsigned long lastRender = -renderInterval;

boolean toDraw = false;
const size_t capacity = JSON_ARRAY_SIZE(2) + 2*JSON_OBJECT_SIZE(4) + 40;


train_t * trainList;

train_t * train_create(unsigned int scheduled, unsigned int estimated){
  train_t * t = (train_t*) calloc(1, sizeof(*t));
  t->scheduled = scheduled;
  t->estimated = estimated;
  t->next = NULL;
  return t;
}

void train_insert(train_t * nt){
  train_t **t = &trainList;
  while(*t){
    t = &(*t)->next;
  }
  nt->next = *t;
  *t = nt;
}

void train_remove_all(){
  train_t * current = trainList;
  train_t * temp_node = NULL;
  
  while (current != NULL) {
      temp_node = current;
      current = temp_node->next;
      free(temp_node);
  }
  trainList = NULL;
}

void format_time(char *buff, unsigned int departureTime){  
    int minutes = (departureTime % 60);
    int hours = (departureTime - minutes) / 60;
    sprintf(buff, "%02d%02d", hours, minutes);
}

void format_delay(char *buff, unsigned int delayMinutes){  
    if(delayMinutes < 9) {
      sprintf(buff, " +%dM", delayMinutes);
    } else {
      sprintf(buff, "+%dM", delayMinutes);
    }
}

void train_list_times(){
  train_t * current = trainList;

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
  screen.renderCharArray(toDisplay);
}


void train_list_delays(){
  train_t * train = trainList;
  char toDisplay[20] = {0};

  while(train != NULL){
    char buff[5] = {0};
    char trainString[10] = {0};
    unsigned int minutesDelay = 0;
    if(train->estimated == train->scheduled) {
      format_time(buff, train->estimated);
    } else {
      if(train->estimated == -1){
        sprintf(buff, "%s", " DLY");
      } else if(train->estimated == -2){
        sprintf(buff, "%s", " CAN");
      } else {
        minutesDelay = train->estimated -  train->scheduled;
        format_delay(buff, minutesDelay);
      }
    }
    sprintf(trainString, "%s %s", train->from, buff);
    strcat(toDisplay, trainString);
    Serial.println(trainString);
    train = train->next;
  }
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
    Serial.println("Waiting for WiFi");
    delay(500);
  }  
}

void fetchDepartures(){
  char json[46] = "{\"from\":[\"NMC\",\"NMN\"],\"to\":[\"MAN\"],\"limit\":2}";
  char status[32] = {0};    
  WiFiClientSecure client;

  Serial.print("connecting to ");
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);
    
  if (!client.connect(host, httpsPort)) {
    Serial.println("Can't Connect");
    return;
  }
  
  client.println(F("POST /spread HTTP/1.1"));
  client.println(F("Host: trains.mcrlab.co.uk"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Content-Length: 45"));
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

  StaticJsonDocument<capacity> doc;
  
 // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    Serial.println(capacity);
    return;
  }
  Serial.println("Success");
  
  
  int numberOfTrains = doc.size();
  Serial.print("There are ");
  Serial.print(numberOfTrains);
  Serial.println(" trains in the list");
  
  
  client.stop();

  
  for(int i  = 0; i < numberOfTrains; i++){
    const char * origin = doc[i]["o"].as<char*>(); 
    const char * destination = doc[i]["d"].as<char*>();
    const int scheduled_time = doc[i]["s"].as<int>();
    const int estimated_time = doc[i]["e"].as<int>();
    
    train_t * t = train_create(scheduled_time, estimated_time);
    strlcpy(t->from, origin, sizeof(t->from));
    strlcpy(t->to, destination, sizeof(t->to)); 
    train_insert(t);
  }
  
}

void loop() {
  
  if(millis() > (lastNetworkRequest + networkRequestInterval)) {
    train_remove_all();
    fetchDepartures();  
    lastNetworkRequest = millis();
  } else {
     if(millis() > (lastRender + renderInterval)){
      if(toDraw){
        train_list_delays();
      } else {
         train_list_times();
      }
      toDraw = !toDraw;
      lastRender = millis();
    }
  }

}
