#define MINTIME 1000//min time in micros
#define MAXTIME 2025//max time in micros
#define MICROSTEP 16//microstepping
#define STEPSPERREV 200//steps per revolution
#define STEP120DEG (((MICROSTEP*STEPSPERREV)/2)/180)*120//steps for 180 degrees
#define RCCONSTANT STEP120DEG/(MAXTIME-MINTIME)//

#include "SPI.h"
#include <AccelStepper.h>
AccelStepper stepper(1,STEP,DIR);

int RCpin = 14;
unsigned long time = 0;
int steps = 0;
int i = 0;

volatile unsigned int result = 0;
volatile unsigned int result1 = 0;
volatile unsigned int result2 = 0;

float getPos()
{
  float degree = 0.0;
  digitalWrite(SS, LOW);
  result1 = SPI.transfer(0xff);
  result2 = SPI.transfer(0xff);  
  digitalWrite(SS, HIGH);
  result1 &= 0b00111111;
  result1 = result1 << 8;
  result = (result1 | result2) >> 1;
  result &= 0xFFF;
  degree = ((float)result)/4096.0;
  degree *= 360.0;
  return degree;
}

void setup()
{
  stepper.setEnablePin(ENA);//enablepin sidder på udgang 6
  stepper.setMaxSpeed(20000);
  stepper.setSpeed(20000);
  stepper.setAcceleration(80000);
  stepper.disableOutputs();
  Serial.begin(115200);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV8); //you can chose faster SPI frequency 
  pinMode(RCpin, INPUT);
  attachInterrupt(1, RC, CHANGE);
  digitalWrite(DIR,HIGH);
}

void loop()
{
  for(i = 0; i < 1000; i++)
  {
    stepper.moveTo(steps);
    stepper.run();
  }
  /*Serial.print("rotation: ");
  Serial.print(getPos());
  Serial.println("Deg");*/
  //delay(500);
}

void RC()
{
  static unsigned long starttime = 0;
  static unsigned long stoptime = 0;
  static unsigned long oldtime = 0;
   
  if (digitalRead(RCpin) == HIGH)
  {
    starttime = micros();
  }
  else
  {
    stoptime = micros();
    time = (stoptime - starttime);
  }  
  if (abs(oldtime - time) >=10)
  {
    steps = (float)((time-MINTIME)*RCCONSTANT);
    if(steps < 0)
    {
      steps = 0;
    }
    oldtime = time;
  }
}
