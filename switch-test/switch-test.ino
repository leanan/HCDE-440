/*
  
 Created by Rui Santos
 
 All the resources for this project:
 https://randomnerdtutorials.com/
 
*/

#include "config.h"

// set up the 'digital' feed in Adafruit IO and reference it here with io.feed("[NAME OF FEED]");
AdafruitIO_Feed *digital = io.feed("door");

int ledOpen=13;
int ledClose=15;
int switchReed=12;
int current = 1;

void setup(){
  pinMode(ledOpen, OUTPUT);
  pinMode(ledClose, OUTPUT);
  pinMode(switchReed, INPUT);
  Serial.begin(115200);

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
  
  if (digitalRead(switchReed)==HIGH){
    current = 1;
    digitalWrite(ledOpen, LOW);
    digitalWrite(ledClose, HIGH);
    Serial.println("Your Door is Closed");
    // digital->save(current);
  }
  else {
    current = 0;
    digitalWrite(ledOpen, HIGH);
    digitalWrite(ledClose, LOW);
    Serial.println("Your Door is Open");
    // digital->save(current);
  }
  
  digital->save(current);
  delay(500);
}
