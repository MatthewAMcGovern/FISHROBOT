// For RAMPS 1.4
#include <PID_v1.h>

#define A_DIR_PIN          5
#define A_STEP_PIN         2
#define A_LIMIT            4



#define B_DIR_PIN          6
#define B_STEP_PIN         3
#define B_LIMIT            7



#define ENABLE_PIN         8



//Macro to quickly switch pins high/low
#define A_STEP_HIGH             PORTD |=  0b00000100;
#define A_STEP_LOW              PORTD &= ~0b00000100;

#define B_STEP_HIGH             PORTD |=  0b00001000;
#define B_STEP_LOW              PORTD &= ~0b00001000;

//Turns Interrupt1 on/off
#define TIMER1_INTERRUPTS_ON    TIMSK1 |=  (1 << OCIE1A);
#define TIMER1_INTERRUPTS_OFF   TIMSK1 &= ~(1 << OCIE1A);

//PID SECTION BEGINS
double Setpoint = 72.46;
double PIDInput, PIDOutput = 127;
double Kp=2, Ki=5, Kd=1;

PID myPID(&PIDInput, &PIDOutput, &Setpoint, Kp, Ki, Kd, DIRECT);

void aStep() {
  A_STEP_HIGH
  A_STEP_LOW
}
void aDir(int dir) {
  digitalWrite(A_DIR_PIN, dir);
}

void bStep() {
  B_STEP_HIGH
  B_STEP_LOW
}
void bDir(int dir) {
  digitalWrite(B_DIR_PIN, dir);
}



#define NUM_STEPPERS 2

volatile stepperInfo steppers[NUM_STEPPERS];

void ballaskSetup() {
  pinMode(A_STEP_PIN,   OUTPUT);
  pinMode(A_DIR_PIN,    OUTPUT);
  pinMode(A_LIMIT, INPUT_PULLUP);
  pinMode(B_STEP_PIN,   OUTPUT);
  pinMode(B_DIR_PIN,    OUTPUT);
  pinMode(B_LIMIT, INPUT_PULLUP);
  pinMode(ENABLE_PIN, OUTPUT);

  digitalWrite(ENABLE_PIN, LOW);

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 1000;                             // compare value
  TCCR1B |= (1 << WGM12);                   // CTC mode
  TCCR1B |= ((1 << CS11) | (1 << CS10));    // 64 prescaler
  interrupts();

  steppers[0].dirFunc = bDir;
  steppers[0].stepFunc = bStep;
  steppers[0].acceleration = 4000;
  steppers[0].minStepInterval = 600;

  steppers[1].dirFunc = aDir;
  steppers[1].stepFunc = aStep;
  steppers[1].acceleration = 4000;
  steppers[1].minStepInterval = 600;

  TIMER1_INTERRUPTS_ON

  //PID
  myPID.SetMode(AUTOMATIC);
  myPID.SetSampleTime(500);
}

void resetStepper(volatile stepperInfo& si) {
  si.c0 = si.acceleration;
  si.d = si.c0;
  si.di = si.d;
  si.stepCount = 0;
  si.n = 0;
  si.rampUpStepCount = 0;
  si.movementDone = true;
}

volatile byte remainingSteppersFlag = 0;

void prepareMovement(int whichMotor, int steps) {
  volatile stepperInfo& si = steppers[whichMotor];
  si.dirFunc( steps < 0 ? LOW : HIGH );
  si.dir = steps > 0 ? 1 : -1;
  si.totalSteps = abs(steps);
  resetStepper(si);
  remainingSteppersFlag |= (1 << whichMotor);
}

volatile byte nextStepperFlag = 0;

volatile int ind = 0;
volatile unsigned int intervals[100];

void setNextInterruptInterval() {

  bool movementComplete = true;

  unsigned int mind = 999999;
  for (int i = 0; i < NUM_STEPPERS; i++) {
    if ( ((1 << i) & remainingSteppersFlag) && steppers[i].di < mind ) {
      mind = steppers[i].di;
    }
  }

  nextStepperFlag = 0;
  for (int i = 0; i < NUM_STEPPERS; i++) {
    if ( ! steppers[i].movementDone )
      movementComplete = false;

    if ( ((1 << i) & remainingSteppersFlag) && steppers[i].di == mind )
      nextStepperFlag |= (1 << i);
  }

  if ( remainingSteppersFlag == 0 ) {
    OCR1A = 65500;
  }

  OCR1A = mind;
}

