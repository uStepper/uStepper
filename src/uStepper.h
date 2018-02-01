/********************************************************************************************
* 	 	File: 		uStepper.h 																*
*		Version:    1.3.0                                           						*
*      	date: 		January 10th, 2018 	                                    				*
*      	Author: 	Thomas Hørring Olsen                                   					*
*                                                   										*	
*********************************************************************************************
*	(C) 2018																				*
*																							*
*	uStepper ApS																			*
*	www.ustepper.com 																		*
*	administration@ustepper.com 															*
*																							*
*	The code contained in this file is released under the following open source license:	*
*																							*
*			Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International			*
* 																							*
* 	The code in this file is provided without warranty of any kind - use at own risk!		*
* 	neither uStepper ApS nor the author, can be held responsible for any damage				*
* 	caused by the use of the code contained in this file ! 									*
*                                                                                           *
********************************************************************************************/
/**
*	\mainpage Arduino library for the uStepper Board
*	
*	This is the uStepper Arduino library, providing software functions for the different features of the uStepper board.
*	
*	\par News!
*	This version of the library adds a new functions to the library:
*	MoveToEnd()     -   This function makes the motor turn in the specified direction, until an obstacle is detected,
*                       at which point the motor stops and resets its home position
*	Changed the "LimitDetection" example to make use of the new moveToEnd() function
*
*   Also some adjustments have been made to different functions. See changelog
*
*	\par Features
*	The uStepper library contains the following features:
*	
*	- Second order acceleration profile, to provide smooth ramping of the speed and to avoid the motor stalling when demanding high speeds.
*	- Closed loop PID position controller
*	- Control of RC servo motors
*	- Temperature measurement of the stepper driver chip by means of a NTC resistor placed beneath the chip.
*	- Measure the current position of the shaft (absolute, multiple revolutions)
*	- Measure the current speed of the motor 
*	
*	The library uses both timer one and timer two in order to function properly, meaning that unless the user of this library
*	can accept the loss of some functionality, these two timers are unavailable and the registers associated with these timers
*	should not be reconfigured.
*
*	Timer one is used for sampling the encoder in order to provide the ability to keep track of both the current speed and the
*	angle moved since the board was reset (or a new home position was configured). Also the drop-in features missed step detection and 
*	correction is done in this timer. 
*	
*	Timer two is used to calculate the stepper acceleration algorithm.  
*	\warning In order to get some features working, it was necessary to write functions to control the I2C hardware in the MCU, since 
*	the build in wire library of Arduino uses interrupt which doesn't play well when used inside a timer interrupt routine. Therefore if
*	the user of this library needs to use the I2C bus, the user should use the functions defined in this library instead of wire library !
* 
*	\par Installation
*	To install the uStepper library into the Arduino IDE, perform the following steps:
*
*	- Go to Sketch->Include Libraries->Manage Libraries... in the arduino IDE
*	- Search for "uStepper", in the top right corner of the "Library Manager" window
*	- Install uStepper library 
*	
*	The library is tested with Arduino IDE 1.8.5
*	
*	\warning MAC users should be aware, that OSX does NOT include FTDI VCP drivers, needed to upload sketches to the uStepper, by default. This driver should be 
*	downloaded and installed from FTDI's website:
*	\warning http://www.ftdichip.com/Drivers/VCP.htm
*	\warning             The uStepper should NOT be connected to the USB port while installing this driver !
*	\warning This is not a problem for windows/linux users, as these drivers come with the arduino installation.
*
*	\par Theory
*
*	The acceleration profile implemented in this library is a second order profile, meaning that the stepper will accelerate with a constant acceleration, the velocity will follow
*	a linear function and the position will look like an S-curve (it will follow a linear function at top speed and a second order function during acceleration and deceleration).
*	As a result, the position of the stepper motor will have continous and differentiable first and second derivatives.
*
*	The second order acceleration profile can be illustrated as follows (<a rel="license" href="http://picprog.strongedge.net/step_prof/step-profile.html">Source</a>):
*
*	\image html dva.gif
*
*	From the above picture it can be seen how the motor moves with this second order acceleration profile. The postion is depicted as orange, velocity as red and the acceleration as green.	
*
*	The algorithm used is NOT designed by me, I ONLY implemented the algorithm! credit is therefore given to the appropriate author: Aryeh Eiderman.	
*
*	for those interested, the deriviation of the algorithm used can be found at the following webpage (Which is neither owned nor affiliated with ON Development IVS in any
*	way): <a rel="license" href="http://hwml.com/LeibRamp.pdf">Real Time Stepper Motor Linear Ramping Just by Addition and Multiplication</a>	
*
*	\par Copyright
*
*	(C)2016 ON Development IVS	
*																	
*	www.on-development.com 																	
*
*	administration@on-development.com 														
*																							
*	<img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" />																
*
*	The code contained in this file is released under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>	
*																							
*	The code in this library is provided without warranty of any kind - use at own risk!		
* 	neither ON Development IVS nor the author, can be held responsible for any damage		
* 	caused by the use of the code contained in this library ! 	
*
*	\par To do list
*	- Clean out in unused variables
*	- Update comments
*	- Add support for limit switches
*	- Split the library into multiple files
*
*	\par Known Bugs
*	- No known bugs
*
*	\author Thomas Hørring Olsen (thomas@ustepper.com)
*	\par Change Log
* 	\version 1.3.0:
*  	- Changed MoveToAngle() function, to support update of angle setpoint in PID mode
*  	- Changed getMotorState() function, to support update of angle setpoint in PID mode
*	- Fixed bug in moveAngle() function, where negative inputs had no effect
*	- Changed the "LimitDetection" example to make use of the new moveToEnd() function
* 	\version 1.2.3:
*  	- Added moveAngle() and MoveToAngle() functions
*  	- Minor adjustments in setup routines
* 	\version 1.2.2:
*  	- Adjusted parameters in limitDetection example, and dropin example
*  	- Added setCurrent() function to the uStepper object, for the user to easily change current setting
*   - Minor corrections in PID algorithms
* 	\version 1.2.1:
*  	- Fixed a bug in the implementation of the standalone PID controller, primarily anti windup
*  	- Added example to show how limit detection can be implemented without the use of limit switches
*   - Adjusted encoder.setHome() function to also reset the stepsSinceReset variable
* 	\version 1.2.0:
*  	- Adjusted the code related to the PID closed loop position control. Now it actually works as intended!
*  	- Fixed Bug with encoder.getStrength and encoder.getAgc functions not reading the correct registers in the encoder
*   - Changed getStepsSinceReset() to return a 32 bit signed integer, instead of a 64 bit signed integer, which is just overkill, and doesn't really play well with arduino
*   - Changed some minor things in the assembly code to optimise stuff with regards to the PID mode	
*   - Fixed a bug in the assembly code causing the StepsSinceReset variable to always decrease regardless of the direction of rotation. This resulted in the number of steps moved since reset, always being negative
*   - Fixed a typo in the documentation of the getCurrentDirection() function, to state the correct return values
*   - Added several keywords to the library
*   - Added Example to demonstrate the use of PID closed loop position control 
*	\version 1.1.0:
*	- Fixed bug with encoder.setHome() function, where number of revolutions was never reset, resulting in the angle being reset to the number of revolutions times 360 degrees, instead of 0 degrees
*	- Implemented Timeout in I2C functions. This ensures that the program will not lock up if access to a non-existing I2C address is attempted.  
*	- Added overloaded function to pwmD8() and pwmD3(), in order to be able to revert the function of these to pins, back to normal IO mode
*	\version 1.0.0:
*	- Added PID functionality to drop-in Feature
*	- Added PID functionality to regular movement functions
*	- Added support for servo motors
*	- Added two new examples
*	\version 0.4.5:
*	- Changed setup of Timer1 and Timer2, to allow for PWM generation on D3 and D8. See documentation of "pwmD3()" and "pwmD8()" functions in class "uStepper", for instructions.
*	- Changed setup of external interrupts for dropin feature, to be able to compile with arduino IDE 1.6.10.
*	- Updated board installation package to version 1.1.0, to be compliant with arduino IDE 1.6.10. Update uStepper board using package manager in Arduino IDE ("Tools->Board->Board Manager", scroll down to the uStepper board and choose update or install).
*	\version 0.4.4:
*	- Added the attribute "used" to declarations of interrupt routines. This enables the library to be compiled in Arduino IDE 1.6.10 
*	- Updated documentation of "uStepper::setup()"
*	\version 0.4.3:
*	- Fixed bug where initial deceleration phase (used when changing speed setting or changing direction while motor is moving), would never be entered, causing motor to accelerate
*	\version 0.4.2:
*	- Fixed bug with setHome() causing getAngleMoved() to return alternating values (offset by exactly 180 degrees) between resets.
*	- Fixed bug in runContinous(), where direction of the motor did not get updated if the function was called with the motor standing still.
*	\version 0.4.1:
*	- Fixed bug with getAngleMoved() returning alternating values (offset by exactly 180 degrees) between resets.
*	- Added keywords
*	- Updated pin connection description for Drop-in example 
*	\version 0.4.0:
*	- Added Drop-in feature to replace stepsticks with uStepper for error correction
*	- Fixed bug in stepper acceleration algorithm, making the motor spin extremely slow at certain accelerations. Also this fix reduced the motor resonance
*	- Implemented an IIR filter on the speed measurement, to smooth this out a bit.
*	\version 0.3.0:
*	- Added support for speed readout
*	- Added support for measuring the shaft position with respect to a zero reference. (absolute within multiple revolutions)
*	\version 0.2.0:
*	- Complete rewrite of the stepper algorithm in assembler
*	- Changed from fixed point to floating point variables, due to the need for more precision
*	- Removed the getSpeed() method, as it didn't work, and therefore it would make more sense to remove it
*	  and re-add it when i get the time to fix it
*	- Added a few doxygen comments
*	- Added a new method (getStepsSinceReset()), which returns all steps performed since reset of the uStepper.
*	  positive values corresponds to steps in clockwise direction, while negative values corresponds to steps
*	  in counterclockwise direction.	
*	\version 0.1.0:	
*	- Initial release
*	
*/

