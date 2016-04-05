/* 
 Controlling a servo position using a potentiometer (variable resistor) 
 by Michal Rinott <http://people.interaction-ivrea.it/m.rinott> 

 modified on 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Knob
*/

#include <Servo.h>
#include "SPI.h"

Servo myservo;  // create servo object to control a servo

volatile unsigned int result = 0;
volatile unsigned int result1 = 0;
volatile unsigned int result2 = 0;

int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin


void setup()
{
  myservo.attach(8,1000,2023);  // attaches the servo on pin 9 to the servo object
  Serial.begin(115200);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV8); //you can chose faster SPI frequency 
}

void loop() 
{  
  val = map(result, 0, 4095, 0, 180);  // scale it to use it with the servo (value between 0 and 180) 
  myservo.write(val);                  // sets the servo position according to the scaled value 
  Serial.println(result);
  delay(15);                           // waits for the servo to get there 
  digitalWrite(SS, LOW);
  result1 = SPI.transfer(0xff);
  result2 = SPI.transfer(0xff);  
  digitalWrite(SS, HIGH);
  result1 &= 0b00111111;
  result1 = result1 << 8;
  result = (result1 | result2) >> 1;
  result &= 0xFFF;
} 

/*
void loop() 
{  
  val = map(result, 0, 4095, 0, 180);  // scale it to use it with the servo (value between 0 and 180) 
  myservo.write(val);                  // sets the servo position according to the scaled value 
  Serial.println(result);
  delay(15);                           // waits for the servo to get there 
  digitalWrite(SS, LOW);
  result1 = SPI.transfer(0xFF);
  result2 = SPI.transfer(0xFF);  
  digitalWrite(SS, HIGH);
  result1 = result1 << 8;
  result = (result1 | result2) >> 2;
  result &= 0x0FFF;
}*/
