/********************************************************************************************
* 	 	File: 		uStepper.h 																*
*		Version:    0.4.3                                           						*
*      	date: 		August 5th, 2016	                                    				*
*      	Author: 	Thomas Hørring Olsen                                   					*
*                                                   										*	
*********************************************************************************************
*	(C) 2016																				*
*																							*
*	ON Development IVS																		*
*	www.on-development.com 																	*
*	administration@on-development.com 														*
*																							*
*	The code contained in this file is released under the following open source license:	*
*																							*
*			Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International			*
* 																							*
* 	The code in this file is provided without warranty of any kind - use at own risk!		*
* 	neither ON Development IVS nor the author, can be held responsible for any damage		*
* 	caused by the use of the code contained in this file ! 									*
*                                                                                           *
********************************************************************************************/
/**
*	\mainpage Arduino library for the uStepper Board
*	
*	This is the uStepper Arduino library, providing software functions for the different features of the uStepper board.
*	
*	\par Features
*	The uStepper library contains the following features:
*	
*	- Second order acceleration profile, to provide smooth ramping of the speed and to avoid the motor stalling when demanding high speeds.
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
*	- Download uStepper.zip
*	- Open the Arduino IDE
*	- Select sketch from the top menu
*	- Select include library
*	- Select add .ZIP library
*	- Navigate to where you downloaded uStepper.zip to and select
*	- The library contains support for driving the stepper, measuring temperature and reading out encoder data. Two examples are included to show the functionality of the library. 
*	
*	The library is tested with Arduino IDE 1.6.8
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
*	- Implement PID-controller into dropin feature
*	- Implement multiaxis feature between multiple uSteppers
*	- Add support for limit switches
*	- Split the library into multiple files
*
*	\par Known Bugs
*	- No known bugs
*
*	\author Thomas Hørring Olsen (thomas@ustepper.com)
*	\par Change Log
*	\version 0.4.3:
*	- Fixed bug where initial deceleration phase (used when changing speed setting or changing directionwhile motor is moving), would never be entered, causing motor to accelerate
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

/**	\file uStepper.h
*	\brief Function prototypes and definitions for the uStepper library
*	
*	This file contains class and function prototypes for the library, as well as 
*	necessary constants and global variables.
*	
*	\author Thomas Hørring Olsen (thomas@ustepper.com)
*/

#ifndef _USTEPPER_H_
#define _USTEPPER_H_

#ifndef __AVR_ATmega328P__
#error !!This library only supports the ATmega328p MCU!!
#endif

#include <inttypes.h>
#include <avr/io.h>
#include <Arduino.h>

#define FULL 1
#define HALF 2
#define QUARTER 4
#define EIGHT 8
#define SIXTEEN 16

#define NORMAL 0
#define DROPIN 1

#define STOP 1					/**< Value to put in state variable in order to indicate that the motor should not be running */
#define ACCEL 2					/**< Value to put in state variable in order to indicate that the motor should be accelerating */
#define CRUISE 4				/**< Value to put in state variable in order to indicate that the motor should be decelerating */
#define DECEL 8					/**< Value to put in state variable in order to indicate that the motor should be cruising at constant speed with no acceleration */
#define INITDECEL 16			/**< Value to put in state variable in order to indicate that the motor should be decelerating to full stop before changing direction */
#define INTFREQ 28200.0f        /**< Frequency of interrupt routine, in which the delays for the stepper algorithm is calculated */ 
#define CW 0					/**< Value to put in direction variable in order for the stepper to turn clockwise */
#define CCW 1					/**< Value to put in direction variable in order for the stepper to turn counterclockwise */
#define HARD 1					/**< Value to put in hold variable in order for the motor to block when it is not running */
#define SOFT 0					/**< Value to put in hold variable in order for the motor to \b not block when it is not running */

#define ENCODERADDR 0x36		/**< I2C address of the encoder chip */

