#include <uStepper.h>

uStepperServo servo;    //instantiate uStepperServo object
uint16_t value;

void setup()
{
  servo.attach(10);     //Attach Servo to digital pin 10
  Serial.begin(9600);
}

void loop()
{
  if(Serial.available())
  {
    value = Serial.parseInt();      //Read angle argument from serial
    servo.write(value);             //Write angle to servo object
  }

  uStepperServo::refresh();         //Call this method at least once every 50ms to keep the servo running
}
