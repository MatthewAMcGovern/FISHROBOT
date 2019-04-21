//



#include <PID_v1.h>
#include "struct.h"


//com stuff
bool gyroFlag = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("**Welcome to FishOS**");
  ballaskSetup();
  Gyrosetup();
}

void loop() {
//  stepperTest();
//  calibrateBallasts();
//     Serial.println("goodbye");
//COM STUFF

  if (Serial.available() > 0){
    switch (Serial.read()){
    case 'F':
      timer1(1);
      prepareMovement(1, Serial.parseInt());
      setNextInterruptInterval();
      break;
    case 'B':
      timer1(1);
      prepareMovement(0, Serial.parseInt());
      setNextInterruptInterval();
      break;
    case 'D':
      Serial.println(PINB, BIN);
      diagnosticBallast();
      Serial.read();
    case '\n':
      break;
    case 'G':
      gyroFlag = !gyroFlag;  
     default:
      Serial.println("Invalid Character");
       break;
    }
  }
//GYRO STUFF
//  if (gyroFlag==1){
//    gyroloop();
//  }
}
