/* 
Following code by EricWillaim
All build information, code and forum posts/discussion can be found at:
http://mkme.org

Dedicated build page:
http://www.mkme.org/index.php/arduino-sainsmart-4wd-robot/

This is my attempt at programming the Sainsmart 4WD robot
So far it now checks for a new direction periodically and performs
backup and turn functions as in reaction to the environment it is presented.
It is by far not optimized code- but works for now :)
I will update this periodically as I get time to learn new methods and improve the code. 
Please check out my Youtube videos here and consider a thumbs up if this helped you!
Youtube : http://www.youtube.com/user/Shadow5549

Some portions of this code adapted from:
http://www.duino-robotics.com/

V9 Dec 12 2013:
Added a forward distance check after turns- will continue to turn if distance still too small
Minor Tweaks but still using delay functions

V10 May 2014
Added support for Roam mode- serial commands will toggle roam versus individual driving commands

V11 May 22 2014
Modded code for Android app control using accelerometers (just change of key assigments in code to match app)
App:
https://play.google.com/store/apps/details?id=braulio.calle.bluetoothRCcontroller
Website:
https://sites.google.com/site/bluetoothrccar/


V12 July 28 2014
Cleaned up code

*/

#include <Servo.h> //servo library
#include "pitches.h"// used for the speaker output
#define SPEAKER 4// Speaker Pin

Servo headservo;
const int EchoPin = 2; // Ultrasonic signal input
const int TrigPin = 3; // Ultrasonic signal output
const int leftmotorpin1 = 8; //signal output of Dc motor driven plate
const int leftmotorpin2 = 9;
const int rightmotorpin1 = 6;
const int rightmotorpin2 = 7;
const int HeadServopin = 5; // signal input of headservo
const int maxStart = 800; //run dec time- no clue what this is
unsigned long time; //used for upcoming improvements (time used instead of loops)
unsigned long time1; //used for upcoming improvements (time used instead of loops)
int add= 0; //used for nodanger loop count
int add1= 0;  //used for nodanger loop count
int roam = 0;
int currDist = 5000; // distance
boolean running = false;// This is from old code examples

void setup() {
  //initialize beeps
  tone(SPEAKER, NOTE_C7, 100);
  delay(500);
  tone(SPEAKER, NOTE_C6, 100);
  tone(SPEAKER, NOTE_C7, 100);
  delay(500);
  tone(SPEAKER, NOTE_C6, 100);
  tone(SPEAKER, NOTE_C7, 100);
  delay(500);
  tone(SPEAKER, NOTE_C6, 100);
  //End Initialize Beeps
  
Serial.begin(9600); // Enables Serial monitor for debugging purposes
Serial.println("Serial Data Initiated!"); // Test the Serial communication
pinMode(EchoPin, INPUT);//signal input port
for (int pinindex = 3; pinindex < 10; pinindex++) {
pinMode(pinindex, OUTPUT); // set pins 3 to 10 as outputs
}

// headservo interface
headservo.attach(HeadServopin);
headservo.write(160);
delay(1000);
headservo.write(20);
delay(1000);
headservo.write(90);
delay(1000);
return;
}

void loop() 
{
 if (Serial.available() > 0) 
 {
   int data = Serial.read();	//read serial input commands
   switch(data)
   {
    case 'F' : 
   Serial.println("Rolling!");
   moveForward(); 
   break;
    case 'B' : 
   back(); 
   //buzz(); 
   break;
    case 'L' : 
   lturn(); 
   //buzz(); 
   break;
    case 'R' : 
   rturn(); 
   // buzz(); 
   break;
   case 'S' : 
   totalhalt(); 
   break;
    case 'W' : 
   toggleRoam(); 
   buzz(); 
   break; }
 }
  if(roam == 0){ //just listen to serial commands and wait
   // Do something else if you like
 }
  else if(roam == 1)//If roam active- drive autonymously
{  
//time = millis(); // Sets "time" to current system time count
currDist = MeasuringDistance(); //measure front distance
Serial.print("Current Forward Distance: ");
//Serial.println(currDist);
if(currDist > 35) {
  add = (add1++);// Start adding up the loop count done in nodanger
  nodanger();
  Serial.println("Nodanger: ");}

else if(currDist < 35){
  //add=0;
  Serial.println("Forward Blocked- Decide Which Way");
  backup();
  whichway();
 }  
 }
 }
    
