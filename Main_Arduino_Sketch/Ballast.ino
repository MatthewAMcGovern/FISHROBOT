//SEE 'iforce2d coridinated stepper motor control' for partial source code
//This code takes control of timer1. Do not use timer1 for anything other than stepper control
#define A_DIR_PIN          4
#define A_STEP_PIN         5
#define A_ENABLE_PIN       6

#define B_DIR_PIN          7
#define B_STEP_PIN         8
#define B_ENABLE_PIN       9

//Macro to quickly switch pins high/low
#define A_STEP_HIGH             PORTD |=  0b00010000;
#define A_STEP_LOW              PORTD &= ~0b00010000;

#define B_STEP_HIGH             PORTB |=  0b10000000;
#define B_STEP_LOW              PORTB &= ~0b10000000;

//Turns Interrupt1 on/off
#define TIMER1_INTERRUPTS_ON    TIMSK1 |=  (1 << OCIE1A);
#define TIMER1_INTERRUPTS_OFF   TIMSK1 &= ~(1 << OCIE1A);

//Group stepper information
struct stepperInfo {
  // externally defined parameters
  float acceleration;
  volatile unsigned int minStepInterval;   // ie. max speed, smaller is faster
  void (*dirFunc)(int);
  void (*stepFunc)();

  // derived parameters
  unsigned int c0;                // step interval for first step, determines acceleration
  long stepPosition;              // current position of stepper (total of all movements taken so far)

  // per movement variables (only changed once per movement)
  volatile int dir;                        // current direction of movement, used to keep track of position
  volatile unsigned int totalSteps;        // number of steps requested for current movement
  volatile bool movementDone = false;      // true if the current movement has been completed (used by main program to wait for completion)
  volatile unsigned int rampUpStepCount;   // number of steps taken to reach either max speed, or half-way to the goal (will be zero until this number is known)

  // per iteration variables (potentially changed every interrupt)
  volatile unsigned int n;                 // index in acceleration curve, used to calculate next interval
  volatile float d;                        // current interval length
  volatile unsigned long di;               // above variable truncated
  volatile unsigned int stepCount;         // number of steps completed in current movement
};


//Define basic IO control
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

//Generate stepper info for 2 steppers
#define NUM_STEPPERS 2
volatile stepperInfo steppers[NUM_STEPPERS];

//Initialize pins and triggers
void ballaskSetup() {
  pinMode(A_STEP_PIN,   OUTPUT);
  pinMode(A_DIR_PIN,    OUTPUT);
  pinMode(A_ENABLE_PIN, OUTPUT);

  pinMode(B_STEP_PIN,   OUTPUT);
  pinMode(B_DIR_PIN,    OUTPUT);
  pinMode(B_ENABLE_PIN, OUTPUT);

  digitalWrite(A_ENABLE_PIN, LOW);
  digitalWrite(B_ENABLE_PIN, LOW);

  //black magic I half understand
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 1000;                             // compare value
  TCCR1B |= (1 << WGM12);                   // CTC mode
  TCCR1B |= ((1 << CS11) | (1 << CS10));    // 64 prescaler
  interrupts();

  //Stepper max/min accer/vel. maps function to stepper struct
  steppers[0].dirFunc = bDir;
  steppers[0].stepFunc = bStep;
  steppers[0].acceleration = 1000;
  steppers[0].minStepInterval = 50;

  steppers[1].dirFunc = aDir;
  steppers[1].stepFunc = aStep;
  steppers[1].acceleration = 4000;
  steppers[1].minStepInterval = 50;
  
}

//Note flag is a byte not a bit
volatile byte nextStepperFlag = 0;

//This function is called to determine when the next time interrupt needs to be called
void setNextInterruptInterval() {

  bool movementComplete = true;

  //Find the next stepper that needs to stepped and when
  unsigned int mind = 999999;
  for (int i = 0; i < NUM_STEPPERS; i++) {
    if ( ((1 << i) & remainingSteppersFlag) && steppers[i].di < mind ) {
      mind = steppers[i].di;
    }
  }

  //Reset the nextStepperFlag
  nextStepperFlag = 0;
  for (int i = 0; i < NUM_STEPPERS; i++) {
    if ( ! steppers[i].movementDone )
      movementComplete = false;

    if ( ((1 << i) & remainingSteppersFlag) && steppers[i].di == mind )
      nextStepperFlag |= (1 << i);
  }

  //There are no more steps that need to be taken. Set OCR1A to a big num
  if ( remainingSteppersFlag == 0 ) {
    OCR1A = 65500;
  }
  //I think this should be an else statement. Will test later
  OCR1A = mind;
}

//The interrupt function itself. Contains black magic
ISR(TIMER1_COMPA_vect)
{
  unsigned int tmpCtr = OCR1A;

  OCR1A = 65500;

  for (int i = 0; i < NUM_STEPPERS; i++) {

    if ( ! ((1 << i) & remainingSteppersFlag) )
      continue;

    if ( ! (nextStepperFlag & (1 << i)) ) {
      steppers[i].di -= tmpCtr;
      continue;
    }

    volatile stepperInfo& s = steppers[i];

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

  setNextInterruptInterval();

  TCNT1  = 0;
}

//Simple function to wait for a movement to be completed
//Does not need to be called. setNextInterruptInterval() is all
//that needs to be called. Things can happen while the motor is stepping
//and thats p cool
void runAndWait() {
  setNextInterruptInterval();
  while ( remainingSteppersFlag );
}

void heyLetsTestTheMotor(){
  prepareMovement( 0, 800 );
  prepareMovement( 1,  800 );
  runAndWait();
  prepareMovement( 0, 0 );
  prepareMovement( 1, 1600 );
}


