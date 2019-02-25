#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "Display.h"
#include "config.h"

Display screen = Display();
boolean paused = true;


typedef struct train {
  train * next;  // pointer to a tram
  unsigned int  departure;
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
  Serial.println("remove all trains");
  train_t * current = train_list;
  train_t * temp_node = NULL;
  
  while (current != NULL) {
      temp_node = current;
      Serial.print("removing ");
      Serial.println(temp_node->departure);
      current = temp_node->next;
      free(temp_node);
  }
  Serial.println("removed all");
  train_list = NULL;
}


void train_list_all(){
  train_t * current = train_list;
  Serial.println("---------\nCurrent Trains\n---------");
  while(current != NULL){
    
    Serial.println(current->departure);
    
    current = current->next;
  }
  Serial.println("---------");

}


void setup() {
  Serial.begin(9600);
  delay(1000); // wait before starting Up
  screen.init();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  char toDisplay[5] = "WIFI";
  screen.renderCharArray(toDisplay, -1);
  waitForWifi();
}

void waitForWifi(){
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }  
}

unsigned int fetchDepartures(char from[], char to[]){
  char json[43];
    
  WiFiClientSecure client;

  if (!client.connect(host, httpsPort)) {
    return -1;
  }
  
  snprintf(json, sizeof(json), "{\"from\":[\"%s\"], \"to\":[\"%s\"], \"limit\":1 }", from, to);

  client.println("POST /spread HTTP/1.1");
  client.println("Host: trains.mcrlab.co.uk");
  client.println("Content-Type: application/json");
  client.println("Content-Length: 42");
  client.println();
  client.println(json);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println(line);

  String from_data;
  String to_data;
  String departure_time;
  

  int ind1, ind2, ind3;

  ind1 = line.indexOf('|');
  from_data = line.substring(0, ind1);

  ind2 = line.indexOf('|', ind1+1 );   //finds location of second ,
  to_data = line.substring(ind1+1, ind2);
  
  ind3 = line.indexOf(',', ind2+1 );
  departure_time = line.substring(ind2+1, ind3);

  Serial.print("From ");
  Serial.println(from_data);

  Serial.print("To ");
  Serial.println(to_data);

  Serial.print("Time ");
  Serial.println(departure_time);
  
  unsigned int time_data = departure_time.toInt();
  return time_data;
}

void displayDepartures(unsigned int time_data) {
  char toDisplay[5];
  
  int minutes = (time_data % 60);
  int hours = (time_data - minutes)/ 60;

  snprintf(toDisplay, sizeof(toDisplay), "%02d%02d", hours, minutes);
  screen.renderCharArray(toDisplay, 1);
}

void loop() {
  char from[4] = "NMC";
  char to[4] = "MAN";

  train_remove_all();
  
  for(unsigned int i = 0; i < 10; i++){
    train_t * t = train_create(i);
    train_insert(t);
  }
  train_list_all();
  delay(5000);
 // waitForWifi();
 // unsigned int nextDeparture = fetchDepartures(from, to);  
 // displayDepartures(nextDeparture);
 // delay(60000);
}