/**
 * @file uStepper.h
 * @brief      Function prototypes and definitions for the uStepper library
 *
 *             This file contains class and function prototypes for the library,
 *             as well as necessary constants and global variables.
 *
 * @author     Thomas Hørring Olsen (thomas@ustepper.com)
 */

#ifndef _USTEPPER_H_
#define _USTEPPER_H_

#ifndef __AVR_ATmega328P__
#error !!This library only supports the ATmega328p MCU!!
#endif

#include <inttypes.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <Arduino.h>
#include <uStepperServo.h>

/** Full step definition*/
#define FULL 1							
/** Half step definition*/
#define HALF 2							
/** Quarter step definition*/
#define QUARTER 4						
/** Eighth step definition*/
#define EIGHT 8							
/** Sixteenth step definition*/	
#define SIXTEEN 16						
/** Value defining normal mode*/	
#define NORMAL 	0						
/** Value defining PWM mode of corresponding IO pin. Used to switch pin D3 and D8 between pwm or normal IO mode*/
#define PWM 	1						
/** Value defining dropin mode for 3d printer/CNC controller boards*/
#define DROPIN 	1						
/** Value defining PID mode for normal library functions*/
#define PID 	2						

/** Value to put in state variable in order to indicate that the motor should not be running */
#define STOP 1							
/** Value to put in state variable in order to indicate that the motor should be accelerating */
#define ACCEL 2	
/** Value to put in state variable in order to indicate that the motor should be decelerating */						
#define CRUISE 4						
/** Value to put in state variable in order to indicate that the motor should be cruising at constant speed with no acceleration */
#define DECEL 8							
/** Value to put in state variable in order to indicate that the motor should be decelerating to full stop before changing direction */
#define INITDECEL 16					
/** Frequency of interrupt routine, in which the delays for the stepper algorithm are calculated */ 
#define INTFREQ 28200.0f       			
/** Period of interrupt routine for stepper algortihm*/
#define INTPERIOD 1000000.0/INTFREQ		
/** constant to calculate the amount of interrupts TIMER2 has to wait with generating new pulse, during PID error correction*/
#define INTPIDDELAYCONSTANT 0.028199994 
/** Value to put in direction variable in order for the stepper to turn clockwise */
#define CW 0						
/** Value to put in direction variable in order for the stepper to turn counterclockwise */	
#define CCW 1						
/** Value to put in hold variable in order for the motor to block when it is not running */
#define HARD 1							
/** Value to put in hold variable in order for the motor to \b not block when it is not running */
#define SOFT 0							

