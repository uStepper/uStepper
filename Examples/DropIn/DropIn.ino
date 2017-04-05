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
*
*	When you connect your stepper motor and set the direction in the firmware of your controller, you should observe that the uStepper is running as expected! 
*	If the motor is connected incorrect or the direction is inverted from the controller, the algorithm will make uStepper run in the wrong direction. 
*	please make sure to check this.
*/

#include <uStepper.h>

uStepper stepper;

void setup(void)
{
	stepper.setup(DROPIN,SIXTEEN,10,5,100.0,2.0,0.6);
}

void loop(void)
{
}
