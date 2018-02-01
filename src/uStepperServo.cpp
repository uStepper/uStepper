/********************************************************************************************
*       File:       uStepper.cpp                                                            *
*       Version:    1.3.0                                                                   *
*       date:       January 10th, 2018                                                      *
*       Author:     Thomas Hørring Olsen                                                    *
*                                                                                           *   
*********************************************************************************************
*                       uStepperServo class                                                 *
*                                                                                           *
*   This file contains the implementation of the class methods, incorporated in the         *
*   uStepperServo arduino library. The library is used by instantiating an uStepperServo    *
*   object by calling either of the two overloaded constructors:                            *
*                                                                                           *
*       example:                                                                            *
*                                                                                           *
*       uStepperServo servo;                                                                *
*                                                                                           * 
*                                                                                           *
*   after instantiation of the object, the object attach function, should be called within  *
*   arduino's setup function:                                                               *
*                                                                                           *
*       example:                                                                            *
*                                                                                           *
*       uStepperServo servo;                                                                *
*                                                                                           *
*       void setup()                                                                        *
*       {                                                                                   *
*           servo.attach(10);                                                               *
*       }                                                                                   *
*                                                                                           *
*   This will attach a servo to pin 10, which is the argument of the attach function.       *
*                                                                                           *
*   The servo pulse widths are normally around 500 us for 0 deg and 2500 us for 180 deg.    *
*   The default values in this library are 1472 and 2400 us - giving a work area of         *
*   ~90-180deg. These values can be redefined to fit your servos specifications by calling  *
*    the setMaximumPulse and SetMinimumPulse functions. However, because of running the     *
*    stepper algorithm simultaniously with the servo, there is a risk of twitching if       *
*    using lower values than the 1500 us.                                                   *                           *
*                                                                                           *
*       example:                                                                            *
*                                                                                           *
*       uStepperServo servo;                                                                *
*                                                                                           *
*       void setup()                                                                        *
*       {                                                                                   *
*           servo.attach(10);                                                               *
*           servo.SetMaximumPulse(2400);                                                    *
*           servo.SetMinimumPulse(1500);//Should be kept above 1500!!                       *
*       }                                                                                   *
*                                                                                           *
*   To apply the pulses to the attached servos, the refresh function should be called       *
*   periodically at a rate of 20-50 Hz, i.e. every 50-20 ms. Calling the function more      *
*   often than every 20 ms is not a problem for the servo library.                          *
*                                                                                           *
*       example                                                                             *
*                                                                                           *
*       uStepperServo servo;                                                                *
*                                                                                           *
*       void setup()                                                                        *
*       {                                                                                   *  
*           servo.attach(10);                                                               *
*           servo.SetMaximumPulse(2400);                                                    *
*           servo.SetMinimumPulse(1500);  //Should be kept above 1500!!                     *
*       }                                                                                   *
*                                                                                           *
*        void loop()                                                                        *
*       {                                                                                   *
*           uStepperServo::refresh();                                                       *
*       }                                                                                   *
*   After this, the library is ready to control the Servo!                                  *
*                                                                                           *
*********************************************************************************************
*   (C) 2018                                                                                *
*                                                                                           *
*   uStepper ApS                                                                            *
*   www.ustepper.com                                                                        *
*   administration@ustepper.com                                                             *
*                                                                                           *
*   The code contained in this file is released under the following open source license:    *
*                                                                                           *
*           Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International         *
*                                                                                           *
*   The code in this file is provided without warranty of any kind - use at own risk!       *
*   neither uStepper ApS nor the author, can be held responsible for any damage             *
*   caused by the use of the code contained in this file !                                  *
*                                                                                           *
********************************************************************************************/
/** @file uStepperServo.cpp
 * @brief      Class implementations for the uStepper library
 *
 *             This file contains the implementations of the classes defined in
 *             uStepper.h
 *
 * @author     Thomas Hørring Olsen (thomas@ustepper.com)
 */

