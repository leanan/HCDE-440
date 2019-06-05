/*
  
LEANA NAKKOUR
MAGNETIC CONTACT SWITCH (Door Sensor) + IFTTT
THe following code checks for if your door is closed or opened and reports that to an Adafruit Feed called "Door"
link: https://io.adafruit.com/leana_n/dashboards/hcde440-final
If the door is opened, IFTTT is triggered to send the user a message about sustainable behaviors.
This is DONE - need to enclose
 
*/

#include "config.h"

// set up the 'digital' feed in Adafruit IO and reference it here with io.feed("[NAME OF FEED]");
AdafruitIO_Feed *digital = io.feed("door");

int ledOpen = 13;       // Green LED
int ledClose = 15;      // Red LED
int switchReed = 12;    // Switch Magnet
int door_state = 1;     // Reports to Adafruit Feed if door is open(0)/closed(1)

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
  
  delay(500);
  // digital->save(current);
}