/** Frequency at which the encoder is sampled, for keeping track of angle moved and current speed */
#define ENCODERINTFREQ 1000.0			
/** Constant to convert angle difference between two interrupts to speed in revolutions per second */
#define ENCODERSPEEDCONSTANT ENCODERINTFREQ/10.0/4096.0	
/** I2C address of the encoder chip */
#define ENCODERADDR 0x36				
/** Address of the register, in the encoder chip, containing the 8 least significant bits of the stepper shaft angle */
#define ANGLE 0x0E						
/** Address of the register, in the encoder chip, containing information about whether a magnet has been detected or not */
#define STATUS 0x0B						
/** Address of the register, in the encoder chip, containing information about the current gain value used in the encoder chip. This value should preferably be around 127 (Ideal case!) */
#define AGC 0x1A						
/** Address of the register, in the encoder chip, containing the 8 least significant bits of magnetic field strength measured by the encoder chip */
#define MAGNITUDE 0x1B					

/** The NTC resistor used for measuring temperature, is placed in series with a 4.7 kohm resistor. This is used to calculate the temperature */
#define R 4700.0 						

/** I2C bus is not currently in use */
#define I2CFREE 0						

/** Value for RW bit in address field, to request a read */
#define READ  1							

/** Value for RW bit in address field, to request a write */
#define WRITE 0							

/** start condition transmitted */
#define START  0x08						

/** repeated start condition transmitted */
#define REPSTART 0x10					

/** slave address plus write bit transmitted, ACK received */
#define TXADDRACK  0x18					

/** data transmitted, ACK received */
#define TXDATAACK 0x28					

/** slave address plus read bit transmitted, ACK received */
#define RXADDRACK 0x40				

/** value to indicate ACK for i2c transmission */
#define ACK 1							

/** value to indicate NACK for i2c transmission */
#define NACK 0							
		
/**
 * Coefficients needed by the Steinhart-hart equation in order to find the
 * temperature of the NTC (and hereby the temperature of the motor driver) from
 * the current resistance of the NTC resistor. The coefficients are calculated
 * for the following 3 operating points:
 *
 * A: T = 5 degree Celsius
 *
 * B: T = 50 degree Celsius
 *
 * C: T = 105 degree Celsius
 *
 * The Steinhart-Hart equation is described at the following link:
 *
 * https://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation#Developers_of_the_equation
 */

#define A 0.001295752996237  	
/** See description of A */
#define B 0.000237488365866  			
/** See description of A */
#define C 0.000000083423218  			

/**
 * @brief      Used by dropin feature to take in step pulses
 *
 *             This interrupt routine is used by the dropin feature to keep
 *             track of step and direction pulses from main controller
 */
extern "C" void interrupt0(void);

/**
 * @brief      Used by dropin feature to take in enable signal
 *
 *             This interrupt routine is used by the dropin feature to keep
 *             track of enable signal from main controller
 */
extern "C" void interrupt1(void);

/**
 * @brief      Used to apply step pulses to the motor
 *
 *
 *             This interrupt routine is in charge of applying step pulses to
 *             the motor. The routine runs at a frequency of 28.2kHz, and
 *             handles acceleration algorithm calculations, as well as applying
 *             step pulses during compensation for missed steps, while either
 *             dropin or PID feature are enabled.
 */
extern "C" void TIMER2_COMPA_vect(void) __attribute__ ((signal,naked,used));

/**
 * @brief      Measures angle and speed of motor.
 *
 *             This interrupt routine is in charge of sampling the encoder and
 *             measure the current speed of the motor. In case of Dropin or PID
 *             feature this routine runs at a frequency of 500Hz while during
 *             normal operation it runs at a frequency of 1kHz.
 */
extern "C" void TIMER1_COMPA_vect(void) __attribute__ ((signal,used));

class float2
{
	public:
		float2(void);
		float getFloatValue(void);
		uint64_t getRawValue(void);
		void setValue(float val);
		float2 & operator=(const float &value);
		bool operator==(const float2 &value);
		bool operator!=(const float2 &value);
		bool operator>=(const float2 &value);
		bool operator<=(const float2 &value);
		bool operator<=(const float &value);
		bool operator<(const float2 &value);
		bool operator>(const float2 &value);
		float2 & operator*=(const float2 &value);
		float2 & operator-=(const float2 &value);
		float2 & operator+=(const float2 &value);
		float2 & operator+=(const float &value);
		float2 & operator/=(const float2 &value);
		const float2 operator+(const float2 &value);
		const float2 operator-(const float2 &value);
		const float2 operator*(const float2 &value);
		const float2 operator/(const float2 &value);
		uint64_t value;

	private:
		friend void TIMER2_COMPA_vect(void) __attribute__ ((signal,used));
		
};

/**
 * @brief      Prototype of class for the temperature sensor
 *
 *             This class enables the user of the library to access the
 *             temperature sensor on the uStepper board. This class can be
 *             instantiated as a standalone object if this is the only feature
 *             on the uStepper required by the programmers specific application.
 */

class uStepperTemp
{
public:
	
	/**
	 * @brief      Constructor
	 *
	 *             This is the constructor of the temperature sensor class, and
	 *             should be used in order to instantiate a temperature sensor
	 *             object as follows:
	 *
	 * @code{.cpp}
	 *   uStepperTemp temp;
	 * @endcode
	 */

	uStepperTemp(void);
	
	/**
	 * @brief      Request a reading of current temperature
	 *
	 *             This method performs a reading of the current NTC temperature
	 *             and returns this value. Since the NTC resistor is physically
	 *             placed close to the stepper driver chip on the uStepper
	 *             board, the temperature returned by this method will be close
	 *             to the actual temperature of the driver chip.
	 *
	 * @return     Current NTC Temperature in degree Celsius
	 */

	float getTemp(void);
private:
};

