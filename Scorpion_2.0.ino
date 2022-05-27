#include "Scorpion.h"
#include <HCSR04.h>

int DutyCycle = 200;
int FrontWall = 20, RightWall = 20, LeftWall = 20, RoadWidth = 100, SideSpace = 20; //Declaring Sonar sensor variable
int IRA = 19, IRB = 18, IRC = 5, IRD = 17, IRE = 16; //IR variable for declaring GPIO Pin
int A = 0, B = 0, C = 0, D = 0, E = 0, AIR; //IR variable for store value

bool obstacle = false;

HCSR04 sonarA(22, 23); //Front Sonor - initialisation class HCSR04 (trig pin , echo pin)
HCSR04 sonarB(2, 15); //Right Sonor - initialisation class HCSR04 (trig pin , echo pin)
HCSR04 sonarC(21, 4); //Left Sonor - initialisation class HCSR04 (trig pin , echo pin)

//Using class "Motor" {methods = Forward, Backward, Stop, Speed, Status}
Motor MotorR(27, 14, 26, 0);  // Right Motor - (in1, in2, en, pwm channel)
Motor MotorL(33, 25, 32, 1);  // Left Motor - (inputpin1, inputpin2, enablepin, pwmChannel[0-18])

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(IRA, INPUT);
  pinMode(IRB, INPUT);
  pinMode(IRC, INPUT);
  pinMode(IRD, INPUT);
  pinMode(IRE, INPUT);

  MotorR.Speed(DutyCycle);
  MotorL.Speed(DutyCycle);
  MotorR.Release();
  MotorL.Release();
  delay(1000);
  MotorR.Forward();
  MotorL.Forward();
}

void loop() {
  //ReadSonar(); // reading sonar data
  ReadIR(); // reading IR data
  FrontWall = sonarA.dist();
  Serial.print(":FrontWall=");
  Serial.print(FrontWall);
  
  AIR = A + B + C + D + E; // sum of all IR sensor


  if (FrontWall >= 25){
    if (AIR == 4)
    {
      if (A == 0) {
        _90dLeft();
      }
      else if (B == 0) {
        MedLeft();
      }
      else if (C == 0) {
        Straight();
      }
      else if (D == 0) {
        MedRight();
      }
      else if (E == 0) {
        _90dRight();
      }
    }
    else if (AIR == 3)
    {
      if (B == 0) {
        (A == 0) ? SharpLeft() : LowLeft();
      }
      else if (D == 0) {
        (C == 0) ? LowRight() : SharpRight();
      }
    }
    else if (AIR == 2 || AIR == 1) {
      (A == 1) ? _90dRight() : _90dLeft();
    }
    else if (AIR == 0)
    {
      DefaultTurn();
    }
    else if(AIR == 5)
    {
      Straight();
      delay(500);
      CarStop(); // Speed 0 with forward gear
      ReadIR(); ReadSonar();
      AIR = A + B + C + D + E;
      
      if(AIR =! 5){Straight();} // if found track. It was a blank track.
      else if(AIR == 5) // if no track
      {
        if(LeftWall <= 50 || RightWall <= 50)// if no track & found walls
        {
          // follow walls until the track is founded
          do{
            ReadIR();
            AIR = A + B + C + D + E;
            PassThroughWalls();  
            }
          while(AIR == 5);
        } 
        else{_180dturn();} //if no track and no Side Walls. The track ends here.
      }
    }  
  }
  else if(FrontWall <= 5){
    AvoidObstacle();
  }
}

//*** Straight Forward
void Straight() {
  MotorR.Speed(255);
  MotorL.Speed(255);
}

//*** Car speed 0 with with forward gear
void CarStop() {
  MotorR.Speed(0);
  MotorL.Speed(0);
}

//*** Car stop and neutral
void CarRelease() {
  MotorR.Release();
  MotorL.Release();
}

//*** Low Left Turn
void LowLeft() {
  MotorL.Speed(230);
  MotorR.Speed(255);
}

//*** Medium Left Turn
void MedLeft() {
  MotorL.Speed(200);
  MotorR.Speed(255);
}

//*** Low Left Turn
void SharpLeft() {
  MotorL.Speed(0);
  MotorR.Speed(255);
}

//*** Low Right Turn
void LowRight() {
  MotorR.Speed(230);
  MotorL.Speed(255);
}

//*** Medium Right Turn
void MedRight() {
  MotorR.Speed(200);
  MotorL.Speed(255);
}

//*** Sharp Right Turn
void SharpRight() {
  MotorR.Speed(0);
  MotorL.Speed(255);
}

//*** 90d left turn
void _90dLeft() {
  MotorR.Speed(255); MotorL.Speed(0);
  do {
    ReadIR();
    AIR = A + B + C + D + E;
  }
  while (!(AIR == 4 && C == 0));
}

//*** 90d Right Turn
void _90dRight() {
  MotorL.Speed(255); MotorR.Speed(0);
  do {
    ReadIR();
    AIR = A + B + C + D + E;
  }
  while (!(AIR == 4 && C == 0));
}

//*** 180d turn on place
void _180dturn() {
  CarRelease(); // Both motor stop with neutral gear
  MotorR.Speed(255); MotorL.Speed(255);
  delay(68);
  MotorL.Forward(); MotorR.Backward();// Rotate on place
  do {
    ReadIR();
    AIR = A + B + C + D + E;
  }
  while (!(AIR == 4 && C == 0)); // finish 180d ?
  CarRelease(); // Both motor stop with neutral gear
  delay(68);
  Straight(); // Forward gear
}

//*** Default turn
void DefaultTurn(){
   true ? _90dRight() : _90dLeft();
}


//*** Move straight forward in the walls by middle
void PassThroughWalls(){
  ReadSonar();
  RoadWidth = LeftWall + RightWall; // total side gap
  SideSpace = RoadWidth/2 - 2; // average side gap for each side
  
  if(RoadWidth <= 120){ //when the sensor can count distance. Go by the middle of path
    if (LeftWall <= SideSpace) { // car is not in middle of the. 
    MedRight();
    }
    else if (RightWall <= SideSpace) { // car is not in middle of the walls
    MedLeft();
    }
    else{ // car is now in middle of the walls
      Straight();
    }
  }
}


//*** Reading all Sonar sensor
void ReadSonar() {
  RightWall = sonarB.dist();
  LeftWall = sonarC.dist();
  Serial.print(" :RightWall= ");
  Serial.print(RightWall);
  Serial.print(" :LeftWall=");
  Serial.println(LeftWall);
}

//*** Read all IR sensor
void ReadIR() {
  A = digitalRead(IRA); // IR Sensor output pin connected to D1
  B = digitalRead(IRB); // IR Sensor output pin connected to D1
  C = digitalRead(IRC); // IR Sensor output pin connected to D1
  D = digitalRead(IRD); // IR Sensor output pin connected to D1
  E = digitalRead(IRE); // IR Sensor output pin connected to D1
  Serial.println(" ");
  Serial.print(":A=");
  Serial.print(A);
  Serial.print(":B=");
  Serial.print(B);
  Serial.print(":C=");
  Serial.print(C); 
  Serial.print(":D=");
  Serial.print(D);
  Serial.print(":E=");
  Serial.print(E);
}

void AvoidObstacle() {

}

