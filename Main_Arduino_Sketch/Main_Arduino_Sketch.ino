#include <MPU6050.h>
#include <PID_v1.h>
#include "struct.h"
#include "CustomGyroFish.h"

//com stuff
bool gyroFlag = 0;
bool moveBallastFlag = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("**Welcome to FishOS**");
  ballaskSetup();
}

void loop() {

//COM STUFF
  while (Serial.available()){
    switch (Serial.read()){
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
    case 'G':
      gyroFlag = !gyroFlag;
      break;
    //META CASES
    case '\n':
      if (moveBallastFlag){
        timer1(1);
        setNextInterruptInterval();
        moveBallastFlag = false;
        Serial.println("Larry");
      }
      break;
    default:
      Serial.println("Invalid Character");
      break;
    }
  }
}
