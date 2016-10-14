#include <uStepper.h>

uStepperServo servo;
uint16_t value;

void setup()
{
  servo.attach(10);
  servo.setMaximumPulse(2400);
  servo.setMinimumPulse(1500);
  Serial.begin(9600);
}

void loop()
{
  if(Serial.available())
  {
    value = Serial.parseInt();
  }

  servo.write(value);
  uStepperServo::refresh();
}
