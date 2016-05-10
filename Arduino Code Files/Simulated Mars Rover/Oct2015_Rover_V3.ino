/* 
Following code by EricWillaim
All build information, code and forum posts/discussion can be found at:
http://mkme.org

Dedicated build page:
http://blog.mkme.org/index.php/arduino-sainsmart-4wd-robot/
Youtube : http://www.youtube.com/mkmeorg

This code will be dedicated to reproducing a Mars type rover at home with simple hardware
This will be set up similar to a space rover in that you must send commands then asess the results prior to executing more commands
Real time remote control is easy- space is much harder and requires some forethought
Hopefully this code will simulate real space missions :)

Rover consists of a 4WD chassis and a servo-mounted ultrasonic distance sensor as well as the additional systems

Oct 20 2015
Began complete code rework for simulated planetary exploration rover
no need for autonymous mode so adapting code to monitor while obeying remote commands (mode removed)
NOTE:  If you just send multiple commands it will only retrieve one at a time from the buffer!  Making multiple commands easy :)

Added SONAR Sweep!
Radar/Sonar Function implemented- code found here:http://www.grook.net/how-to-make-radar-using-arduino-uno
Initiates a full sonar sweep and reports the measurements which can be viewed in Processing (doesnt work with current code due to other serial traffic)
Processing code here for standalone radar:http://www.grook.net/sites/default/files/processing%20code%20-%20Radar.txt

V2
Fixed distance measure using code found at source above for radar (Remember to edit the autonymous code to fix it as well- this one is much better)
Added more serial feedback msgs
added startup serial msgs
Longer delay for servo move on whichway()- Fixed erratic measurements
Added a little startup POST messages just to look cool in serial monitor :)
Need ESP8266 wifi connection- this seems like a great option: http://hackaday.com/2015/09/18/transparent-esp8266-wifi-to-serial-bridge/
http://ez-gui.com/manual/multiwii-clearflight-wifi-to-ezi-gui-how-to/
https://www.reddit.com/r/esp8266/comments/34izb1/is_there_an_easy_way_to_setup_a_port_to_serial/
 Or wireless serial module:
 http://www.instructables.com/id/Arduino-Wireless-Serial-Communication/

V3- Minor Edits and organisation
Added more comments to make it easier for others
verified all sonar and distance direction functionality as well as tested over bluetooth
 
*/

#include <Servo.h> //servo library
#include "pitches.h"// used for the speaker output
#define SPEAKER 4// Speaker Pin

Servo headservo;
const int EchoPin = 2; // Ultrasonic signal input
const int TrigPin = 3; // Ultrasonic signal output
const int leftmotorpin1 = 8; //signal output of Dc motor 
const int leftmotorpin2 = 9;//signal output of Dc motor 
const int rightmotorpin1 = 6;//signal output of Dc motor 
const int rightmotorpin2 = 7;//signal output of Dc motor 
const int HeadServopin = 5; // signal input of headservo
int currDist; // distance
int dur = 1000; //this is the duration for all movements 
int i; // this is the for loop counter in all movements
int pos = 0; // variable to store the servo position for RADAR

void setup() {
  //initialize beeps
  tone(SPEAKER, NOTE_C7, 100);
  
Serial.begin(9600); // Enables Serial monitor for debugging purposes
Serial.println("Exploration Rover Initialized");
Serial.print("O/S Starting .");
delay(1000);
Serial.print(" .");
delay(1000);
Serial.print(" .");
delay(1000);
Serial.println(" .");
Serial.println("System Check Results : OK ");
delay(1000);
Serial.println("Data Link Initialized... "); // Test the Serial communication
Serial.println("Awaiting Commands from mission control");
Serial.println("----------------------------------------------------------------");
pinMode(EchoPin, INPUT);//signal input port
for (int pinindex = 3; pinindex < 10; pinindex++) {
pinMode(pinindex, OUTPUT); // set pins 3 to 10 as outputs
}
// headservo interface
headservo.attach(HeadServopin);
headservo.write(90); //Center the servo
}