/**
 * @brief      Prototype of class for the AS5600 encoder
 *
 *             This class enables the user of the library to access the AS5600
 *             encoder on the uStepper board. This class can be instantiated as
 *             a standalone object if all the features of the uStepper is not
 *             needed by the programmers specific application.
 */

class uStepperEncoder
{
public:
	/** Variable used to store that measured angle moved from the
	* reference position */
	volatile int32_t angleMoved;		
	
	/** Angle of the shaft at the reference position. */
	uint16_t encoderOffset;				

	/** Used to stored the previous measured angle for the speed
	 * measurement, and the calculation of angle moved from reference
	 * position */
	volatile uint16_t oldAngle;			

	/** This variable always contain the current rotor angle, relative
	* to a single revolution */
	volatile uint16_t angle;			
	
	/** This variable contains the number of revolutions in either
	 * direction, since last home position was set. negative numbers
	 * corresponds to CCW turns, and positive to CW turns */
	volatile int16_t revolutions;		

	/** Variable used to store the last measured rotational speed of
	* the motor shaft */
	volatile float curSpeed;			 	
	/**
	 * @brief      Constructor
	 *
	 *             This is the constructor of the uStepperEncoder class.
	 */
	uStepperEncoder(void);
	
	/**
	 * @brief      Measure the current shaft angle
	 *
	 *             This function reads the current angle of the motor shaft. The
	 *             resolution of the angle returned by this function is
	 *             0.087890625 degrees (12 bits) The Angle is read by means of
	 *             the I2C interface, using the I2C interface implemented in
	 *             this library.
	 *
	 * @return     Floating point representation of the current motor shaft
	 *             angle
	 */
	float getAngle(void);
	
	/**
	 * @brief      Measure the current speed of the motor
	 *
	 *             This function returns the current speed of the motor. The
	 *             speed is not calculated in this function, it is merely
	 *             returning a variable. The speed is calculated in the
	 *             interrupt routine associated with timer1.
	 *
	 * @return     Current speed in revolutions per second (RPS)
	 */
	float getSpeed(void);
	
	/**
	 * @brief      Measure the strength of the magnet
	 *
	 *             This function returns the strength of the magnet
	 *
	 * @return     Strength of magnet
	 */
	uint16_t getStrength(void);
	
	/**
	 * @brief      Read the current AGC value of the encoder chip
	 *
	 *             This function returns the current value of the AGC register
	 *             in the encoder chip (AS5600). This value ranges between 0 and
	 *             255, and should preferably be as close to 128 as possible.
	 *
	 * @return     current AGC value
	 */
	uint8_t getAgc(void);
	
	/**
	 * @brief      Detect if magnet is present and within range
	 *
	 *             This function detects whether the magnet is present, too
	 *             strong or too weak.
	 *
	 * @return     0 - Magnet detected and within limits
	 * @return     1 - Magnet too strong
	 * @return     2 - Magnet too weak
	 */
	uint8_t detectMagnet(void);
	
	/**
	 * @brief      Measure the angle moved from reference position
	 *
	 *             This function measures the angle moved from the shaft
	 *             reference position. When the uStepper is first powered on,
	 *             the reference position is reset to the current shaft
	 *             position, meaning that this function will return the angle
	 *             rotated with respect to the angle the motor initially had. It
	 *             should be noted that this function is absolute to an
	 *             arbitrary number of revolutions !
	 *
	 *             The reference position can be reset at any point in time, by
	 *             use of the setHome() function.
	 *
	 * @return     The angle moved.
	 */
	float getAngleMoved(void);
	
	/**
	 * @brief      Setup the encoder
	 *
	 *             This function initializes all the encoder features.
	 *
	 * @param[in]  mode  Variable to indicate if the uStepper is in normal or
	 *                   drop-in mode
	 */
	void setup(uint8_t mode);
	
	/**
	 * @brief      Define new reference(home) position
	 *
	 *             This function redefines the reference position to the current
	 *             angle of the shaft
	 */
	void setHome(void);

private:

	friend void TIMER1_COMPA_vect(void) __attribute__ ((signal,used));
};

/**
 * @brief      Prototype of class for accessing all features of the uStepper in
 *             a single object.
 *
 *             This class enables the user of the library to access all features
 *             of the uStepper board, by use of a single object.
 */

class uStepper
{
private:
	//Address offset: 0	
	/** This variable is used by the stepper acceleration algorithm to set the delay between step pulses when running at the set cruise speed */
	uint16_t cruiseDelay;

	//Address offset: 2
	/** This is the constant multiplier used by the stepper algorithm.
	 * See description of timer2 overflow interrupt routine for more
	 * details. */		
	float2 multiplier;					
	
	//Address offset: 10
	/** This variable is used by the stepper algorithm to keep track of
	 * which part of the acceleration profile the motor is currently
	 * operating at. */
	uint8_t state;		

	//Address offset: 11
	/** This variable keeps track of how many steps to perform in the
	 * acceleration phase of the profile. */
	uint32_t accelSteps;						
	
	//Address offset: 15
	/** This variable keeps track of how many steps to perform in the
	 * deceleration phase of the profile. */
	uint32_t decelSteps;				
	
	//Address offset: 19
	/** This variable keeps track of how many steps to perform in the
	* initial deceleration phase of the profile. */
	uint32_t initialDecelSteps;				
	
	//Address offset: 23
	/** This variable keeps track of how many steps to perform in the
	* cruise phase of the profile. */
	uint32_t cruiseSteps;				
	
	//Address offset: 27
	/** This variable keeps track of the current step number in the
	* current move of a predefined number of steps. */
	uint32_t currentStep;			 
	
	//Address offset: 31
	/** This variable keeps track of the total number of steps to be
	* performed in the current move of a predefined number of steps. */
	uint32_t totalSteps;			
	
	//Address offset: 35
	/** This variable tells the algorithm whether the motor should
	 * rotated continuous or only a limited number of steps. If set to
	 * 1, the motor will rotate continous. */
	bool continous;					

	//Address offset: 36
	/** This variable tells the algorithm if it should block the motor
	 * by keeping the motor coils excited after the commanded number of
	 * steps have been carried out, or if it should release the motor
	 * coil, allowing the shaft to be rotated freely. */
	bool hold;						

	//Address offset: 37
	/** This variable tells the algorithm the direction of rotation for
	 * the commanded move. */
	bool direction;					
	