#define ANGLE 0x0E				/**< Address of the register, in the encoder chip, containing the 8 least significant bits of the stepper shaft angle */
#define STATUS 0x0B				/**< Address of the register, in the encoder chip, containing information about whether a magnet has been detected or not */
#define AGC 0x1A				/**< Address of the register, in the encoder chip, containing information about the current gain value used in the encoder chip. This value should preferably be around 127 (Ideal case!) */
#define MAGNITUDE 0x1B			/**< Address of the register, in the encoder chip, containing the 8 least significant bits of magnetic field strength measured by the encoder chip */

#define ENCODERINTFREQ 1000.0	/**< Frequency at which the encoder is sampled, for keeping track of angle moved and current speed */
#define ENCODERSPEEDCONSTANT ENCODERINTFREQ/10.0/360.0	/**< Constant to convert angle difference between two interrupts to speed in revolutions per second */

#define R 4700.0 				/**< The NTC resistor used for measuring temperature, is placed in series with a 4.7 kohm resistor. This is used to calculate the temperature */

#define I2CFREE 0				/**< I2C bus is not currently in use */

#define READ  1					/**< Value for RW bit in address field, to request a read */

#define WRITE 0					/**< Value for RW bit in address field, to request a write */

#define START  0x08				/**< start condition transmitted */

#define REPSTART 0x10			/**< repeated start condition transmitted */

#define TXADDRACK  0x18			/**< slave address plus write bit transmitted, ACK received */

#define TXDATAACK 0x28			/**< data transmitted, ACK received */

#define RXADDRACK 0x40			/**< slave address plus read bit transmitted, ACK received */

#define ACK 1					/**< value to indicate ACK for i2c transmission */

#define NACK 0					/**< value to indicate NACK for i2c transmission */

/**
*	Coefficients needed by the Steinhart-hart equation in order to find the temperature of the NTC (and hereby the temperature of the motor driver)
*	from the current resistance of the NTC resistor. The coefficients are calculated for the following 3 operating points:
*
*	A: T = 5 degree Celsius
*
*	B: T = 50 degree Celsius
*
*	C: T = 105 degree Celsius
*
*	The Steinhart-Hart equation is described at the following link:
*
*	https://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation#Developers_of_the_equation
*
*/

#define A 0.001295752996237  	
#define B 0.000237488365866  /**< See description of A */
#define C 0.000000083423218  /**< See description of A */

#define ALPHA 0.85
#define BETA (1.0 - ALPHA)


extern "C" void INT0_vect(void) __attribute__ ((signal));
extern "C" void INT1_vect(void) __attribute__ ((signal));
extern "C" void TIMER2_COMPA_vect(void) __attribute__ ((signal,naked));
extern "C" void TIMER1_COMPA_vect(void) __attribute__ ((signal));

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
		friend void TIMER2_COMPA_vect(void) __attribute__ ((signal));
		
};

/**
*	\brief Prototype of class for the temperature sensor 
*
*	This class enables the user of the library to access the temperature sensor
*	on the uStepper board.
*	This class can be instantiated as a standalone object if this is the only feature
*	on the uStepper required by the programmers specific application.
*/

class uStepperTemp
{
public:
	
	/**
	*	\brief Constructor
	*	
	*	This is the constructor of the temperature sensor class, and should be 
	*	used in order to instantiate a temperature sensor object as follows:
	*
	*	\code{.cpp}
	*		uStepperTemp temp;
	*	\endcode
	*
	*/

	uStepperTemp(void);
	
	/**
	*	\brief Request a reading of current temperature
	*
	*	This method performs a reading of the current NTC temperature and returns this value. 
	*	Since the NTC resistor is physically placed close to the stepper driver chip on the uStepper board,
	*	the temperature returned by this method will be close to the actual temperature of the driver chip.
	*	
	*	\return Current NTC Temperature in degree Celsius
	*/

	float getTemp(void);
private:
};

/**
*	\brief Prototype of class for the AS5600 encoder 
*
*	This class enables the user of the library to access the AS5600 encoder on the uStepper board.
*	This class can be instantiated as a standalone object if all the features of the uStepper is not needed by the programmers specific application.
*/

class uStepperEncoder
{
public:
	/**
	*	\brief Constructor
	*
	*	This is the constructor of the uStepperEncoder class.
	*/
	uStepperEncoder(void);
	
