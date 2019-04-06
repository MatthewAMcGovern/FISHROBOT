

void Update(bool pid, set){
  potRprev = potR;
  potLprev = potL;
  potL = analogRead(potLpin);
  potR = analogRead(potRpin);
  filter();

  
  
}

void setupTailSys(){
  readPots();
  filter();
  
}

void filter(){
  prevT = T;
  T = (micros()-prevT)*1e-6;
  potLfiltprev = potLfilt;
  potLfilt = ((1.0-0.5*omega_c*T)/(1.0+0.5*omega_c*T))*potLfiltprev+(0.5*omega_c*T/(1.0+0.5*omega_c*T))*(potL+potLprev);
  potRfiltprev = potRfilt;
  potRfilt = ((1.0-0.5*omega_c*T)/(1.0+0.5*omega_c*T))*potRfiltprev+(0.5*omega_c*T/(1.0+0.5*omega_c*T))*(potR+potRprev);
}

void writeToTails(int LSpeed, int RSpeed){
  analogWrite(tailLpin, LSpeed);
  analogWrite(tailRpin, RSpeed);
}