#include <uStepperServo.h>

uStepperServo *uStepperServo::first;



uStepperServo::uStepperServo() : pin(0),angle(NO_ANGLE),pulse(0),min16(92),max16(150),next(0)
{

}

void uStepperServo::setMinimumPulse(uint16_t t)
{
    min16 = t/16;
}

void uStepperServo::setMaximumPulse(uint16_t t)
{
    max16 = t/16;
}

uint8_t uStepperServo::attach(int pinArg)
{
    pin = pinArg;
    angle = NO_ANGLE;
    pulse = 0;
    next = first;
    first = this;
    digitalWrite(pin,0);
    pinMode(pin,OUTPUT);
    return 1;
}

void uStepperServo::detach()
{
    for ( uStepperServo **p = &first; *p != 0; p = &((*p)->next) ) {
	if ( *p == this) {
	    *p = this->next;
	    this->next = 0;
	    return;
	}
    }
}

void uStepperServo::write(int angleArg)
{
    if ( angleArg < 0) angleArg = 0;
    if ( angleArg > 180) angleArg = 180;
    angle = angleArg;
    // bleh, have to use longs to prevent overflow, could be tricky if always a 16MHz clock, but not true
    // That 64L on the end is the TCNT0 prescaler, it will need to change if the clock's prescaler changes,
    // but then there will likely be an overflow problem, so it will have to be handled by a human.
      pulse = (min16*16L*clockCyclesPerMicrosecond() + (max16-min16)*(16L*clockCyclesPerMicrosecond())*angle/180L)/64L;
}

void uStepperServo::refresh()
{
    uint8_t count = 0, i = 0;
    uint16_t base = 0;
    uStepperServo *p;
    static unsigned long lastRefresh = 0;
    unsigned long m = millis();

    // if we haven't wrapped millis, and 20ms have not passed, then don't do anything
    if ( m >= lastRefresh && m < lastRefresh + 20) return;
    lastRefresh = m;

    for ( p = first; p != 0; p = p->next ) if ( p->pulse) count++;
    if ( count == 0) return;

    // gather all the uStepperServos in an array
    uStepperServo *s[count];
    for ( p = first; p != 0; p = p->next ) if ( p->pulse) s[i++] = p;

    // bubblesort the uStepperServos by pulse time, ascending order
    for(;;) {
	uint8_t moved = 0;
	for ( i = 1; i < count; i++) {
	    if ( s[i]->pulse < s[i-1]->pulse) {
		uStepperServo *t = s[i];
		s[i] = s[i-1];
		s[i-1] = t;
		moved = 1;
	    }
	}
	if ( !moved) break;
    }

    // turn on all the pins
    // Note the timing error here... when you have many uStepperServos going, the
    // ones at the front will get a pulse that is a few microseconds too long.
    // Figure about 4uS/uStepperServo after them. This could be compensated, but I feel
    // it is within the margin of error of software uStepperServos that could catch
    // an extra interrupt handler at any time.
    TCCR1B &= ~(1 << CS10); 
    //cli();
    for ( i = 0; i < count; i++) digitalWrite( s[i]->pin, 1);

    uint8_t start = TCNT0;
    uint8_t now = start;
    uint8_t last = now;

    // Now wait for each pin's time in turn..
    for ( i = 0; i < count; i++) {
	uint16_t go = start + s[i]->pulse;// current time + pulse length for specific servo

	// loop until we reach or pass 'go' time
    
	for (;;) {
	    now = TCNT0;
	    if ( now < last) base += 256;//Timer 0 tops at 255, so add 256 on overflow
	    last = now;//update overflow check variable 
                  
        if(base + now >= go - 16)
        {
            cli();
        }

	    if ( base+now > go) {
		digitalWrite( s[i]->pin,0);
        sei();
		break;
	    }
	}
    }
    TCCR1B |= (1 << CS10);
}
