/*
*			DropIn example sketch!
*
*	Pin connections:
*	-------------------------
*	| Controller | uStepper |
*	|-----------------------|
*	|	Enable   |	D2 		|
*	|	Step     |	D3 		|
*	|	Dir      |	D4 		|
*	|	GND      |	GND		|
*	-------------------------	
*/

#include <uStepper.h>

uStepper stepper;

void setup(void)
{
	stepper.setup(DROPIN,SIXTEEN,3000.0,10);			//Activate dropin, 1/16th microstepping, speed during fault: 3000 steps/s, # of missed steps allowed
	Serial.begin(115200);
}

void loop(void)
{
	Serial.print(stepper.encoder.getSpeed());
	Serial.print("   ");
	Serial.println(stepper.encoder.getAngleMoved());
	delay(10);
}