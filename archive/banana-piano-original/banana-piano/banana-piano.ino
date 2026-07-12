
/* musical notes */
#include "pitches.h"


// PIN VARIABLE DECLARATIONS
const int echoPin = 11;
const int triggerPin = 12;
const int ledPin = 13;
 
// VARIABLES FOR CALCULATIONS
unsigned int duration, distance;
 
// TONE OUTPUT
const int Buzz = 8;


void setup(){
  // SET UP SERIAL COMMUNICATION
  //Serial.begin(9600);
  // CONFIGURE INPUT AND OUTPUT PINS
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(13, OUTPUT);
  
// Declare the buzzer
  pinMode(Buzz, OUTPUT);

}

void loop(){

  /*
 // SEND THE FIRING PULSE ON THE "TRIGGER" PIN
  digitalWrite(triggerPin, LOW);
  //delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  // THE PULSE STAYS HIGH FOR AT LEAST 10 uS
  //delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
 
  // MEASURE THE HIGH TIME ON THE "ECHO" PIN; THE PULSE IS PROPORTIONAL TO THE MEASURED DISTANCE
  duration = pulseIn(echoPin, HIGH);
 
  // THE SPEED OF SOUND IS 340 M/S, I.E. 29 MICROSECONDS PER CENTIMETER
  // DIVIDE THE PULSE TIME BY 58, THE ROUND-TRIP TIME OF THE SOUND WAVE PER CENTIMETER
  distance = duration / 58;
 
  // SEND THE RESULT TO THE SERIAL MONITOR
  //Serial.print(distance);
  //Serial.println(" cm");
  //delay(200);
 */


  


// LEMON KEYBOARD
 if( ((analogRead(0)+analogRead(0))/2) <=1019){
  tone(Buzz, NOTE_C3, 100);    
 }

 if( ((analogRead(1)+analogRead(1))/2)<= 1019){  
   tone(Buzz, NOTE_D3, 100);
 }

if( ((analogRead(2)+analogRead(2))/2)<= 1019){  
   tone(Buzz, NOTE_E3, 100);
 }

if( ((analogRead(3)+analogRead(3))/2)<= 1019){  
   tone(Buzz, NOTE_F3, 100);
 }

if( ((analogRead(4)+analogRead(4))/2)<= 1019){  
   tone(Buzz, NOTE_G3, 100);
 }

if( ((analogRead(5)+analogRead(5))/2)<= 1019){  
   tone(Buzz, NOTE_A3, 100);
 }

if( ((analogRead(6)+analogRead(6))/2)<= 1019){  
   tone(Buzz, NOTE_B3, 150);
 }
 
 






}
