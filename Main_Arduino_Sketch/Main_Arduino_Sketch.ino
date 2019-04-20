#include <PID_v1.h>
#include "struct.h"

void setup() {
  Serial.begin(9600);
  Serial.println("hello");
  ballaskSetup();
}

void loop() {
//  stepperTest();
//  calibrateBallasts();
//     Serial.println("goodbye");
  if (Serial.available() > 0){
    switch (Serial.read()){
    case 'F':
      timer1(1);
      prepareMovement(1, -Serial.parseInt());
      runAndWait();
      timer1(0);
      Serial.println("did I move?");
      break;
    case 'B':
      timer1(1);
      prepareMovement(0, Serial.parseInt());
      runAndWait();
      timer1(0);
      Serial.println("did I move?");
      break;
    case 'D':
      Serial.println(PINB, BIN);
      Serial.println(digitalRead(9));
      Serial.println(digitalRead(10));      
      Serial.read();  
     default:
       Serial.println("Invalid Character");
       break;
    }
  }
}
