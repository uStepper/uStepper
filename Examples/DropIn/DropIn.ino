/*
*			DropIn example sketch!
*
*		IMPORTANT! READ THIS !!!
*
*	In order for the drop in feature to work, the user
*	needs to enable the feature. This is done be uncommenting
*	the "DROPIN" define in the "conf.h" file of the library.
*	This file can be found in the src folder of the uStepper
*	library installation.
*/

#include <uStepper.h>

uStepper stepper;

void setup(void)
{
	stepper.setup();
	Serial.begin(115200);
}

void loop(void)
{
	Serial.print(stepper.encoder.getSpeed());
	Serial.print("   ");
	Serial.println(stepper.encoder.getAngleMoved());
	delay(10);
}