	//Address offset: 38
	/**This variable contains an open-loop number of steps moved from
	 * the position the motor had when powered on (or reset). a negative
	 * value represents a rotation in the counter clock wise direction
	 * and a positive value corresponds to a rotation in the clock wise
	 * direction. */
	volatile int32_t stepsSinceReset;

	/** Dummy variable, to make variable addresses fit with algorithm
	 *	After the stepsSinceReset has been changed to int32_t */
	int32_t dummy;		
	
	//Address offset: 46
	/** This variable contains the exact delay (in number of
	 * interrupts) before the next step is applied. This variable is
	 * used in the calculations of the next step delay. */
	float2 exactDelay;								
	
	//Address offset: 54	
	/** This variable is used by the stepper algorithm to keep track of
	 * when to apply the next step pulse. When the algorithm have
	 * applied a step pulse, it will calculate the next delay (in number
	 * of interrupts) needed before the next pulse should be applied. A
	 * truncated version of this delay will be put in this variable and
	 * is decremented by one for each interrupt untill it reaches zero
	 * and a step is applied. */
	uint16_t delay;				

	//Address offset: 56
	/** Not used anymore ! */
	bool dropIn;				

	//Address offset: 57
	/** This variable contains the maximum velocity, the motor is
	 * allowed to reach at any given point. The user of the library can
	 * set this by use of the setMaxVelocity() function, and get the
	 * current value with the getMaxVelocity() function. */
	float velocity;					

	//Address offset: 61
	/** This variable contains the maximum acceleration to be used. The
	 * can be set and read by the user of the library using the
	 * functions setMaxAcceleration() and getMaxAcceleration()
	 * respectively. Since this library uses a second order acceleration
	 * curve, the acceleration applied will always be either +/- this
	 * value (acceleration/deceleration)or zero (cruise). */
	float acceleration;				

	//address offset: 65
	/** This variable contains the number of missed steps allowed
 	* before the PID controller kicks in, if activated */	
	volatile float tolerance;		
	
	//address offset: 69
	/** This variable contains the error which the PID controller
	 * should have obtained in order to switch off */
	volatile float hysteresis;		
	
	//address offset: 73
	/** This variable contains the conversion coefficient from raw
	* encoder data to number of steps */					
	volatile float stepConversion;	

	//address offset: 77
	/** This variable is used by Timer2 to check wether it is time to
	* generate steps or not. only used if PID is activated */	
	volatile uint16_t counter;		
	
	//address offset: 79
	/** This variable contains the number of steps commanded by
	* external controller, in case of dropin feature */
	volatile int32_t stepCnt;		
	
	//address offset: 83
	/** This variable contains the number of steps we are off the
	* setpoint, and is updated once every PID sample. */
	volatile int32_t control;		
	
	//address offset: 87
	/** This variable contains the number of microseconds between last
	* step pulse from external controller */
	volatile uint32_t speedValue[2];
	
	//address offset: 95
	/** This variable contains the proportional coefficient used by the
	* PID */
	float pTerm;					
	
	//address offset: 99
	/** This variable contains the integral coefficient used by the PID */
	float iTerm;					

	//address offset: 103
	/** This variable contains the differential coefficient used by the
	* PID */
	float dTerm;					
	
	//address offset: 107
	/** This variable is used to indicate which mode the uStepper is
	* running in (Normal, dropin or pid)*/
	uint8_t mode;					

	/** This variable converts an angle in degrees into a corresponding
	 * number of steps*/
	float angleToStep;		

	/** This variable holds information on wether the motor is stalled or not.
	0 = OK, 1 = stalled */
	volatile bool stall;

	volatile bool invertDir;

	friend void TIMER2_COMPA_vect(void) __attribute__ ((signal,naked,used));
	friend void TIMER1_COMPA_vect(void) __attribute__ ((signal,used));
	friend void interrupt1(void);
	friend void uStepperEncoder::setHome(void);

	/**
	 * @brief      Starts timer for stepper algorithm
	 *
	 *             This function actually doesn't start the timer for the
	 *             stepper algorithm, as the timer is always running. Instead it
	 *             clears the counter value, clears any pending interrupts and
	 *             enables the timer compare match interrupt.
	 */
	void startTimer(void);
	

	/**
	 * @brief      Stops the timer for the stepper algorithm.
	 *
	 *             As the startTimer() function, this function doesn't stop the
	 *             timer, instead it disables the timer compare match interrupt
	 *             bit in the timer registers, ensuring that the stepper
	 *             algorithm will not run when the motor is not supposed to run.
	 */
	void stopTimer(void);
	

	/**
	 * @brief      Enables the stepper driver output stage.
	 *
	 *             This function enables the output stage of the stepper driver.
	 *             If no step pulses is applied to the stepper driver, either
	 *             manually or by means of the stepper algorithm of this
	 *             library, this will make the force the motor to brake in the
	 *             current position.
	 */
	void enableMotor(void);
	

	/**
	 * @brief      Disables the stepper driver output stage.
	 *
	 *             This function disables the stepper driver output stage. If
	 *             this function is used, the motor will not brake, and it will
	 *             be possible to turn the motor shaft by hand
	 */
	void disableMotor(void);

	/**
	 * @brief      This method handles the actual PID controller calculations
	 *             for drop-in feature, if enabled.
	 */
	void pidDropIn(void);

	/**
	 * @brief      This method handles the actual PID controller calculations,
	 *             if enabled.
	 */
	void pid(void);

public:
	/** Instantiate object for the temperature sensor */
	uStepperTemp temp;				
	/** Instantiate object for the encoder */
	uStepperEncoder encoder;		

	/**
	 * @brief      Constructor of uStepper class
	 *
	 *             This is the constructor of the uStepper class. This version
	 *             of the constructor takes in two arguments, "accel" and "vel".
	 *             These two arguments lets the programmer set the maximum
	 *             acceleration and velocity, respectively, during instantiation
	 *             of the uStepper object.
	 *
	 * @param      accel  - Floating point representation of the maximum
	 *                    acceleration allowed in steps/s^2.
	 * @param      vel    - Floating point representation of the maximum
	 *                    velocity allowed in steps/s.
	 */
	uStepper(float accel, float vel);
	

