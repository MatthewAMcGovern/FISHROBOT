#include <PID_v1.h>


#include "struct.h"

//DAVIDS STUFF
//global
byte potLpin = A2;
byte potRpin;

byte tailRpin;
byte tailLpin = 3;

//inst
double potL;
double potR;
double potRprev;
double potLprev;
double potLfilt;
double potRfilt;
double potLfiltprev;
double potRfiltprev;
double prevTi;
double omega_c = 1;


double sigR;
double sigL;

//targets for the tails
double targR;
double targL;

//pid constants
double kpR;
double kiR;
double kdR;

double kpL;
double kiL;
double kdL;

//error on tails
double eR;
double eL;
double eRprev;
double eLprev;

int T;
int ti;
//END OF DAVIDS STUFF


void setup() {
  
  Serial.begin(9600);
  ballaskSetup();

  
  
  

 
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //heyLetsTestTheMotor();

  UpdateTailReading();
  //Serial.println(potL);
//  Serial.println(potLfilt);
//  int LSpeed = (int)((double)potL*((double)255/(double)1023));
//  writeToTails(LSpeed,0);
  
  
}
