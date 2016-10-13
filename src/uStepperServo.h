/********************************************************************************************
*       File:       uStepperServo.h                                                         *
*       Version:    1.0.0                                                                   *
*       date:       October 13th, 2016                                                       *
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

/**
 * @brief      Class for ustepper servo.
 */
class uStepperServo
{
  private:
    uint8_t pin;        /** < Digital output pin connected to servo input terminal */
    uint8_t angle;      /** < Current angle in degrees */
    uint16_t pulse;     /** < Pulse width in timer0 ticks (1 tick = 16us) */
    uint8_t min16;      /** < Minimum pulse width in timer0 ticks (default = 92 = 1.472ms) */
    uint8_t max16;      /** < Maximum pulse width in timer0 ticks (default = 150 = 2.4ms) */

    class uStepperServo *next;      /** < Pointer to hold address of next servo in chain, if multiple
                                     * servos are connected */
    static uStepperServo* first;    /** < Pointer to hold address of first servo in chain */
  public:

    /**
     * @brief      Constructor for servo class
     *
     *             This constructor is used to instantiate a new servo class,
     *             and should be used for each servo the user wishes to connect
     *             to the uStepper.
     */
    uStepperServo();
    
    /**
     * @brief      Attaches the servo motor to a specific pin
     *
     *             This method is used to attach the instantiated servo motor
     *             object to a specific pin connected to the servo input
     *             terminal
     *
     * @param[in]  pinArg  Pin connected to servo input terminal
     *
     * @return     0 on failure
     */
    uint8_t attach(int pinArg);
    
    /**
     * @brief      Detaches the servo motor from the uStepper
     *
     *             This method detaches the servo motor from the uStepper
     */
    void detach();
    
    /**
     * @brief      Specify angle of servo motor
     *
     *             This method sets an angle setpoint for the servo motor
     *
     * @param[in]  angleArg  The angle argument
     */
    void write(int angleArg);
    
    /**
     * @brief      Sets the minimum pulse.
     *
     *             This method sets the minimum pulse length given to the servo
     *             motor. This pulse length defines the fully closed angle.
     *
     * @param[in]  t     Minimum pulse width in microseconds
     */
    void setMinimumPulse(uint16_t t);
    
    /**
     * @brief      Sets the maximum pulse.
     *
     *             This method sets the maximum pulse length given to the servo
     *             motor. This pulse length defines the fully open angle.
     *
     * @param[in]  t     Maximum pulse width in microseconds
     */
    void setMaximumPulse(uint16_t t);
    
    /**
     * @brief      Updates servo output pins
     *
     *             This method updates the pulses on the servo output pins. This
     *             method must be called at least once every 50 ms, to ensure
     *             correct movement of the Servo motors!
     */
    static void refresh();                        
};

#endif
