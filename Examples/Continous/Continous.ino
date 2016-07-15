#include <uStepper.h>

#define MAXACCELERATION 1500         //Max acceleration = 1500 Steps/s^2
#define MAXVELOCITY 1100           //Max velocity = 1100 steps/s

uStepper stepper(MAXACCELERATION, MAXVELOCITY);

void setup() {
  // put your setup code here, to run once:
  stepper.setup();
  Serial.begin(115200);
  stepper.runContinous(CCW);
}

void loop() {
  char cmd;
  // put your main code here, to run repeatedly:
  while(!Serial.available());
  Serial.println("ACK!");
  cmd = Serial.read();
  if(cmd == '1')                      //Run continous clockwise
  {
    stepper.runContinous(CW);
  }
  
  else if(cmd == '2')                 //Run continous counter clockwise
  {
    stepper.runContinous(CCW);
  }
  
  else if(cmd == '3')                 //Stop without deceleration and block motor
  {
    stepper.hardStop(HARD);
  }
  
  else if(cmd == '4')                 //Stop without deceleration and don't block motor
  {
    stepper.hardStop(SOFT);
  }
  
  else if(cmd == '5')                 //Stop with deceleration and block motor
  {
    stepper.softStop(HARD);
  }
  
  else if(cmd == '6')                 //Stop with deceleration and don't block motor
  {
    stepper.softStop(SOFT);
  }
}
