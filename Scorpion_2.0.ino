#include "Scorpion.h"
#include <HCSR04.h>

int wrt = 555; // wrt = whell rotation time. time for rotating two time in mls
int DutyCycle = 200, low_speed = 200, med_speed = 230, max_speed = 255;
int FrontWall = 20, RightWall = 100, LeftWall = 100, RoadWidth = 100, SideSpace = 20; //Declaring Sonar sensor variable
int IRA = 19, IRB = 18, IRC = 5, IRD = 17, IRE = 16; //IR variable for declaring GPIO Pin
int A = 0, B = 0, C = 0, D = 0, E = 0, AIR; //IR variable for store value
int dt = 1; // default turn (1 = right, 0   = left).

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

  Neutral();
  MotorR.Speed(DutyCycle);
  MotorL.Speed(DutyCycle);

  delay(100);
  MotorR.Forward();
  MotorL.Forward();
}

//*** Default turn
void DefaultTurn() {
  (dt == 1) ? _90dRight() : _90dLeft();
}

void loop() {
  //ReadSonar(); // reading sonar data
  ReadIR(); // reading IR data
  //FrontWall = sonarA.dist();
  //Serial.print(":FrontWall=");
  //Serial.print(FrontWall);

  if (FrontWall >= 20) {
    if (AIR == 4)
    {
      (A == 0) ? _90dLeft() : (B == 0) ? MedLeft() : (C == 0 ) ? Straight() : ( D == 0 )? MedRight() : _90dRight(); 
//      if (A == 0) {
//        _90dLeft();
//      }
//      else if (B == 0) {
//        MedLeft();
//      }
//      else if (C == 0) {
//        Straight();
//      }
//      else if (D == 0) {
//        MedRight();
//      }
//      else if (E == 0) {
//        _90dRight();
//      }
    }
    else if (AIR == 3)
    {
      if (B == 0) {
        (A == 0) ? SharpLeft() : SmoothLeft();
      }
      else if (D == 0) {
        (C == 0) ? SmoothRight() : SharpRight();
      }
    }
    else if (AIR == 2 || AIR == 1) {
      (A == 1) ? _90dRight() : _90dLeft();
    }
    else if (AIR == 0)//multiple line
    {
      DefaultTurn();
    }
    else if (AIR == 5)// White space
    {
      //Serial.println("white space!");delay(500);
      Straight(); //go 14cm forward
      delay(wrt);
      Brake(); // Speed 0 with forward gear

      ReadIR();
      //ReadSonar();

      if (AIR < 5) {
        //Serial.println("Track Found"); delay(500);
        Straight(); // if found track. It was a blank track.
      }
      else if (AIR == 5) // if no track
      {
        //Serial.println("No Track Found! Wall checking"); delay(500);
        if (LeftWall <= 50 && RightWall <= 50) // if no track & found walls
        {
          //Serial.println("Wall found!"); delay(500);
          // follow walls until the track is founded
          do {
            ReadIR();
            PassThroughWalls();
          }
          while (AIR == 5);
        }
        else if(LeftWall >= 100 && RightWall >= 100){
          //Serial.println("No track and no wall found!"); delay(1000);
          _180dturn(); //if no track and no Side Walls. The track ends here.
        }
      }
    }
  }
  else if (FrontWall <= 5) {
    AvoidObstacle();
  }
}

//*** Straight Forward
void Straight() {
  MotorR.Speed(max_speed);
  MotorL.Speed(max_speed);
}

//*** Car speed 0 with with forward gear
void Brake() {
  MotorR.Speed(0);
  MotorL.Speed(0);
}

//*** Car stop and neutral
void Neutral() {
  MotorR.Release();
  MotorL.Release();
}

//*** Smooth Left Turn
void SmoothLeft() {
  MotorL.Speed(med_speed);
  MotorR.Speed(max_speed);
}

//*** Medium Left Turn
void MedLeft() {
  MotorL.Speed(low_speed);
  MotorR.Speed(max_speed);
}

//*** Sharp Left Turn
void SharpLeft() {
  MotorL.Speed(0);
  MotorR.Speed(max_speed);
}

//*** Smooth Right Turn
void SmoothRight() {
  MotorR.Speed(med_speed);
  MotorL.Speed(max_speed);
}

//*** Medium Right Turn
void MedRight() {
  MotorR.Speed(low_speed);
  MotorL.Speed(max_speed);
}

//*** Sharp Right Turn
void SharpRight() {
  MotorR.Speed(0);
  MotorL.Speed(max_speed);
}

//*** 90d left turn
void _90dLeft() {
  MotorR.Speed(max_speed); MotorL.Speed(0);
  do {
    ReadIR();
  }
  while (!(AIR == 4 && C == 0));
}

//*** 90d Right Turn
void _90dRight() {
  MotorL.Speed(max_speed); MotorR.Speed(0);
  do {
    ReadIR();
  }
  while (!(AIR == 4 && C == 0));
}

//*** 180d turn on place
void _180dturn() {
  //Serial.println("Taking U turn"); delay(2000);
  Neutral(); // Both motor stop with neutral gear
  delay(68);
  MotorL.Forward(); MotorR.Backward();// Rotate on place
  MotorR.Speed(max_speed); MotorL.Speed(max_speed);
  do {
    ReadIR();
    AIR = A + B + C + D + E;
  }
  while (!(AIR == 4 && C == 0)); // finish 180d ?
  Brake(); // Both motor stop with neutral gear
  Neutral();
  delay(68);
  MotorL.Forward(); MotorR.Forward();
}

//*** Move straight forward in the walls by middle
void PassThroughWalls() {
  ReadSonar();
  RoadWidth = LeftWall + RightWall; // total side gap
  SideSpace = RoadWidth / 2 - 2; // average side gap for each side

  if (RoadWidth <= 120) { //when the sensor can count distance. Go by the middle of path
    if (LeftWall <= SideSpace) { // car is not in middle of the.
      MedRight();
    }
    else if (RightWall <= SideSpace) { // car is not in middle of the walls
      MedLeft();
    }
    else { // car is now in middle of the walls
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
  AIR = A + B + C + D + E;

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
  Serial.print(":AIR=");
  Serial.print(AIR);
}

//***Avoid obstacle if found
//for default turn == left
void AvoidObstacle() {
  MotorL.Speed(0); MotorR.Speed(max_speed);// Left turn
  delay(wrt);
  MotorR.Speed(0); MotorL.Speed(max_speed);// Right turn
  delay(wrt);
  MotorR.Speed(max_speed);// Straight forward
  delay(wrt);
  do {
    ReadIR();
  }
  while (AIR > 3); // untill tow sensor track the line
  Brake();
}
