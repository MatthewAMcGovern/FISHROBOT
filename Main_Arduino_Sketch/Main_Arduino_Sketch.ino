#include <PID_v1.h>
#include "struct.h"
#include "CustomGyroFish.h"

//com stuff
bool gyroFlag = 1;
bool moveBallastFlag = 0;
double Setpoint = 72;
double Input, Output;
double Kp=2, Ki=5, Kd=1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void setup() {
  Serial.begin(9600);
  Serial.println("**Welcome to FishOS**");
  ballaskSetup();
  gsetup();
  myPID.SetMode(AUTOMATIC);
  myPID.SetSampleTime(500);
  
}

void loop() {

//COM STUFF
  while (Serial.available()){
    switch (Serial.read()){
    case 'C':
      calibrateBallast();
    case 'F': //Front Ballast
      prepareMovement(1, Serial.parseInt());
      moveBallastFlag = true;
      break;
    case 'B': //Back Ballast
      prepareMovement(0, Serial.parseInt());
      moveBallastFlag = true;
      break;
    case 'D': //Diagnostic 
//      stepperTest();
      Serial.println(PINB, BIN);
      diagnosticBallast();
      Serial.read();
      break;
//    case 'G':
//      gyroFlag = !gyroFlag;
//      break;
    //META CASES
    case 'S':
      Setpoint = double(Serial.parseFloat());
      break;
    case 'M':
      setNextInterruptInterval();
      break;
    case '\n':
      if (moveBallastFlag){
        timer1(1);
        setNextInterruptInterval();
        moveBallastFlag = false;
        Serial.println("Moving");
      }
      break;
    default:
      Serial.println("Invalid Character");
      break;
    }
  }
// End Coms stuff
//
if (digitalRead(13)==0){
  Serial.println("!flooding! !in! !engine! !room! !lower! !level!");
  timer1(1);
  prepareMovement(1, 1100);
  prepareMovement(0, 1100);
  setNextInterruptInterval();
}
//GYRO STUFF
//  if(gyroFlag){
//    gloop3();
//    //Serial.println(globRoll);
//  }
//PID STUFF
//  Input = globRoll;
//  myPID.Compute();
//  Serial.print(Output);
//  Serial.print(' ');
//  Serial.println(Input);
//BallastPIDCompute(Output,500.0);
}
