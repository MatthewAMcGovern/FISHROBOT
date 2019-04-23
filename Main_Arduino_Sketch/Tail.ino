
void UpdateTailReading(){
  TailDataFrame loc;
  pTIME = TIME;
  TIME = micros()*1e-6;
  T = TIME - pTIME;
  loc.potL = analogRead(potLpin);
  loc.potR = analogRead(potRpin);
  
  loc.T = T;
  loc.ReadTime = TIME;
  loc.Lvel = (loc.potL - hist[0].potL)/loc.T;
  loc.Rvel = (loc.potR - hist[0].potR)/loc.T;
  
  HistBuff(loc);
  //filter();
  MovingAvgFilter();
  
}

void HistBuff(TailDataFrame tdf){
  for(int i = 0; i < s-1; i++){
    hist[i+1] = hist[i];
  }
  hist[0] = tdf;
}

void FiltBuff(TailDataFrame tdf){
  for(int i = 0; i < s-1; i++){
    Filt[i+1] = Filt[i];
  }
  Filt[0] = tdf;
}

void setupTailSys(){
  pinMode(potLpin, INPUT);
  pinMode(tailLpin, OUTPUT);
  pinMode(potRpin, INPUT);
  pinMode(tailRpin, OUTPUT);
  TIME = micros();
  TailDataFrame loc;
  loc.potL = analogRead(potLpin);
  loc.potR = analogRead(potRpin);
  loc.T = 0;
  loc.ReadTime = TIME;
  for(int i = 0; i < s; i++){
    hist[i] = loc;
  }
  UpdateTailReading();
  
}

//needs conversion to use buffer
//also needs setup support code
void LowPassFilter(){
//  potLfiltprev = potLfilt;
//  potLfilt = ((1.0-0.5*omega_c*T)/(1.0+0.5*omega_c*T))*potLfiltprev+(0.5*omega_c*T/(1.0+0.5*omega_c*T))*(potL+potLprev);
//  potRfiltprev = potRfilt;
//  potRfilt = ((1.0-0.5*omega_c*T)/(1.0+0.5*omega_c*T))*potRfiltprev+(0.5*omega_c*T/(1.0+0.5*omega_c*T))*(potR+potRprev);
}

//assumes hist is up to date on current cycle
void MovingAvgFilter(){
  TailDataFrame loc;
  double RAvg = 0;
  double LAvg = 0;
  double avgSum = 0;
  
  for(int i = 0; i < s; i++){
    RAvg += hist[i].potR * 1.0/((double)i+1);
    LAvg += hist[i].potL * 1.0/((double)i+1);
    avgSum += 1.0/((double)i+1);
  }
  loc.potR = RAvg/avgSum;
  loc.potL = LAvg/avgSum;
  loc.T = hist[0].T;
  loc.Lvel = (loc.potL - Filt[0].potL)/loc.T;
  loc.Rvel = (loc.potR - Filt[0].potR)/loc.T;
  loc.ReadTime = hist[0].ReadTime;
  FiltBuff(loc);
  
}

void writeToTails(int LSpeed, int RSpeed){
  if(LSpeed >= 0){
    if(LSpeed > 130){
      LSpeed = 130;
      
    }
    else if(LSpeed < 80){
      LSpeed = 80;
    }
    Serial.println(LSpeed);
    analogWrite(tailLpin, LSpeed);
  }

  if(RSpeed >= 0){
    if(RSpeed > 130){
      RSpeed = 130;
    }
    else if(RSpeed < 80){
    RSpeed = 80;
    }

    
    analogWrite(tailRpin, RSpeed);
  }
}


///////////////////////////////
//PID Loop Logic

// setpoint and error calculated externally, only error is needed
double pid(double e, double kp, double ki, double kd, TailDataFrame* data){
  //double out = kp*e + kd*(e - eprev)/T
}

//done every cycle once PID is engaged and passed to pid function
void updateErrorBuff(double Lset, double Rset, TailDataFrame* data){
  PIDDataFrame loc;
  

}

//done every cycle before error frame is calculated
void updateSetpointBuff(){
  SetFrame loc;
  
  
}

//used to correlate frequency and pwm output
double BaseOutput(double freq){
  // need regression for baseline guess on certain frequency
  
}

void moveToStartL(TailDataFrame* data){
  if(!LEngaged){
    writeToTails(105,-1);
    if(!(data[0].Lvel > 0 ^ LStartVel)){
      if(LStartVel){
        if(data[0].potL >= Lmid){
          LEngaged = true;
          writeToTails(0,-1);
        }
      }
      else{
        if(data[0].potL <= Lmid){
          LEngaged = true;
          writeToTails(0, -1);
        }
      }
    }
  }
  else{
    writeToTails(0,-1);
  }
}

void moveToStartR(TailDataFrame* data){
  if(!REngaged){
    writeToTails(-1, 105);
    if(!(data[0].Rvel > 0 ^ RStartVel)){
      if(LStartVel){
        if(data[0].potR >= Rmid){
          REngaged = true;
          writeToTails(-1, 0);
        }
      }
      else{
        if(data[0].potR <= Rmid){
          REngaged = true;
          writeToTails(-1, 0);
        }
      }
    }
    
  }
  else{
    writeToTails(-1, 0);
  }
}

