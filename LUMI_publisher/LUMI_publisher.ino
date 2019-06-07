/*  Final Project
 *  Subscriber/Display
 * 
 */

#include <ESP8266WiFi.h>        //Requisite Libraries . . .
#include <ESP8266HTTPClient.h>  //
#include "Wire.h"               //
#include <PubSubClient.h>       //
#include <ArduinoJson.h>        //
#include "config.h"

/////////// FOR LED & PUSH BUTTON DISPLAY /////////////// 
// these constants won't change:
const int buttonPin = 2;    // the pin that the pushbutton is attached to
const int redPin= 13;        // the pin that the LED is attached to
const int greenPin = 12;     //
const int bluePin = 14;      //

long push_time;              // variable to store time since program was ran

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

//////////////////// MQTT server ///////////////////////
#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server

WiFiClient espClient;                     //blah blah blah, espClient
PubSubClient mqtt(espClient);             //blah blah blah, tie PubSub (mqtt)

char mac[6];                              //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!
char message[201];                        //201, as last character in the array is the NULL character, denoting the end of the array


//////////////////// WEATHER API  ///////////////////////
const char* weatherAPIkey= "e8f7cdfb1bd8edcf0475cb27ef2b30d0";  // Personal API key for Open Weather API

typedef struct {
  float tp;  // Variable to store Temperature as a float number
} MetData;

MetData conditions; // Creates the instance of a MetData type with the variable "conditions"

void setup() {
  // initialize serial communication:
  Serial.begin(115200);

  /// CONNECT TO WIFI ///
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) { // While the Wifi status reads that it is not connected
    delay(500);                           // Delay for half a second
    Serial.print(".");                    // Prints a "."
  }
  
  getMet(); 

  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);  //register the callback function
  
  push_time = millis();
  // initialize the button pin as a input:
  pinMode(buttonPin, INPUT);
  // initialize the LED as an output:
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  Serial.println("==========================================");
  Serial.println("The current temperature is: " + String(conditions.tp) + " degrees Fahrenheit.");
  Serial.println("==========================================");
}


void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop(); //this keeps the mqtt connection 'active'
  
  // TIMER to keep track of when the button is pushed
  long current_time = millis();
  
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);
  
  // How much time has passed since program was ran and when the button was pushed?
  int time_passed = abs(push_time - current_time);   

  //debugging
  // Serial.println("Time Passed: " + String(time_passed));

  // Conditional to check if user is inputing  their behavior within 24 hours since last recording
  if (time_passed > 30000){     // push time - current time < 24 hours (ex: 30 seconds)
      // reset LED color and button count (set this to zero) 
      setColor(255, 255, 255);
      buttonPushCounter = 0;      // Resets counter bc streak is over
   }
   
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter += 1;
      push_time = millis();
      //Serial.println("on");
      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter);
      LEDcolor();
    }
    // Delay a little bit to avoid bouncing
    delay(500);
  }
  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;

  // NEED TO EDIT THIS //
  String temp = String(conditions.tp);
  sprintf(message, "{\"Outdoor Temperature\":\"%s\"}", temp.c_str()); //JSON format using {"XX":"XX"}  
  mqtt.publish("fromLUMI/outdoortemp", message);


}

/*  LEDcolor funciton sets the LED display to a certain color depending on the
 *  number of inputs the user has inputted using the button
 */
void LEDcolor(){
  if (buttonPushCounter == 1 || buttonPushCounter == 2) {
    setColor(255, 0, 0); // Red Color
    delay(500);
  } else if (buttonPushCounter == 3 || buttonPushCounter == 4) {
    setColor(255, 255, 0); // Yellow Color
    delay(500);
  } else if (buttonPushCounter == 5 || buttonPushCounter == 6) {
    setColor(0, 255, 0); // Green Color
    delay(500);
  } else if (buttonPushCounter == 7) {
    setColor(0, 255, 255); // Blue Color
    delay(500);
  } else if (buttonPushCounter > 7) {
    setColor(255, 0, 255); // Purple Color
    delay(500);
  } 
   
}

void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}

/*
 *  getMet function uses the Open Weather API to get the temperature of the area  
 *  that the user resides (Ex: uses Seattle, WA) and if a certain temperature 
 *  outside is measured, then it tells the subscriber to display a certain color
 *  to alert the user to turn off their heat before leaving their house.
 */

void getMet() {
  HTTPClient theClient;
  String apiCall = "http://api.openweathermap.org/data/2.5/weather?q=Seattle";
  apiCall += "&units=imperial&appid=";
  apiCall += weatherAPIkey;
  theClient.begin(apiCall);
  int httpCode = theClient.GET();
  if (httpCode > 0) {

    if (httpCode == HTTP_CODE_OK) {
      String payload = theClient.getString();
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success()) {
        Serial.println("parseObject() failed in getMet().");
        return;
      }

      conditions.tp = root["main"]["temp"].as<float>();
      // String temp = String(conditions.tp);
      // Debugging lines:
      // Serial.println("-----------TEMPERATURE: " + temp);  
      // Serial.println("-----------DESCRIPTION: " + conditions.wd); 
      
    }
  }
  else {
    Serial.printf("Something went wrong with connecting to the endpoint in getMet().");
  }
}


/////CONNECT/RECONNECT/////Monitor the connection to MQTT server, if down, reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe("fromLUMI/+"); // fromLUMMI (LED display) to door sensor we are subscribing to 'theTopic' and all subtopics below that topic
    } else {                        //please change 'theTopic' to reflect your topic you are subscribing to
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

  if (!root.success()) { //well?
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }

  /////
  //We can use strcmp() -- string compare -- to check the incoming topic in case we need to do something
  //special based upon the incoming topic, like move a servo or turn on a light . . .
  //strcmp(firstString, secondString) == 0 <-- '0' means NO differences, they are ==
  /////

  if (strcmp(topic, "fromLUMI/LBIL") == 0) {
    Serial.println("A message from Batman . . .");
  }

  else if (strcmp(topic, "fromLUMI/sunstatus") == 0) {
    Serial.println("Some weather info has arrived . . .");
  }

  else if (strcmp(topic, "fromLUMI/switch") == 0) {
    Serial.println("The switch state is being reported . . .");
  }

  root.printTo(Serial); //print out the parsed message
  Serial.println(); //give us some space on the serial monitor read out
}
