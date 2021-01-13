extern "C" 
{
#include "user_interface.h"
} 

#include <FS.h>
#include <SPI.h>
#include <Wire.h>

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <DNSServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebOTA.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

//todo - stop hard coding this
const char* mqttServer = "172.16.0.2";
const int mqttPort = 1883;

//todo - stop hard coding this
#define garage_topic_out "sensor/garageDoorStatus"
#define garage_topic_in "sensor/garageDoorRelay"

//todo - stop hard coding this
//constants for GPIO pins
const int groundPin = 12;     // the number of the pushbutton pin
const int relayPin =  4;      // the number of the LED pin
const int switchPin = 13; 

// variables will change:
int switchState = 0;         // variable for reading the pushbutton status
int lastState = 0;

void flipRelay() {
  digitalWrite(relayPin, HIGH);
  delay(500);
  digitalWrite(relayPin, LOW);
}

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];  //Conver *byte to String
  }
  Serial.println("-----------------------"); 
  Serial.println(message);
  if(message == "FLIP") {
    flipRelay();
    }
  Serial.println("-----------------------");  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, uncomment next line
    if (client.connect("ESP8266_garage")) {
      // if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish(garage_topic_out, "****light esp8266 online****");
      // ... and resubscribe
      client.subscribe(garage_topic_in);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//// function to publish an mqtt message
//void sendSignal(String mySignal) {
//  //Serial.println("sending signal");
//  client.publish(garage_topic_out, mySignal);
//  lastState = digitalRead(switchPin);
//}


void checkSwitch() {
  String myMessage = "";
  Serial.println("checking switch");
  switchState = digitalRead(switchPin);
  Serial.println(String(switchState).c_str());
  
  if (switchState == 1) {
    myMessage = "OPEN";
  } else {
    myMessage = "CLOSED";
  }

  //if state has changed, publish mqtt state change
  if (switchState != lastState){
    client.publish(garage_topic_out, (char*) myMessage.c_str());
    lastState = switchState;    
  }
}

void setup() {
  Serial.begin(115200);
  
  //start wifi
  wifiSetup();
  mdnsSetup();
  
  // initialize the switch pin as an input:
  pinMode(switchPin, INPUT_PULLUP);
  // initialize the relay as an output:
  pinMode(relayPin, OUTPUT);

  //hack for extra ground pin
  pinMode(groundPin, OUTPUT);
  digitalWrite(groundPin, LOW);
  

  //set lastState on boot so when it checks the first time it won't change anything
  //lastState = digitalRead(switchPin);

  
  Serial.println(WiFi.localIP());
  

  //webota
  // To use a specific port and path uncomment this line
  // Defaults are 8080 and "/webota"
  //webota.init(8888, "/update");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(MQTTcallback);
 

  Serial.println("Connecting to MQTT...");
 
  if (client.connect("ESP8266")) {
    Serial.println("connected");  
  } 
  else {
    Serial.print("failed with state ");
    Serial.println(client.state());  //If you get state 5: mismatch in configuration
    delay(2000);
  }
  client.subscribe(garage_topic_in);
  checkSwitch();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  webota.handle();
  checkSwitch();
  client.loop();
  delay(500);
}