void loop() //Do all your stuffs here!
{
GetCommand(); //Get a command from mission control and execute it
//CheckSystems()  Add systems monitoring etc
//SystemsReport() Report out on all the sensor, battery etc
}
 
    
//measure forward distance, unit “cm” Manualy toggle the SONAR sensor pins and calc dist
 float MeasuringDistance() {
  digitalWrite(TrigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(TrigPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(TrigPin, LOW);
  // Distance Calculation
  float distance = pulseIn(EchoPin, HIGH); 
  distance= distance/58; 
  return(distance);
} 
  
  
void GetCommand(){ //Get commands from serial buffer
if (Serial.available() > 0){
int data = Serial.read();	//read serial input commands
   switch(data)
   {
   case '1' : //Set distance to travel per command to lowest
   Serial.println("Set Distance to Lowest Duration- 1000");
   dur = 1000;  
   Serial.println("Awaiting Next Command");
   Serial.println("----------------------------------------------------------------");
   break;
   case '9' : //Set distance to travel to highest
   Serial.println("Set Distance to Highest Duration- 2000");
   dur = 2000;  
   Serial.println("Awaiting Next Command");
   Serial.println("----------------------------------------------------------------");
   break;
   case 'f' : //Forward move rover
   Serial.println("Forward Move Rover Command Received");
   moveForward(); 
   break;
   case 'b' : //Reverse the rover
   back(); 
   break;
   case 'l' : // left turn rover
   lturn(); 
   break;
   case 'r' : //right turn rover
   rturn(); 
   break;
   case 's' : //Stop rover and apply brakes
   Serial.println("Manual STOP / BRAKES Command Received");
   totalhalt(); 
   Serial.println("----------------------------------------------------------------");
   break;
   case 'v' : //Perform a Radar/Sonar Sweep
   radar(); 
   break;
   case 'w' :  //Measure 3 distances for left right and centre
   whichway(); 
   break; }}
 }

void moveForward() { //Move the rover in the forward direction while monitoring distance
 headservo.write(90); //set pan servo to middle
 delay(1000);//wait untill servo gets there
 currDist = MeasuringDistance(); //measure front distance
 Serial.print("Current Forward Distance: ");
 Serial.println(currDist);
if(currDist > 35) {
 Serial.println("Forward Path is Clear - Proceeding Forward ");}
else if(currDist < 35){
  Serial.println("*******FORWARD BLOCKED****** Cannot Execute Forward Movement");
  Serial.println("Awaiting Next Command");
  Serial.println("----------------------------------------------------------------");
  return;
}
  //For loop in all movements which the duration is set by the dur integer
for (int i=0; i <= (dur/25); i++){ //Remember to Scale DUR as forward movement takes way longer due to ongoing distance measurement /20 too long sometimes
  currDist = MeasuringDistance(); //measure front distance
  if(currDist > 35) {
   //Serial.println(i); Uncomment this to see the for loop counter for debugging
   analogWrite(leftmotorpin1, 0);//Changed these to analog write for slower
   analogWrite(leftmotorpin2, 120);
   analogWrite(rightmotorpin1, 0);
   analogWrite(rightmotorpin2, 120);}
 else {Serial.println("Forward now blocked! - Stopping");
      Serial.print("Steps Moved Before Being Blocked: ");
      Serial.println(i);
   totalhalt();
   break;}
}
Serial.print("Finished Forward Travel Duration: ");
Serial.println(dur);
totalhalt();
Serial.println("Awaiting Next Command");
Serial.println("----------------------------------------------------------------");
}

void back() { //Reverse the rover
  Serial.println("Reverse Move Rover Command Received");
  for (int i=0; i <= (dur*10); i++){ //remember the dur integer is scaled because forward movement takes much longer due to measurements
  analogWrite(leftmotorpin1, 120);
  analogWrite(leftmotorpin2, 0);
  analogWrite(rightmotorpin1, 120);
  analogWrite(rightmotorpin2, 0);}
  Serial.print("Completed Reverse Movement Duration: ");
  Serial.println(dur); //print the duration of move
  totalhalt();
  Serial.println("Awaiting Next Command");
  Serial.println("----------------------------------------------------------------");
}

//Turn Left
void lturn() {
  Serial.println("Left Turn Rover Command Received");
  for (int i=0; i <= (dur*5); i++){
  analogWrite(leftmotorpin1, 0);
  analogWrite(leftmotorpin2, 190);
  analogWrite(rightmotorpin1, 190);
  analogWrite(rightmotorpin2, 0);
  }
  Serial.print("Completed Left Movement Duration: ");
  Serial.println(dur);
  totalhalt();
  Serial.println("Awaiting Next Command");
  Serial.println("----------------------------------------------------------------");
}

//Turn Right
void rturn() {
  Serial.println("Right Turn Rover Command Received");
  for (int i=0; i <= (dur*5); i++){ //Scaled dured by factor to lengthen
  analogWrite(leftmotorpin1, 190);
  analogWrite(leftmotorpin2, 0);
  analogWrite(rightmotorpin1, 0);
  analogWrite(rightmotorpin2, 190);
  //Serial.println("Turning Right");
  }
  Serial.print("Completed Right Movement Duration: ");
  Serial.println(dur);
  totalhalt();
  Serial.println("Awaiting Next Command");
  Serial.println("----------------------------------------------------------------");
}

//Check directions on demand for clearest path if we need to rely on sensor instead of video downlink
void whichway() {//Measure Distance in all directions
  Serial.println("Measure Left/Right Distances Command Received");
  totalhalt();
  headservo.write(130); //This works best on current robot at 130 to 50 to avoid reflections from the wheels
  delay(1100);
  int lDist = MeasuringDistance(); // check left distance
  Serial.print("Left Distance: ");
  Serial.println(lDist);
  headservo.write(50); // turn the servo right
  delay(1100);
  int rDist = MeasuringDistance(); // check right distance
  Serial.print("Right distance: ");
  Serial.println(rDist);
  headservo.write(90);
  delay(1100);
  int FDist = MeasuringDistance();
  Serial.print("Forward distance: ");
  Serial.println(FDist);
  Serial.println("Awaiting Next Command");
  Serial.println("----------------------------------------------------------------");
} 


void totalhalt() { // This applies the brakes so rover cannot move   
  digitalWrite(leftmotorpin1, HIGH);
  digitalWrite(leftmotorpin2, HIGH);
  digitalWrite(rightmotorpin1, HIGH);
  digitalWrite(rightmotorpin2, HIGH);
  //Serial.println("Totalhalt!");
  //headservo.write(90); // set servo to face forward
  //delay(250);
  return;}

void buzz(){ //make some noise
  tone(SPEAKER, NOTE_C7, 100);
  delay(50);
  tone(SPEAKER, NOTE_C6, 100);}

void blip(){ //make some noise
 tone(SPEAKER, NOTE_C2, 200);
 delay(50);
 tone(SPEAKER, NOTE_C1, 400); 
 delay(100);
 noTone(SPEAKER);}
 
 //------------------------------------------- RADAR/SONAR Functions----------------------------------------
 void radar(){
 Serial.println("SONAR Sweep Command Received");
 Serial.println("SONAR Sweep Initiated...");
 headservo.write(20); // tell servo to go to position in variable 'pos'
 delay(2000);
 for(pos = 20; pos <= 160; pos += 1) // goes from 20 degrees to 160 degrees
  { // in steps of 1 degree
   headservo.write(pos); // tell servo to go to position in variable 'pos'
   Print(MeasuringDistance() , pos);    
   delay(10); // waits 15ms for the servo to reach the position   
  }
  delay(1000); //Give a rest before starting the reverse arc to avoid false measurements
  Serial.println("---------Sweep Back--------------"); 
  for(pos = 160; pos >= 20; pos -= 1) // goes from 160 degrees to 20 degrees
  { // in steps of 1 degree
   headservo.write(pos); // tell servo to go to position in variable 'pos'
   Print(MeasuringDistance() , pos);    
   delay(10); // waits 15ms for the servo to reach the position 
 }
 delay(500); //Wait 500 milisec so it doesnt jump right to centre
 headservo.write(90);// Return the servo to centre
 Serial.println("SONAR Sweep Complete- Awaiting next command...");
 Serial.println("----------------------------------------------------------------");
  }
  
void Print (int R , int T)  //functions used for printing radar sweep values
  {
   Serial.print(R);Serial.print(", ");
   Serial.print(T);Serial.println(".");
   delay(100);
  }
//--------------------------------------------END RADAR/SONAR FUNCTIONS-----------------------------------------