ISR(TIMER1_COMPA_vect)
{
  unsigned int tmpCtr = OCR1A;

  OCR1A = 65500;
  
  for (int i = 0; i < NUM_STEPPERS; i++) {

    volatile stepperInfo& s = steppers[i];

    if ( ! ((1 << i) & remainingSteppersFlag) )
      continue;

    if ( ! (nextStepperFlag & (1 << i)) ) {
      steppers[i].di -= tmpCtr;
      continue;
    }

//    LIMIT CONTROL CODE
    bool endLimitReached = false;
//    s.limitTrip = ((~PINB) & (1<<(2-i)));
//    if (s.limitTrip & 0>s.dir)
//    {
//      resetStepper(s);
//      s.stepPosition = 0;
//      remainingSteppersFlag  &= ~(1 << i);
//      endLimitReached = true;
//    }
//    if (s.stepPosition >= 1100 && 0<s.dir){
//      resetStepper(s);
//      s.stepPosition = 1100;
//      remainingSteppersFlag  &= ~(1 << i);
//      endLimitReached = true;
//    }
  
    if (!endLimitReached){
      if ( s.stepCount < s.totalSteps ) {
        s.stepFunc();
        s.stepCount++;
        s.stepPosition += s.dir;
        if ( s.stepCount >= s.totalSteps ) {
          s.movementDone = true;
          remainingSteppersFlag &= ~(1 << i);
        }
      }
  
  
      if ( s.rampUpStepCount == 0 ) {
        s.n++;
        s.d = s.d - (2 * s.d) / (4 * s.n + 1);
        if ( s.d <= s.minStepInterval ) {
          s.d = s.minStepInterval;
          s.rampUpStepCount = s.stepCount;
        }
        if ( s.stepCount >= s.totalSteps / 2 ) {
          s.rampUpStepCount = s.stepCount;
        }
      }
      else if ( s.stepCount >= s.totalSteps - s.rampUpStepCount ) {
        s.d = (s.d * (4 * s.n + 1)) / (4 * s.n + 1 - 2);
        s.n--;
      }
  
      s.di = s.d; // integer
    }
  }

  setNextInterruptInterval();

  TCNT1  = 0;
}

void stepperTest() {
  TIMER1_INTERRUPTS_ON

  prepareMovement(1, -10);
  prepareMovement(0, 100);
  runAndWait();
  prepareMovement(1, 100);
  prepareMovement(0, -100);
  runAndWait();

  TIMER1_INTERRUPTS_OFF
}

void runAndWait() {
  setNextInterruptInterval();
  while ( remainingSteppersFlag );
}


void timer1(bool i){
  if (i){
    TIMER1_INTERRUPTS_ON
  }
  else{
    TIMER1_INTERRUPTS_OFF
  }
}
void calibrateBallast(){
  timer1(1);
  prepareMovement(1, -1200);
  prepareMovement(0, -1200);
  runAndWait();
  prepareMovement(1, 1100);
  prepareMovement(0, 1100);
  runAndWait();
  
}
void diagnosticBallast(){
  Serial.println(steppers[0].limitTrip, BIN);
  Serial.println(steppers[1].limitTrip, BIN);
  Serial.println(steppers[0].stepPosition);
  Serial.println(steppers[1].stepPosition);
}

void ballastMoveToPos(int i, double absPos){
  prepareMovement(i, absPos-steppers[i].stepPosition);
}

void BallastPIDCompute(double boyancyTarget){
  getRoll();
  PIDInput = double(getRoll());
  myPID.Compute();
  Serial.println(PIDInput);
  Serial.println(PIDOutput-127);
  ballastMoveToPos(1,int(boyancyTarget - PIDOutput-127));
  ballastMoveToPos(0,int(boyancyTarget + PIDOutput-127));  
  setNextInterruptInterval();
}
void setSetpoint(){
  Setpoint = double(globRoll);
}