	/**
	 * @brief      Constructor of uStepper class
	 *
	 *             This is the constructor of the uStepper class. This version
	 *             of the constructor doesn't take any arguments, and
	 *             instantiates an object with a maximum acceleration and
	 *             velocity of 1000 steps/s^2 and 1000 steps/s, respectively.
	 */
	uStepper(void);
	

	/**
	 * @brief      Set the maximum acceleration of the stepper motor.
	 *
	 *             This function lets the user of this library set the
	 *             acceleration used by the stepper algorithm. The algorithm is
	 *             a second order acceleration profile, meaning that the
	 *             acceleration only assumes three values; -a, 0 and a, with a
	 *             being the acceleration set by this function.
	 *
	 * @param      accel  - Maximum acceleration in steps/s^2
	 */
	void setMaxAcceleration(float accel);
	

	/**
	 * @brief      Get the value of the maximum motor acceleration.
	 *
	 *             This function returns the maximum acceleration used by the
	 *             stepper algorithm.
	 *
	 * @return     Maximum acceleration in steps/s^2
	 */
	float getMaxAcceleration(void);
	

	/**
	 * @brief      Sets the maximum rotational velocity of the motor
	 *
	 *             This function sets the maximum velocity at which the motor is
	 *             allowed to run. The rotational velocity of the motor will
	 *             gradually be ramped up to the value set by this function, by
	 *             the stepper acceleration profile implemented in this library.
	 *
	 * @param      vel   - Maximum rotational velocity of the motor in steps/s
	 */
	void setMaxVelocity(float vel);
	

	/**
	 * @brief      Returns the maximum rotational velocity of the motor
	 *
	 *             This function returns the maximum rotational velocity the
	 *             motor is allowed to run. In order to change this velocity,
	 *             The function setMaximumVelocity() should be used.
	 *
	 * @return     maximum rotational velocity of the motor in steps/s.
	 */
	float getMaxVelocity(void);
	

	/**
	 * @brief      Make the motor rotate continuously
	 *
	 *             This function makes the motor rotate continuously, using the
	 *             acceleration profile implemented in this library. The motor
	 *             will accelerate at the rate set by setMaximumAcceleration(),
	 *             and eventually reach the speed set by setMaximumVelocity()
	 *             function. The direction of rotation is set by the argument
	 *             "dir".
	 *
	 * @param      dir   - Can be set to "CCW" or "CW" (without the quotes)
	 */
	void runContinous(bool dir);
	

	/**
	 * @brief      Make the motor perform a predefined number of steps
	 *
	 *             This function makes the motor perform a predefined number of
	 *             steps, using the acceleration profile implemented in this
	 *             library. The motor will accelerate at the rate set by
	 *             setMaximumAcceleration(), and eventually reach the speed set
	 *             by setMaximumVelocity() function. The direction of rotation
	 *             is set by the argument "dir". The argument "holdMode",
	 *             defines whether the motor should brake or let the motor
	 *             freewheel after the steps has been performed.
	 *
	 * @param      steps     -	Number of steps to be performed.
	 * @param      dir       -	Can be set to "CCW" or "CW" (without the quotes).
	 * @param      holdMode  -	can be set to "HARD" for brake mode or "SOFT" for
	 *                       freewheel mode (without the quotes).
	 */
	void moveSteps(int32_t steps, bool dir, bool holdMode);
	

	/**
	 * @brief      Stop the motor without deceleration
	 *
	 *             This function will stop any ongoing motor movement, without
	 *             any deceleration phase. If the motor is rotation at a
	 *             significant speed, the motor might not stop instantaneously,
	 *             due to build up inertia. The argument "holdMode" can be used
	 *             to define whether the motor should brake or freewheel after
	 *             the function has been called.
	 *
	 * @param      holdMode  -	can be set to "HARD" for brake mode or "SOFT" for
	 *                       freewheel mode (without the quotes).
	 */
	void hardStop(bool holdMode);
	

	/**
	 * @brief      Stop the motor with deceleration
	 *
	 *             This function stops any ongoing motor movement, with a
	 *             deceleration phase. This will take longer for the motor to
	 *             stop, however the mechanical vibrations related to the
	 *             stopping of the motor can be significantly reduced compared
	 *             to the hardStop() function. The argument "holdMode" can be
	 *             used to define whether the motor should brake or freewheel
	 *             after the function has been called.
	 *
	 * @param      holdMode  -	can be set to "HARD" for brake mode or "SOFT" for
	 *                       freewheel mode (without the quotes).
	 */
	void softStop(bool holdMode);
	

	/**
	 * @brief      Initializes the different parts of the uStepper object
	 *
	 *             This function initializes the different parts of the uStepper
	 *             object, and should be called in the setup() function of the
	 *             arduino sketch. This function is needed as some things, like
	 *             the timer can not be setup in the constructor, since arduino
	 *             for some strange reason, resets a lot of the AVR registers
	 *             just before entering the setup() function.
	 *
	 * @param[in]  mode             Default is normal mode. Pass the constant
	 *                              "DROPIN" to configure the uStepper to act as
	 *                              dropin compatible to the stepstick. Pass the
	 *                              constant "PID", to enable PID feature for
	 *                              regular movement functions, such as
	 *                              moveSteps()
	 * @param[in]  microStepping    When mode is set to anythings else than
	 *                              "NORMAL", this parameter should be set to
	 *                              the current microstep setting. available
	 *                              arguments are: FULL HALF QUARTER EIGHT
	 *                              SIXTEEN
	 * @param[in]  faultTolerance   This parameter defines the allowed number of
	 *                              missed steps before the correction should
	 *                              kick in.
	 * @param[in]  faultHysteresis  The number of missed steps allowed for the
	 *                              PID to turn off
	 * @param[in]  pTerm            The proportional coefficent of the PID
	 *                              controller
	 * @param[in]  iTerm            The integral coefficent of the PID
	 *                              controller
	 * @param[in]  dterm            The differential coefficent of the PID
	 *                              controller
	 * @param[in]  setHome          When set to true, the encoder position is
	 *								Reset. When set to false, the encoder
	 *								position is not reset.
	 */
	void setup(	uint8_t mode = NORMAL, 
				uint8_t microStepping = SIXTEEN, 
				float faultTolerance = 10.0,
				float faultHysteresis = 5.0, 
				float pTerm = 1.0, 
				float iTerm = 0.02, 
				float dterm = 0.006,
				bool setHome = true);	

