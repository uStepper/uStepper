/********************************************************************************************
*       File:       uStepperServo.h                                                         *
*       Version:    0.5.0                                                                   *
*       date:       October 9th, 2016                                                       *
*       Author:     Thomas Hørring Olsen                                                    *
*                                                                                           *   
*********************************************************************************************
*   (C) 2016                                                                                *
*                                                                                           *
*   ON Development IVS                                                                      *
*   www.on-development.com                                                                  *
*   administration@on-development.com                                                       *
*                                                                                           *
*   The code contained in this file is based on the Arduino Software Servo Library found at *
*   http://playground.arduino.cc/ComponentLib/Servo                                         *
*                                                                                           *
*   The code in this file is provided without warranty of any kind - use at own risk!       *
*   neither ON Development IVS nor the author, can be held responsible for any damage       *
*   caused by the use of the code contained in this file !                                  *
*                                                                                           *
********************************************************************************************/
/** @file uStepperServo.h
 * @brief      Function prototypes and definitions for the uStepper Servo
 *             library
 *
 *             This file contains class and function prototypes for the library,
 *             as well as necessary constants and global variables.
 *
 * @author     Thomas Hørring Olsen (thomas@ustepper.com)
 */

#ifndef _USTEPPERSERVO_H
#define _USTEPPERSERVO_H

#include <Arduino.h>
#include <inttypes.h>

class uStepperServo
{
  private:
    uint8_t pin;
    uint8_t angle;       // in degrees
    uint16_t pulse;     // pulse width in TCNT0 counts
    uint8_t min16;       // minimum pulse, 16uS units  (default is 34)
    uint8_t max16;       // maximum pulse, 16uS units, 0-4ms range (default is 150)
    class uStepperServo *next;
    static uStepperServo* first;
  public:
    uStepperServo();
    uint8_t attach(int);     // attach to a pin, sets pinMode, returns 0 on failure, won't
                             // position the servo until a subsequent write() happens
    void detach();
    void write(int);         // specify the angle in degrees, 0 to 180
    uint8_t read();
    uint8_t attached();
    void setMinimumPulse(uint16_t);  // pulse length for 0 degrees in microseconds, 540uS default
    void setMaximumPulse(uint16_t);  // pulse length for 180 degrees in microseconds, 2400uS default
    static void refresh();    // must be called at least every 50ms or so to keep servo alive
                              // you can call more often, it won't happen more than once every 20ms
};

#endif