//measure distance, unit “cm”
long MeasuringDistance() {
long duration;
digitalWrite(TrigPin, LOW);
delayMicroseconds(2);
digitalWrite(TrigPin, HIGH);
delayMicroseconds(5);
digitalWrite(TrigPin, LOW);
duration = pulseIn(EchoPin, HIGH);
return duration / 29 / 2;
}
  
//----------------------------------------------------- These are for serial control only
// forward
void moveForward() {
analogWrite(leftmotorpin1, 0);//Changed these to analog write for slower
analogWrite(leftmotorpin2, 120);
analogWrite(rightmotorpin1, 0);
analogWrite(rightmotorpin2, 120);
}


void back() {
analogWrite(leftmotorpin1, 120);//Changed these to analog write for slower
analogWrite(leftmotorpin2, 0);
analogWrite(rightmotorpin1, 120);
analogWrite(rightmotorpin2, 0);
}

void lturn() {
analogWrite(leftmotorpin1, 0);
analogWrite(leftmotorpin2, 190);
analogWrite(rightmotorpin1, 190);
analogWrite(rightmotorpin2, 0);
//Serial.println("Turning Left");
}

//turn right
void rturn() {
analogWrite(leftmotorpin1, 190);
analogWrite(leftmotorpin2, 0);
analogWrite(rightmotorpin1, 0);
analogWrite(rightmotorpin2, 190);
//Serial.println("Turning Right");
}
//----------------------------------------------------- These are for serial control only

void toggleRoam(){
if(roam == 0){
roam = 1;
Serial.println("Activated Roam Mode");
  }
else{
roam = 0;
totalhalt();
Serial.println("De-activated Roam Mode");
}}

void nodanger() {
analogWrite(leftmotorpin1, 0);//Changed these to analog write for slower
analogWrite(leftmotorpin2, 120);
analogWrite(rightmotorpin1, 0);
analogWrite(rightmotorpin2, 120);
if (add1 > 38 ) whichway(); // Robot will stop and check direction every X loops through nodanger then resets in totalhalt (40 is good)
//This is reset only in VoidBackup
return;}

//backward
void backup() {
add1=0; // resets the counter for the nodanger loops
digitalWrite(leftmotorpin1, HIGH);
digitalWrite(leftmotorpin2, LOW);
digitalWrite(rightmotorpin1, HIGH);
digitalWrite(rightmotorpin2, LOW);
Serial.println("Backing Up");
fart();}

//choose which way to turn
void whichway() 
{
totalhalt();
headservo.write(160);
delay(900);
int lDist = MeasuringDistance(); // check left distance
Serial.println("checking left");
Serial.println(lDist);
headservo.write(20); // turn the servo right
delay(900);
int rDist = MeasuringDistance(); // check right distance
Serial.println("checking right");
Serial.println(rDist);
if(lDist < rDist) {
 Serial.println("Decided Right Is Best");
 buzz();// Make him talk
 body_rturn();
 totalhalt();
 currDist = MeasuringDistance(); //measure front distance
 if(currDist < 45) body_rturn();  //if front distance still too small- turn again
}
else{
 Serial.println("Decided Left Is Best");
 buzz();// Make him talk
 body_lturn();
 totalhalt();
 currDist = MeasuringDistance(); //measure front distance
 if(currDist < 45) body_lturn();// if front distance still too small- turn again
   }
return;} 

void totalhalt() {
digitalWrite(leftmotorpin1, HIGH);
digitalWrite(leftmotorpin2, HIGH);
digitalWrite(rightmotorpin1, HIGH);
digitalWrite(rightmotorpin2, HIGH);
Serial.println("Totalhalt!");
headservo.write(90); // set servo to face forward
delay(250);
add1=0; // resets the counter for the nodanger loops
return;
}

//turn left
void body_lturn() {
analogWrite(leftmotorpin1, 0);
analogWrite(leftmotorpin2, 220);
analogWrite(rightmotorpin1, 220);
analogWrite(rightmotorpin2, 0);
Serial.println("Turning Left");
delay(400);
}

//turn right
void body_rturn() {
analogWrite(leftmotorpin1, 220);
analogWrite(leftmotorpin2, 0);
analogWrite(rightmotorpin1, 0);
analogWrite(rightmotorpin2, 220);
Serial.println("Turning Right");
delay(400);
}

void buzz(){
tone(SPEAKER, NOTE_C7, 100);
delay(50);
tone(SPEAKER, NOTE_C6, 100);
}

void fart(){
 tone(SPEAKER, NOTE_C2, 200);
 delay(50);
 tone(SPEAKER, NOTE_C1, 400); 
 delay(100);
 noTone(SPEAKER);
}