	/**
	*	\brief Measure the current shaft angle 
	*
	*	This function reads the current angle of the motor shaft. The resolution of the angle returned by this function is 0.087890625 degrees (12 bits)
	*	The Angle is read by means of the I2C interface, using the I2C interface implemented in this library.
	*
	*	\return Floating point representation of the current motor shaft angle
	*/
	float getAngle(void);
	
	/**
	*	\brief	Measure the current speed of the motor 
	*	
	*	This function returns the current speed of the motor. The speed is not calculated in this function, it is merely returning a variable.
	*	The speed is calculated in the interrupt routine associated with timer1.
	*
	*	\return Current speed in revolutions per second (RPS) 
	*
	*/
	float getSpeed(void);
	
	/**
	*	\brief 	Measure the strength of the magnet
	*
	*	This function returns the strength of the magnet
	*
	*	\return Strength of magnet
	*
	*/
	uint16_t getStrength(void);
	
	/**
	*	\brief 	Read the current AGC value of the encoder chip 
	*
	*	This function returns the current value of the AGC register in the encoder chip (AS5600).
	*	This value ranges between 0 and 255, and should preferably be as close to 128 as possible.
	*
	*	\return current AGC value
	*/
	uint8_t getAgc(void);
	
	/**
	*	\brief Detect if magnet is present and within range
	*
	*	This function detects whether the magnet is present, too strong or too weak.
	*
	*	\return 0 - Magnet detected and within limits
	*	\return 1 - Magnet too strong
	*	\return 2 - Magnet too weak
	*
	*/
	uint8_t detectMagnet(void);
	
	/**
	*	\brief Measure the angle moved from reference position
	*	
	*	This function measures the angle moved from the shaft reference position.
	*	When the uStepper is first powered on, the reference position is reset to the current shaft position, meaning that this function
	*	will return the angle rotated with respect to the angle the motor initially had. It should be noted that this function is absolute to
	*	an arbitrary number of revolutions !
	*	
	*	The reference position can be reset at any point in time, by use of the setHome() function.
	*
	*	
	*
	*/
	float getAngleMoved(void);
	
	/**
	*	\brief Setup the encoder
	*	
	*	This function initializes all the encoder features.
	*
	*/
	void setup(void);
	
	/**
	*	\brief Define new reference(home) position
	*
	*	This function redefines the reference position to the current angle of the shaft
	*
	*/
	void setHome(void);

private:

	friend void TIMER1_COMPA_vect(void) __attribute__ ((signal));

	float encoderOffset;				/**< Angle of the shaft at the reference position. */
	volatile float oldAngle;			/**< Used to stored the previous measured angle for the speed measurement, and the calculation of angle moved from reference position */
	volatile float angle;
	volatile float curSpeed;			/**< Variable used to store the last measured rotational speed of the motor shaft */ 
	volatile float angleMoved;			/**< Variable used to store that measured angle moved from the reference position */
};

/**
*	\brief Prototype of class for accessing all features of the uStepper in a single object. 
*
*	This class enables the user of the library to access all features of the uStepper board, by use of a single object.
*
*/

