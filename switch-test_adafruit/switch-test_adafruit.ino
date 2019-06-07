/*
  
LEANA NAKKOUR
Subscriber Code
MAGNETIC CONTACT SWITCH (Door Sensor) + IFTTT
THe following code checks for if your door is closed or opened and reports that to an Adafruit Feed called "Door"
link: https://io.adafruit.com/leana_n/dashboards/hcde440-final
If the door is opened, IFTTT is triggered to send the user a message about sustainable behaviors.
This is DONE - need to enclose
 
*/

#include "config.h"
#include <Wire.h>               // Necessary Libraries for Light sensor and MQTT server
#include <PubSubClient.h>       //
#include <ArduinoJson.h>        //
#include <ESP8266WiFi.h>        //
#include <ESP8266HTTPClient.h>  //
#include "Wire.h" 

////////// MQTT SERVER ////////////
#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server

WiFiClient espClient;                     //blah blah blah, espClient
PubSubClient mqtt(espClient);             //blah blah blah, tie PubSub (mqtt)

char mac[6]; //A MAC address as the unique user ID!
char message[201]; //201, as last character in the array is the NULL character, denoting the end of the array
char espUUID[8] = "ESP8602"; // Name of the microcontroller


// set up the 'digital' feed in Adafruit IO and reference it here with io.feed("[NAME OF FEED]");
// https://io.adafruit.com/leana_n/dashboards/hcde440-final
AdafruitIO_Feed *digital = io.feed("door");
AdafruitIO_Feed *digital2 = io.feed("outdoorTemp");


// These variables will not change
const int ledOpen = 13;       // Green LED
const int ledClose = 15;      // Red LED
const int switchReed = 12;    // Switch Magnet

// These variables WILL change
int door_state = 1;     // Reports to Adafruit Feed if door is open(0)/closed(1)
float temperature;    // Stores temperature received from Publisher

void setup(){
  pinMode(ledOpen, OUTPUT);
  pinMode(ledClose, OUTPUT);
  pinMode(switchReed, INPUT);
  Serial.begin(115200);

  /// CONNECT TO WIFI ///
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) { // While the Wifi status reads that it is not connected
    delay(500);                           // Delay for half a second
    Serial.print(".");                    // Prints a "."
  }

  /// CONNECT TO MQTT SERVER ///
  mqtt.setServer(mqtt_server, 1883); // Start the mqtt
  mqtt.setCallback(callback); //Register the callback function
  
  // ADAFRUIT 
  // wait for serial monitor to open
  while(! Serial);

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.println(io.statusText());
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
}

void loop(){
  io.run();

  if (!mqtt.connected()) {  // Try connecting again
    reconnect();
  }
  mqtt.loop(); //this keeps the mqtt connection 'active'

  
  bool new_state = digitalRead(switchReed);       // Gets current state of the door sensor
  int current_state;      
  if (new_state == HIGH){
    current_state = 1;
  } else {
    current_state = 0;
  }

  // Keep track of previous door state to compare to new state to see if it has changed
  // This limits how often data is pushed to adafruit and overflow of feed
  if (door_state != current_state){
    if (digitalRead(switchReed)==HIGH){        // Magnets are touching, so door is closed
      door_state = 1;                  
      digitalWrite(ledOpen, LOW);              // Green LED OFF
      digitalWrite(ledClose, HIGH);            // Red LED ON
      Serial.println("Your Door is Closed");
      digital->save(door_state);
    }else {                                      // Magnets not touching, so door is open
      door_state = 0;
      digitalWrite(ledOpen, HIGH);              // Green LED ON
      digitalWrite(ledClose, LOW);              // Red LED OFF
      Serial.println("Your Door is Open");
      digital->save(door_state);
    }
  }
  
  delay(1000);
  // digital->save(current);
}

//function to reconnect if we become disconnected from the server
void reconnect() {

  // Loop until we're reconnected
  while (!espClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // Attempt to connect
    if (mqtt.connect(espUUID, mqtt_user, mqtt_password)) { //the connction
      Serial.println("connected");
      // Once connected, publish an announcement...
      char announce[40];
      strcat(announce, espUUID);
      strcat(announce, "is connecting. <<<<<<<<<<<");
      mqtt.publish(espUUID, announce);
      // ... and resubscribe
      //      client.subscribe("");
      mqtt.subscribe("fromLUMI/outdoortemp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic); //'topic' refers to the incoming topic name, the 1st argument of the callback function
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer; //blah blah blah a DJB
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!

  // Receives the light sensor data and stores it in a variable light_status
  if (String(topic) == "fromLUMI/outdoortemp") {
    Serial.println("printing message");
    Serial.print("Message arrived in topic: ");
    temperature = root["Outdoor Temperature"];
    // debug
    Serial.println("TEMPERATURE: " + String(temperature));

    if (temperature > 49) { //would change to 72 deg
      int trigger = 1;
      digital2->save(trigger);
    }
    
 }

  if (!root.success()) { //well?
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }
}
