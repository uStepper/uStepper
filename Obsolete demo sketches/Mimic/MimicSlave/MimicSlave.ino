#define MINTIME 1000//min time in micros
#define MAXTIME 2023//max time in micros
#define MICROSTEP 16//microstepping
#define STEPSPERREV 200//steps per revolution
#define STEP360DEG (MICROSTEP*STEPSPERREV)//steps for 360 degrees
#define RCCONSTANT STEP360DEG/(MAXTIME-MINTIME)//

#define R2 4700.0 //4.7kohm resistance in series with NTC

/****************************
*  Steinhart-Hart coefficients
*  Calculated using matlab script
******************************/
#define A 0.001295752996237  //T1 = 5 degree celcius
#define B 0.000237488365866  //T2 = 50 degree Celcius
#define C 0.000000083423218  //T3 = 105 degree Celcius

int RCpin = 2;
unsigned long time = 0;
int steps = 0;
int i = 0;
int temp = 0;

#include <AccelStepper.h>
#include "SPI.h"
AccelStepper stepper(1,STEP,DIR);
/*
unsigned int getPosition()
{
  unsigned int result1, result2, result;
  
  digitalWrite(SS, LOW);
  result1 = SPI.transfer(0xff);
  result2 = SPI.transfer(0xff);  
  digitalWrite(SS, HIGH);
  result1 &= 0b00111111;
  result1 = result1 << 8;
  result = (result1 | result2) >> 1;
  result &= 0xFFF;
  return result;
}
*/

int tempCalc()
{
  float T = 0.0;
  float Vout = 0;
  float NTC = 0;
  Vout = analogRead(A5)*(5.0/1023.0);//0.0048876=5V/1023
  NTC = ((R2*5.0)/Vout)-R2;//the current NTC resistance
  NTC = log(NTC);
  T = A + B*NTC + C*NTC*NTC*NTC;    //Steinhart-Hart equation
  T = 1/T;
  return (int)(T-273.15);
}

void setup()
{
  unsigned int curPos = 0;
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV8); //you can chose faster SPI frequency 
  stepper.setEnablePin(ENA);//enablepin sidder på udgang 6
  stepper.setMaxSpeed(20000);
  stepper.setSpeed(20000);
  stepper.setAcceleration(80000);
  stepper.disableOutputs();
  Serial.begin(115200);

  pinMode(RCpin, INPUT);
  attachInterrupt(1, RC, CHANGE);
}

void loop()
{
  for(i = 0; i<32000; i++)
  {
    stepper.moveTo(steps);
    stepper.run();
  }
  temp = tempCalc();
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println("C");
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
  if (abs(oldtime - time) >= 10)
  {
    steps = (float)((time-MINTIME)*RCCONSTANT);
    if(steps < 0)
    {
      steps = 0;
    }
    oldtime = time;
  }
}