class uStepper
{
private:
	//Address offset: 0	
	uint16_t cruiseDelay;			/**< This variable is used by the stepper acceleration algorithm to set the delay between step pulses when running at the set cruise speed */
	//Address offset: 2
	float2 multiplier;				/**< This is the constant multiplier used by the stepper algorithm. See description of timer2 overflow interrupt routine for more details. */			
	//Address offset: 10
	uint8_t state;					/**< This variable is used by the stepper algorithm to keep track of which part of the acceleration profile the motor is currently operating at. */					
	//Address offset: 11
	uint32_t accelSteps;			/**< This variable keeps track of how many steps to perform in the acceleration phase of the profile. */			
	//Address offset: 15
	uint32_t decelSteps;			/**< This variable keeps track of how many steps to perform in the deceleration phase of the profile. */	
	//Address offset: 19
	uint32_t initialDecelSteps;		/**< This variable keeps track of how many steps to perform in the initial deceleration phase of the profile. */		
	//Address offset: 23
	uint32_t cruiseSteps;			/**< This variable keeps track of how many steps to perform in the cruise phase of the profile. */	
	//Address offset: 27
	uint32_t currentStep;			/**< This variable keeps track of the current step number in the current move of a predefined number of steps. */ 
	//Address offset: 31
	uint32_t totalSteps;			/**< This variable keeps track of the total number of steps to be performed in the current move of a predefined number of steps. */
	//Address offset: 35
	bool continous;					/**< This variable tells the algorithm whether the motor should rotated continuous or only a limited number of steps. If set to 1, the motor will rotate continous. */
	//Address offset: 36
	bool hold;						/**< This variable tells the algorithm if it should block the motor by keeping the motor coils excited after the commanded number of steps have been carried out, or if it should release the motor coil, allowing the shaft to be rotated freely. */
	//Address offset: 37
	bool direction;					/**< This variable tells the algorithm the direction of rotation for the commanded move. */
	//Address offset: 38
	int64_t stepsSinceReset;		/**< This variable contains an open-loop number of steps moved from the position the motor had when powered on (or reset). a negative value represents a rotation in the counter clock wise direction and a positive value corresponds to a rotation in the clock wise direction. */
	//Address offset: 46
	float2 exactDelay;				/**< This variable contains the exact delay (in number of interrupts) before the next step is applied. This variable is used in the calculations of the next step delay. */				
	//Address offset: 54	
	uint16_t delay;					/**< This variable is used by the stepper algorithm to keep track of when to apply the next step pulse. When the algorithm have applied a step pulse, it will calculate the next delay (in number of interrupts) needed before the next pulse should be applied. A truncated version of this delay will be put in this variable and is decremented by one for each interrupt untill it reaches zero and a step is applied. */
	//Address offset: 56
	bool dropIn;
	//Address offset: 57
	float velocity;					/**< This variable contains the maximum velocity, the motor is allowed to reach at any given point. The user of the library can set this by use of the setMaxVelocity() function, and get the current value with the getMaxVelocity() function. */
	//Address offset: 61
	float acceleration;				/**< This variable contains the maximum acceleration to be used. The can be set and read by the user of the library using the functions setMaxAcceleration() and getMaxAcceleration() respectively. Since this library uses a second order acceleration curve, the acceleration applied will always be eith +/- this value (acceleration/deceleration)or zero (cruise). */
	volatile uint16_t faultStepDelay;
	volatile float tolerance;
	volatile float stepResolution;


	friend void TIMER2_COMPA_vect(void) __attribute__ ((signal,naked));
	friend void TIMER1_COMPA_vect(void) __attribute__ ((signal));


	/**
	*	\brief Starts timer for stepper algorithm
	*	
	*	This function actually doesn't start the timer for the stepper algorithm, as the timer is always running. 
	*	Instead it clears the counter value, clears any pending interrupts and enables the timer compare match interrupt.
	*
	*/
	void startTimer(void);
	

	/**
	*	\brief Stops the timer for the stepper algorithm.
	* 
	*	As the startTimer() function, this function doesn't stop the timer, instead it disables the timer compare match interrupt
	*	bit in the timer registers, ensuring that the stepper algorithm will not run when the motor is not supposed to run.
	*
	*/
	void stopTimer(void);
	

	/**
	*	\brief Enables the stepper driver output stage.
	*
	*	This function enables the output stage of the stepper driver. If no step pulses is applied to the stepper driver, either manually or by means of the 
	*	stepper algorithm of this library, this will make the force the motor to brake in the current position.
	*
	*/
	void enableMotor(void);
	

	/**
	*	\brief Disables the stepper driver output stage.
	*
	*	This function disables the stepper driver output stage. If this function is used, the motor will not brake, and it will be possible to turn the motor shaft by hand
	*
	*/
	void disableMotor(void);

public:
	uStepperTemp temp;				/**< Instantiate object for the temperature sensor */
	uStepperEncoder encoder;		/**< Instantiate object for the encoder */