	/**
	 * @brief      Returns the direction the motor is currently configured to
	 *             rotate
	 *
	 *             This function checks the last configured direction of
	 *             rotation and returns this.
	 *
	 * @return     0 - Clockwise
	 * @return     1 - Counter Clockwise
	 */
	bool getCurrentDirection(void);
	

	/**
	 * @brief      Get the current state of the motor
	 *
	 *             This function returns the current state of the motor, i.e. if
	 *             the motor is rotating or not.
	 *
	 * @return     0 - Motor not rotating
	 * @return     1 - Motor rotating
	 */
	bool getMotorState(void);
	

	/**
	 * @brief      Get the number of steps applied since reset.
	 *
	 *             This function returns the number of steps applied to the
	 *             motor driver since reset. This function doesn't care if the
	 *             step has actually been performed or not. Steps applied in the
	 *             clockwise direction is added and steps applied in the counter
	 *             clockwise direction is subtracted. This means that a negative
	 *             return value indicates that the motor should have rotated the
	 *             returned amount of steps in the counter clockwise direction,
	 *             with respect to the initial position. Likewise a positive
	 *             value indicate the number of steps the motor should have
	 *             rotated in the clockwise direction, with respect to the
	 *             initial position.
	 *
	 * @return     negative value - number of steps the motor should have
	 *             rotated in the counter clockwise direction, with respect to
	 *             the initial position.
	 * @return     positive value - number of steps the motor should have rotated in
	 *             the clockwise direction, with respect to the initial position.
	 */
	int32_t getStepsSinceReset(void);

	/**
	 * @brief      Generate PWM signal on digital output 8
	 *
	 *             This function allows the user to generate PWM signal on
	 *             digital output 8. The PWM signal has a fixed frequency of
	 *             1kHz, from 0% - 100% duty cycle, in steps of 0.00625%
	 *             (resolution of 13.97 bits).
	 *
	 * @param      duty  - Desired duty cycle of PWM signal. range: 0.0 to
	 *                   100.0.
	 */
	void pwmD8(double duty);

	/**
	 * @brief      Set motor output current
	 *
	 *             This function allows the user to change the current setting of the motor
	 *             driver. In order to utilize this feature, the current jumper should be 
	 *             placed in the "I-PWM" position on the uStepper board.
	 *
	 * @param[in]  current Desired current setting in percent (0% - 100%)
	 */
	void setCurrent(double current);

	/**
	 * @brief      Sets the mode of digital pin D8
	 * 
	 * 			   This function changes digital pin D8 between PWM mode and normal I/O mode. 
	 *
	 * @param[in]  mode  By supplying 'PWM' as argument, the digital pin acts as a PWM pin.
	 * 				     By supplying 'NORMAL' as argument, the digital pin acts as a normal I/O pin.
	 */
	void pwmD8(int mode);
	
	/**
	 * @brief      Generate PWM signal on digital output 3
	 *
	 *             This function allows the user to generate PWM signal on
	 *             digital output 3. The PWM signal has a fixed frequency of
	 *             28.2kHz, from 0% - 100% duty cycle, in steps of 1.43%
	 *             (resolution of 6.13 bits).
	 *
	 * @param      duty  - Desired duty cycle of PWM signal. range: 0.0 to
	 *                   100.0.
	 */
	void pwmD3(double duty);


	/**
	 * @brief      Sets the mode of digital pin D3
	 * 
	 * 			   This function changes digital pin D3 between PWM mode and normal I/O mode. 
	 *
	 * @param[in]  mode  By supplying 'PWM' as argument, the digital pin acts as a PWM pin.
	 * 				     By supplying 'NORMAL' as argument, the digital pin acts as a normal I/O pin.
	 */
	void pwmD3(int mode);

	/**
	 * @brief      Updates setpoint for the motor
	 *
	 *             This method updates the setpoint for the motor. This function
	 *             is used when it is desired to provide an absolute position
	 *             for the motor, and should be used in the DROPIN mode
	 *
	 * @param[in]  setPoint  The setpoint in degrees
	 */
	void updateSetPoint(float setPoint);
	
	/**
	 * @brief      	Moves the motor to its physical limit, without limit switch
	 *
	 *              This function, makes the motor run continously, untill the
	 *				encoder detects a stall, at which point the motor is assumed
	 *				to be at it's limit.
	 *
	 * @param[in]  	dir  Direction to search for limit
	 *
	 * @return 		Degrees turned from calling the function, till end was reached
	 */
	float moveToEnd(bool dir);

	/**
	 * @brief      	Moves the motor to an absolute angle
	 *
	 * @param[in]  	angle  Absolute angle. A positive angle makes
	 *				the motor turn clockwise, and a negative angle, counterclockwise.
	 *
	 * @param[in]  	holdMode can be set to "HARD" for brake mode or "SOFT" for
	 *              freewheel mode (without the quotes).
	 */

	void moveToAngle(float angle, bool holdMode);

	/**
	 * @brief      	Moves the motor to a relative angle
	 *
	 * @param[in]  	angle  Relative angle from current position. A positive angle makes
	 *				the motor turn clockwise, and a negative angle, counterclockwise.
	 *
	 * @param[in]  	holdMode can be set to "HARD" for brake mode or "SOFT" for
	 *              freewheel mode (without the quotes).
	 */
	void moveAngle(float angle, bool holdMode);

	/**
	 * @brief      	This method returns a bool variable indicating wether the motor
	 *				is stalled or not
	 *
	 * @return     	0 = not stalled, 1 = stalled
	 */
	bool isStalled(void);

	bool detectStall(float diff, bool running);
};

