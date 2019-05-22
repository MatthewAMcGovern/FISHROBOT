#include "struct.h"
#include "CustomGyroFish.h"
#include <PID_v1.h>
//com stuff
bool PIDflag = 0;
bool moveBallastFlag = 0;
bool flooding = 0;
double boyancyTarget = 600;

void setup() {
  Serial.begin(9600);
  Serial.println("**Welcome to FishOS**");
  ballaskSetup();
  gsetup();  
}

void loop() {

//COM STUFF
  while (Serial.available()){
    switch (Serial.read()){
    case 'K':
      setSetpoint();
      break;
    case 'C':
      calibrateBallast();
      break;
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
//      Serial.println(PINB, BIN);
      diagnosticBallast();
//      Serial.read();
      getRoll();
      Serial.println(globRoll);
      break;
    case 'P':
      BallastPIDCompute(boyancyTarget);  
//      PIDflag = !PIDflag;
      break;
    case 'S':
      boyancyTarget = double(Serial.parseInt());
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
  // Engine Flood shit
  if (digitalRead(13)==1){
    PIDflag = 0;
    Serial.println("!flooding! !in! !engine! !room! !lower! !level!");
  }
//PID STUFF
  
  if (PIDflag){
    getRoll();
  }
//  Serial.println(getRoll());
}