	/**
	*	\brief Constructor of uStepper class 
	*
	*	This is the constructor of the uStepper class. This version of the constructor takes in two arguments, "accel" and "vel". These two arguments
	*	lets the programmer set the maximum acceleration and velocity, respectively, during instantiation of the uStepper object.
	*
	*	\param accel 	- Floating point representation of the maximum acceleration allowed in steps/s^2.
	*	\param vel 		- Floating point representation of the maximum velocity allowed in steps/s.
	*
	*/
	uStepper(float accel, float vel);
	

	/**
	*	\brief Constructor of uStepper class
	*
	*	This is the constructor of the uStepper class. This version of the constructor doesn't take any arguments, and instantiates an object with a maximum 
	*	acceleration and velocity of 1000 steps/s^2 and 1000 steps/s, respectively.
	*
	*/
	uStepper(void);
	

	/**
	*	\brief Set the maximum acceleration of the stepper motor.
	*
	*	This function lets the user of this library set the acceleration used by the stepper algorithm.
	*	The algorithm is a second order acceleration profile, meaning that the acceleration only assumes
	*	three values; -a, 0 and a, with a being the acceleration set by this function.
	*
	*	\param Maximum acceleration in steps/s^2
	*
	*/
	void setMaxAcceleration(float accel);
	

	/**
	*	\brief Get the value of the maximum motor acceleration.
	*
	*	This function returns the maximum acceleration used by the stepper algorithm.
	*
	*	\return Maximum acceleration in steps/s^2
	*	
	*/
	float getMaxAcceleration(void);
	

	/**
	*	\brief Sets the maximum rotational velocity of the motor
	*
	*	This function sets the maximum velocity at which the motor is allowed to run. The rotational velocity
	*	of the motor will gradually be ramped up to the value set by this function, by the stepper acceleration
	*	profile implemented in this library.
	*
	*	\param Maximum rotational velocity of the motor in steps/s
	*
	*/
	void setMaxVelocity(float vel);
	

	/**
	*	\brief Returns the maximum rotational velocity of the motor
	*
	*	This function returns the maximum rotational velocity the motor is allowed to run. In order to change this velocity,
	*	The function setMaximumVelocity() should be used.
	*
	*	\return maximum rotational velocity of the motor in steps/s.
	*
	*/
	float getMaxVelocity(void);
	

	/**
	*	\brief Make the motor rotate continuously
	*
	*	This function makes the motor rotate continuously, using the acceleration profile implemented in this library.
	*	The motor will accelerate at the rate set by setMaximumAcceleration(), and eventually reach the speed
	*	set by setMaximumVelocity() function. The direction of rotation is set by the argument "dir".
	*
	*	\param dir - Can be set to "CCW" or "CW" (without the quotes)
	*
	*/
	void runContinous(bool dir);
	

	/**
	*	\brief Make the motor perform a predefined number of steps 
	*	
	*	This function makes the motor perform a predefined number of steps, using the acceleration profile implemented in this library.
	*	The motor will accelerate at the rate set by setMaximumAcceleration(), and eventually reach the speed
	*	set by setMaximumVelocity() function. The direction of rotation is set by the argument "dir". The argument "holdMode",
	*	defines whether the motor should brake or let the motor freewheel after the steps has been performed.
	*
	*	\param steps 	-	Number of steps to be performed.
	*	\param dir 		-	Can be set to "CCW" or "CW" (without the quotes).
	*	\param holdMode	-	can be set to "HARD" for brake mode or "SOFT" for freewheel mode (without the quotes).
	*
	*/
	void moveSteps(uint32_t steps, bool dir, bool holdMode);
	

	/**
	*	\brief Stop the motor without deceleration
	*
	*	This function will stop any ongoing motor movement, without any deceleration phase. If the motor is rotation at a significant
	*	speed, the motor might not stop instantaneously, due to build up inertia. The argument "holdMode" can be used to define
	*	whether the motor should brake or freewheel after the function has been called.
	*
	*	\param holdMode	-	can be set to "HARD" for brake mode or "SOFT" for freewheel mode (without the quotes).	
	*
	*/
	void hardStop(bool holdMode);
	

