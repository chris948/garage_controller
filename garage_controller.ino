#include <PubSubClient.h>
#include <ESP8266SMTP.h>
#include <ESP8266WiFi.h>
#include "secrets.h"

const char *message = "CLOSED";

const char *wifi_ssid = _wifi_ssid;
const char *wifi_password = _wifi_password;
const char *mqtt_server = _mqtt_server;

//#define wifi_password ""

//#define mqtt_server "172.16.0.2"
//#define mqtt_user "your_username"
//#define mqtt_password "your_password"

#define garage_topic_out "sensor/garageDoorStatus"
#define garage_topic_in "sensor/garageDoorRelay"

//WiFiClient espClient;
//PubSubClient client(espClient);


// constants won't change. They're used here to set pin numbers:
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

// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.

  // Allocate the correct amount of memory for the payload copy
  byte* p = (byte*)malloc(length);

  byte OPEN[5] = "FLIP";

  // Copy the payload to the new buffer
  memcpy(p, payload, length);
  //client.publish("outTopic", p, length);

  //test print output test = 116 101 115 116
  for (int i = 0; i < length; i++)
  {
    Serial.println(p[i]);
  }

  int check;
  check = memcmp(payload, OPEN, sizeof(payload));
  Serial.println(check);
  if (check == 0) {
    flipRelay();
  }

  //  if( (*p == '1') || (*p == 'CLOSE') ){
  //    Serial.println("RECEIVED OPEN OR CLOSE");
  //    flipRelay();
  //  }
  // Free the memory
  free(p);
}

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

//void send_gmail() {
//    if(SMTP.Send("chris@pangburn.io", "Garage Door Open Warning")) {
//    Serial.println(F("Message sent"));
//  } else {
//    Serial.print(F("Error sending message: "));
//    Serial.println(SMTP.getError());
//  }
//
//}

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

//bool checkBound(float newValue, float prevValue, float maxDiff) {
//  return !isnan(newValue) &&
//         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
//}


void sendSignal() {
  Serial.println("sending signal");
  //Serial.println(String(switchState).c_str());
  //flipRelay();
  //client.publish(garage_topic_out, String(switchState).c_str());
  client.publish(garage_topic_out, String(message).c_str());
  lastState = digitalRead(switchPin);
}

//to consider if efficiency is ever important, only send signal when necessary
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
  Serial.println("flipping relay");
  //Serial.println(String(switchState).c_str());

  //test internal LED
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on by making the voltage LOW
  delay(1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(2000);                      // Wait for two seconds

  digitalWrite(relayPin, HIGH);
  delay(500);
  digitalWrite(relayPin, LOW);
}

//monitor mqtt for change

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

  //test LED
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  checkSwitch();
  delay(5000);
}