/**
 * @brief      Prototype of class for accessing the TWI (I2C) interface of the
 *             AVR (master mode only).
 *
 *             This class enables the use of the hardware TWI (I2C) interface in
 *             the AVR (master mode only), which is used for interfacing with
 *             the encoder. This class is needed in this library, as arduino's
 *             build in "wire" library uses interrupts to access the TWI
 *             interface, and since the uStepper library needs to use the TWI
 *             interface within a timer interrupt, this library cannot be used.
 *             As a result of this, the "wire" library, cannot be used in
 *             sketches using the uStepper library, as this will screw with the
 *             setups, and make the sketch hang in the timer interrupt routine.
 *             Instead, if the programmer of the sketch needs to interface with
 *             external I2C devices, this class should be used. This library
 *             contains a predefined object called "I2C", which should be used
 *             for these purposes.
 *
 *             The functions "read()" and "write()", should be the only
 *             functions needed by most users of this library !
 */
class i2cMaster
{
	private:
		/** Contains the status of the I2C bus */
		uint8_t status;			


		/**
		 * @brief      Sends commands over the I2C bus.
		 *
		 *             This function is used to send different commands over the
		 *             I2C bus.
		 *
		 * @param      cmd   - Command to be send over the I2C bus.
		 */
		bool cmd(uint8_t cmd);

	public:

		/**
		 * @brief      Constructor
		 *
		 *             This is the constructor, used to instantiate an I2C
		 *             object. Under normal circumstances, this should not be
		 *             needed by the programmer of the arduino sketch, since
		 *             this library already has a global object instantiation of
		 *             this class, called "I2C".
		 */
		i2cMaster(void);
		
		/**
		 * @brief      Reads a byte from the I2C bus.
		 *
		 *             This function requests a byte from the device addressed
		 *             during the I2C transaction setup. The parameter "ack" is
		 *             used to determine whether the device should keep sending
		 *             data or not after the reception of the currently
		 *             requested data byte.
		 *
		 * @param      ack   - should be set to "ACK" if more bytes is wanted,
		 *                   and "NACK" if no more bytes should be send (without
		 *                   the quotes)
		 * @param      data  - Address of the variable to store the requested
		 *                   data byte
		 *
		 * @return     Always returns 1
		 */
		bool readByte(bool ack, uint8_t *data);

		/**
		 * @brief      sets up I2C connection to device, reads a number of data
		 *             bytes and closes the connection
		 *
		 *             This function is used to perform a read transaction
		 *             between the arduino and an I2C device. This function will
		 *             perform everything from setting up the connection,
		 *             reading the desired number of bytes and tear down the
		 *             connection.
		 *
		 * @param      slaveAddr   -	7 bit address of the device to read from
		 * @param      regAddr     -	8 bit address of the register to read from
		 * @param      numOfBytes  -	Number of bytes to read from the device
		 * @param      data        -	Address of the array/string to store the
		 *                         bytes read. Make sure enough space are
		 *                         allocated before calling this function !
		 *
		 * @return     1			-	Currently always returns this value. In the future
		 *             this value will be used to indicate successful
		 *             transactions.
		 */		
		bool read(uint8_t slaveAddr, uint8_t regAddr, uint8_t numOfBytes, uint8_t *data);

		/**
		 * @brief      sets up connection between arduino and I2C device.
		 *
		 *             This function sets up the connection between the arduino
		 *             and the I2C device desired to communicate with, by
		 *             sending a start condition on the I2C bus, followed by the
		 *             device address and a read/write bit.
		 *
		 * @param      addr  -	Address of the device it is desired to
		 *                   communicate with
		 * @param      RW    -	Can be set to "READ" to setup a read transaction
		 *                   or "WRITE" for a write transaction (without the
		 *                   quotes)
		 *
		 * @return     1		-	Connection properly set up
		 * @return     0		-	Connection failed
		 */
		bool start(uint8_t addr, bool RW);
		
		/**
		 * @brief      Restarts connection between arduino and I2C device.
		 *
		 *             This function restarts the connection between the arduino
		 *             and the I2C device desired to communicate with, by
		 *             sending a start condition on the I2C bus, followed by the
		 *             device address and a read/write bit.
		 *
		 * @param      addr  -	Address of the device it is desired to
		 *                   communicate with
		 * @param      RW    -	Can be set to "READ" to setup a read transaction
		 *                   or "WRITE" for a write transaction (without the
		 *                   quotes)
		 *
		 * @return     1		-	Connection properly set up
		 * @return     0		-	Connection failed
		 */
		bool restart(uint8_t addr, bool RW);
		
		/**
		 * @brief      Writes a byte to a device on the I2C bus.
		 *
		 *             This function writes a byte to a device on the I2C bus.
		 *
		 * @param      data  - Byte to be written
		 *
		 * @return     1	- Byte written successfully
		 * @return     0	- transaction failed
		 */
		bool writeByte(uint8_t data);
		
		/**
		 * @brief      sets up I2C connection to device, writes a number of data
		 *             bytes and closes the connection
		 *
		 *             This function is used to perform a write transaction
		 *             between the arduino and an I2C device. This function will
		 *             perform everything from setting up the connection,
		 *             writing the desired number of bytes and tear down the
		 *             connection.
		 *
		 * @param      slaveAddr   -	7 bit address of the device to write to
		 * @param      regAddr     -	8 bit address of the register to write to
		 * @param      numOfBytes  -	Number of bytes to write to the device
		 * @param      data        -	Address of the array/string containing data
		 *                         to write.
		 *
		 * @return     1			-	Currently always returns this value. In the future
		 *             this value will be used to indicate successful
		 *             transactions.
		 */		
		bool write(uint8_t slaveAddr, uint8_t regAddr, uint8_t numOfBytes, uint8_t *data);

		/**
		 * @brief      Closes the I2C connection
		 *
		 *             This function is used to close down the I2C connection,
		 *             by sending a stop condition on the I2C bus.
		 *
		 * @return     1	-	This is always returned Currently - in a later
		 *             version, this should indicate that connection is
		 *             successfully closed
		 */
		bool stop(void);
		
		/**
		 * @brief      Get current I2C status
		 *
		 *             This function returns the status of the I2C bus.
		 *
		 * @return     Status of the I2C bus. Refer to defines for possible
		 *             status
		 */
		uint8_t getStatus(void);
		
		/**
		 * @brief      Setup TWI (I2C) interface
		 *
		 *             This function sets up the TWI interface, and is
		 *             automatically called in the instantiation of the uStepper
		 *             encoder object.
		 */
		void begin(void);
};
/** Global definition of I2C object for use in arduino sketch */
extern i2cMaster I2C;		

#endif