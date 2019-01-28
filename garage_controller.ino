#include <PubSubClient.h>
#include <ESP8266SMTP.h>
#include <ESP8266WiFi.h>
#include "secrets.h"

const char *message = "CLOSED";

const char *wifi_ssid = _wifi_ssid;
const char *wifi_password = _wifi_password;
const char *mqtt_server = _mqtt_server;


#define garage_topic_out "sensor/garageDoorStatus"
#define garage_topic_in "sensor/garageDoorRelay"


//constants for GPIO pins
const int switchPin = 5;     // the number of the pushbutton pin
const int relayPin =  4;      // the number of the LED pin

// variables will change:
int switchState = 0;         // variable for reading the pushbutton status
int lastState = 0;



void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, callback, espClient);

// Callback function when a message is received
void callback(char* topic, byte* payload, unsigned int length) {


  // Allocate the correct amount of memory for the payload copy
  //byte* p = (byte*)malloc(length);

  //byte array to compare the MQTT incoming message to
  byte OPEN[5] = "FLIP";

  int check;
  check = memcmp(payload, OPEN, sizeof(payload));
  Serial.println(check);
  if (check == 0) {
    flipRelay();
  }
}

//function if I ever want to move the warning to the esp8266
void send_gmail() {
  const char *email = _email;
  const char *email_password = _email_password;
  const char *smtpSend = _smtpSend;

  SMTP.setEmail(email)
  .setPassword(email_password)
  .Subject("Garage door open warning")
  .setFrom("ESP8266SMTP")
  .setForGmail();

  if (SMTP.Send(smtpSend, "Garage Door Open Warning")) {
    Serial.println(F("Message sent"));
  }

  else {
    Serial.print(F("Error sending message: "));
    Serial.println(SMTP.getError());
  }


}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, uncomment next line
    if (client.connect("ESP8266Client")) {
      // if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish(garage_topic_out, "****garage esp8266 online****");
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

// function to publish an mqtt message
void sendSignal() {
  //Serial.println("sending signal");
  client.publish(garage_topic_out, String(message).c_str());
  lastState = digitalRead(switchPin);
}

//todo if efficiency is ever important, only send signal when necessary
void checkSwitch() {
  Serial.println("checking switch");
  switchState = digitalRead(switchPin);
  Serial.println(String(switchState).c_str());
  if (switchState == 1) {
    message = "OPEN";
  } else {
    message = "CLOSED";
  }
  sendSignal();

  //  if (switchState != lastState){
  //    sendSignal();
  //  }
}

void flipRelay() {
  digitalWrite(relayPin, HIGH);
  delay(500);
  digitalWrite(relayPin, LOW);
}


void setup() {
  Serial.begin(115200);
  setup_wifi();
  //  setup_gmail()
  client.setServer(mqtt_server, 1883);

  // initialize the switch pin as an input:
  pinMode(switchPin, INPUT_PULLUP);
  // initialize the relay as an output:
  pinMode(relayPin, OUTPUT);

  //set lastState on boot so when it checks the first time it won't change anything
  //lastState = digitalRead(switchPin);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  checkSwitch();
  delay(5000);
}