	/**
	*	\brief Stop the motor with deceleration
	*
	*	This function stops any ongoing motor movement, with a deceleration phase. This will take longer for the motor to stop,
	*	however the mechanical vibrations related to the stopping of the motor can be significantly reduced compared to the hardStop()
	*	function. The argument "holdMode" can be used to define whether the motor should brake or freewheel after the function has been called.
	*
	*	\param holdMode	-	can be set to "HARD" for brake mode or "SOFT" for freewheel mode (without the quotes).
	*
	*/
	void softStop(bool holdMode);
	

	/**
	*	\brief Initializes the different parts of the uStepper object
	*
	*	This function initializes the different parts of the uStepper object, and should be called in the setup() function of the 
	*	arduino sketch. This function is needed as some things, like the timer can not be setup in the constructor, since arduino
	*	for some strange reason, resets a lot of the AVR registers just before entering the setup() function.
	*
	*/
	void setup(bool mode = NORMAL, uint8_t microStepping = SIXTEEN, float faultSpeed = 3000.0, uint32_t faultTolerance = 20);
	

	/**
	*	\brief Returns the direction the motor is currently configured to rotate
	*
	*	This function checks the last configured direction of rotation and returns this.
	*
	*	\return 0 - Counter clockwise
	*	\return 1 - Clockwise
	*
	*/
	bool getCurrentDirection(void);
	

	/**
	*	\brief Get the current state of the motor
	*
	*	This function returns the current state of the motor, i.e. if the motor is rotating or not.
	*
	*	\return 0 - Motor not rotating
	*	\return 1 - Motor rotating 
	*
	*/
	bool getMotorState(void);
	

	/**
	*	\brief Get the number of steps applied since reset.
	*
	*	This function returns the number of steps applied to the motor driver since reset. This function doesn't care if the step has actually been performed
	*	or not. Steps applied in the clockwise direction is added and steps applied in the counter clockwise direction is subtracted. 
	*	This means that a negative return value indicates that the motor should have rotated the returned amount of steps in the counter clockwise
	*	direction, with respect to the initial position. Likewise a positive value indicate the number of steps the motor should have rotated in the clockwise 
	*	direction, with respect to the initial position.
	*
	*	\return negative value - number of steps the motor should have rotated in the counter clockwise direction, with respect to the initial position.
	*	\return positive value - number of steps the motor should have rotated in the clockwise direction, with respect to the initial position.
	*
	*/
	int64_t getStepsSinceReset(void);
};

/**
*	\brief Prototype of class for accessing the TWI (I2C) interface of the AVR (master mode only). 
*
*	This class enables the use of the hardware TWI (I2C) interface in the AVR (master mode only), which is used for interfacing with the encoder.
*	This class is needed in this library, as arduino's build in "wire" library uses interrupts to access the TWI interface, and since the uStepper
*	library needs to use the TWI interface within a timer interrupt, this library cannot be used. As a result of this, the "wire" library, cannot
*	be used in sketches using the uStepper library, as this will screw with the setups, and make the sketch hang in the timer interrupt routine.
*	Instead, if the programmer of the sketch needs to interface with external I2C devices, this class should be used. This library contains a
*	predefined object called "I2C", which should be used for these purposes.
*
*	The functions "read()" and "write()", should be the only functions needed by most users of this library !
*
*/
class i2cMaster
{
	private:
		uint8_t status;			/**< Contains the status of the I2C bus */


		/**
		*	\brief Sends commands over the I2C bus.
		*
		*	This function is used to send different commands over the I2C bus.
		*
		*	\param cmd - Command to be send over the I2C bus.
		*
		*/
		void cmd(uint8_t cmd);

	public:

		/**
		*	\brief Constructor
		*	
		*	This is the constructor, used to instantiate an I2C object. Under normal circumstances, this should not be needed by the
		*	programmer of the arduino sketch, since this library already has a global object instantiation of this class, called "I2C".
		*
		*/
		i2cMaster(void);
		
		/**
		*	\brief Reads a byte from the I2C bus.
		*
		*	This function requests a byte from the device addressed during the I2C transaction setup. The parameter "ack" is used to 
		*	determine whether the device should keep sending data or not after the reception of the currently requested data byte.
		*
		*	\param data		- Address of the variable to store the requested data byte
		*	\param ack 		- should be set to "ACK" if more bytes is wanted, and "NACK" if no more bytes should be send (without the quotes)
		*
		*/
		bool readByte(bool ack, uint8_t *data);

