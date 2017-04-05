#include <uStepper.h>
#define MICROSTEP 16//microstep setting of uStepper
#define RES (200.0*MICROSTEP)/360.0//calculate step pr. degree
#define STEPPRMM 53.55//step pr. mm for the rail used in the demo
#define MMPRSTEP 1/STEPPRMM//mm pr. step
#define MMPRDEG MMPRSTEP*RES//mm pr. degree

uStepper stepper;

void setup() {
  // put your setup code here, to run once:
  stepper.setup();
  stepper.setCurrent(15.0);//use software current setting
  stepper.setMaxAcceleration(20000);
  stepper.setMaxVelocity(4000);
  Serial.begin(115200);
}

float limit()
{
  int checks=0;
  float pos=0.0;
  stepper.runContinous(CCW);
  while(checks < 3)//allows for 2 checks on movement error
  {
   pos = abs(stepper.encoder.getAngleMoved()-(stepper.getStepsSinceReset()*0.1125));//see current position error
   if(pos<5)//if position error is less than 5 steps it is okay...
    {
      checks=0;
    }
    else //if position error is 5 steps or more, count up checks
    {
      checks++;
    }
  }  
  checks=0;//reset checks for next endstop detection
  stepper.hardStop(SOFT);//stop motor without brake
  stepper.moveSteps(5*STEPPRMM, CW, SOFT);//back off with 5 mm
  while(stepper.getMotorState())//wait for motor to finish move
  {
    delay(1);//because Arduino does not accept the empty while loop for some reason....
  }
  stepper.encoder.setHome();//set new home position
  stepper.runContinous(CW);//do check in opposite direction
  while(checks < 3)//allows for 2 checks on movement error
  {
   pos = abs(stepper.encoder.getAngleMoved()-(stepper.getStepsSinceReset()*0.1125));//see current position error
   if(pos<5)//if position error is less than 5 steps it is okay...
    {
      checks=0;
    }
    else //if position error is 5 steps or more, count up checks
    {
      checks++;
    }
  }  
  stepper.hardStop(SOFT);//stop
  stepper.moveSteps(5*STEPPRMM, CCW, SOFT);//back off with 5 mm
  while(stepper.getMotorState())//wait for motor to finish move
  {
    delay(1);//because Arduino does not accept the empty while loop for some reason....
  }
  pos = stepper.encoder.getAngleMoved();//read current position
  stepper.moveSteps((pos*RES)/2, CCW, SOFT);//go to center
  while(stepper.getMotorState())//wait for motor to finish move
  {
    delay(1);//because Arduino does not accept the empty while loop for some reason....
  }
  return pos;//return max position
}

void loop() {
  Serial.println(limit()*MMPRDEG);//find end positions and read out the recorded end position
  stepper.setup(PID,SIXTEEN,10,5,2.0,2.0,0.6);//enable PID
  stepper.setCurrent(100.0);//use software current setting
  while(1);
}