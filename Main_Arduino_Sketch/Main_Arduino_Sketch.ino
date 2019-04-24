//describes all necessary data in one read cycle used to store raw and filtered data
struct TailDataFrame{
  double potL;
  double potR;
  double Lvel;
  double Rvel;
  double ReadTime;
  double T;
  
};

//describes all necessary data for determining PID output for given time step
struct PIDDataFrame{
  double LE;
  double RE;
  double REvel;
  double LEvel;
  double REint;
  double LEint;
  
};

//store data on setpoint behavior
struct SetFrame{
  double Lset;
  double Rset;
  double LSetVel;
  double RSetVel;
};


//DAVIDS STUFF
//global
byte potLpin = A2;
byte potRpin = A3;

byte tailRpin;
byte tailLpin = 3;

//Read Glob vars

const int s = 5;

TailDataFrame hist[s];


//Low pass
double potLfilt;
double potRfilt;
double potLfiltprev;
double potRfiltprev;
double omega_c = 1;


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

bool REngaged = false;
bool LEngaged = false;
bool RStartVel = false; //false for negative
bool LStartVel = false; //true for positive

//error on tails


//potentiometer bounds
int LUbound = 581;
int LLbound = 476;
int RUbound = 0;
int RLbound = 0;
double Rmid;
double Lmid;



//TIME Glob vars
double T;
double TIME;
double pTIME;

//Moving Avg

TailDataFrame Filt[s];


//TEMP VAR

int LSpeed;


//END OF DAVIDS STUFF


void setup() {
  
  Serial.begin(9600);  
  setupTailSys();
  
//  delay(3000);
//  LSpeed = 250;
//  writeToTails(LSpeed,0);
//  delay(7000);
//  LSpeed = 80;
//  writeToTails(LSpeed,0);
//  delay(7000);
  writeToTails(0,0);
  delay(3000);
  
StartSequenceBlocking();
}

void loop() {
  // put your main code here, to run repeatedly:
  //heyLetsTestTheMotor();

  UpdateTailReading();
//  Serial.println(potL);
//  LSpeed = 105;
  Serial.print(Filt[0].potL);
  Serial.print(" ");
  Serial.print(Filt[0].potR);
  Serial.print(" ");
  Serial.print(millis());
  Serial.print(" ");
  int LSpeed = (int)((double)hist[0].potR*((double)255/(double)1023));
  
  writeToTails(LSpeed,0);
  
  
}