		/**
		*	\brief sets up I2C connection to device, reads a number of data bytes and closes the connection
		*
		*	This function is used to perform a read transaction between the arduino and an I2C device. This function will perform everything 
		*	from setting up the connection, reading the desired number of bytes and tear down the connection.
		*
		*	\return 1			-	Currently always returns this value. In the future this value will be used to indicate successful transactions.
		*	\param slaveAddr	-	7 bit address of the device to read from
		*	\param regAddr		-	8 bit address of the register to read from
		*	\param numOfBytes	-	Number of bytes to read from the device
		*	\param data			-	Address of the array/string to store the bytes read. Make sure enough space are allocated before calling this function !	
		*
		*/		
		bool read(uint8_t slaveAddr, uint8_t regAddr, uint8_t numOfBytes, uint8_t *data);

		/**
		*	\brief sets up connection between arduino and I2C device.
		*
		*	This function sets up the connection between the arduino and the I2C device desired to communicate with, by sending
		*	a start condition on the I2C bus, followed by the device address and a read/write bit.
		*
		*	\param addr 	-	Address of the device it is desired to communicate with
		*	\param RW 		-	Can be set to "READ" to setup a read transaction or "WRITE" for a write transaction (without the quotes)
		*	\return 1		-	Connection properly set up
		*	\return 0		-	Connection failed
		*
		*/
		bool start(uint8_t addr, bool RW);
		
		/**
		*	\brief Restarts connection between arduino and I2C device.
		*
		*	This function restarts the connection between the arduino and the I2C device desired to communicate with, by sending
		*	a start condition on the I2C bus, followed by the device address and a read/write bit.
		*
		*	\param addr 	-	Address of the device it is desired to communicate with
		*	\param RW 		-	Can be set to "READ" to setup a read transaction or "WRITE" for a write transaction (without the quotes)
		*	\return 1		-	Connection properly set up
		*	\return 0		-	Connection failed
		*
		*/
		bool restart(uint8_t addr, bool RW);
		
		/**
		*	\brief Writes a byte to a device on the I2C bus.
		*
		*	This function writes a byte to a device on the I2C bus.
		*
		*	\param data - Byte to be written
		*	\return 1	- Byte written successfully
		*	\return 0	- transaction failed 
		*
		*/
		bool writeByte(uint8_t data);
		
		/**
		*	\brief sets up I2C connection to device, writes a number of data bytes and closes the connection
		*
		*	This function is used to perform a write transaction between the arduino and an I2C device. This function will perform everything 
		*	from setting up the connection, writing the desired number of bytes and tear down the connection.
		*
		*	\return 1			-	Currently always returns this value. In the future this value will be used to indicate successful transactions.
		*	\param slaveAddr	-	7 bit address of the device to write to
		*	\param regAddr		-	8 bit address of the register to write to
		*	\param numOfBytes	-	Number of bytes to write to the device
		*	\param data			-	Address of the array/string containing data to write.	
		*
		*/		
		bool write(uint8_t slaveAddr, uint8_t regAddr, uint8_t numOfBytes, uint8_t *data);

		/**
		*	\brief Closes the I2C connection
		*
		*	This function is used to close down the I2C connection, by sending a stop condition on the I2C bus.
		*
		*	\return 1	-	This is always returned Currently - in a later version, this should indicate that connection is successfully closed
		*
		*/
		bool stop(void);
		
		/**
		*	\brief Get current I2C status
		*
		*	This function returns the status of the I2C bus.
		*
		*	\return Status of the I2C bus. Refer to defines for possible status
		*
		*/
		uint8_t getStatus(void);
		
		/**
		*	\brief Setup TWI (I2C) interface
		*
		*	This function sets up the TWI interface, and is automatically called in the instantiation of the uStepper encoder object.
		*
		*/
		void begin(void);
};

extern i2cMaster I2C;		/**< Global definition of I2C object for use in arduino sketch */

#endif