/********************************************************************************************
*    	File: 		uStepper.S 																*
*		Version:    1.3.0                                          							*
*      	date: 		january 10th, 2018	                                    				*
*      	Author: 	Thomas Hørring Olsen                                   					*
*                                                   										*	
*********************************************************************************************
*			            uStepper acceleration algorithm   									*
* 																							*
* 	This file contains the source code to calculate the acceleration profile 				*
* 	for the attached stepper motor. This code is to be executed in an interrupt 			*
* 	routine, interrupting at a constant rate. The rate at which the interrupt 				*
* 	routine interrupts, is not important directly for this code, since this is 				*
*	taken into account by the setup routines. However, the interrupt frequency				*
*	needs to be slower or equal to the worst case execution time of the code 				*
*	contained in this file.                                                 				* 
*																							*
*	The algorithm used is NOT designed by me, I ONLY implemented the algorithm! 			*
*	I therefore give credit to the appropriate author: Aryeh Eiderman.						*
*																							*
*	for those interested, the deriviation of the algorithm used can be found				*
*	at the following webpage (Which is neither owned nor affiliated with me in any 			*
*	way):																					*
*																							*
*						http://hwml.com/LeibRamp.pdf										*
*																							*
*	In order to ensure high performance, the algorithm is implented using 					*
*	fixed point arithmetic instead of floating point arithmetic.							*
*																							*
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
 * @file uStepper.S
 * @brief      Stepper acceleration profile
 *
 *             This file contains the assembly implementations of the stepper
 *             acceleration algorithm. The algorithm is implemented in assembly
 *             in order to get the best possible performance.
 *
 * @author     Thomas Hørring Olsen (thomas@ustepper.com)
 */

.global _AccelerationAlgorithm			;The label "_AccelerationAlgorithm" needs to be accessible globally, since it is called from uStepper.cpp

.section .text							;Put the following code in program memory
_AccelerationAlgorithm:
;Definitions, similar to #define in C
.equ _STOP,1					
.equ _ACCEL,2
.equ _CRUISE,4
.equ _DECEL,8
.equ _INITDECEL,16
.equ _STOPBIT,0					
.equ _ACCELBIT,1
.equ _CRUISEBIT,2
.equ _DECELBIT,3
.equ _INITDECELBIT,4
.equ _SREG,0x3F
.equ _CW,0
.equ _CCW,1

;Offset in ram of the different variables used by the algorithm, with regards to the uStepper object address
.equ CRUISEDELAY,0
.equ MULTIPLIER,2
.equ STATE,10
.equ ACCELSTEPS,11
.equ DECELSTEPS,15
.equ INITIALDECELSTEPS,19
.equ CRUISESTEPS,23
.equ CURRENTSTEP,27
.equ CONTINOUS,35
.equ HOLD,36
.equ DIRECTION,37
.equ STEPSSINCERESET,38
.equ EXACTDELAY,46
.equ DELAY,54

;Push all registers used in this assembly code to stack, in order to not fuck up flow of code when leaving the interrupt routine (29 clk cycles total!)
push r0
push r1
push r15
push r17
push r18
push r19
push r20
push r21
push r22
push r23

ldd r16,z+STATE				;Load state variable into r16
sbrc r16,_STOPBIT			;Check if state is set to STOP
rjmp _stopMotor				;If state is set to STOP, kill timer interrupt.
ldd r17,z+DELAY				;Load lower part of delay variable
ldd r18,z+DELAY+1			;Load upper part of delay variable
cpi r17,0					;Check if upper part of delay variable is equal to 0
brne _notRdy				;If upper part of delay variable not equal to 0, we should decrement delay variable and leave routine
cpi r18,0					;Check if lower part of delay variable is equal to 0
brne _notRdy				;If lower part of delay variable not equal to 0, we should decrement delay variable and leave routine

sbi 0x0B,4					;Pull Step pin high, to issue a step

rjmp _rdy

_notRdy:					;Not ready to issue step
subi r17,1					;Decrement delay variable
sbci r18,0					;if lower part of delay variable was 0 (carry flag was set during last subtraction), subtract 1 from upper part
std z+DELAY,r17				;store lower part of delay variable back into ram
std z+DELAY+1,r18			;Store upper part of delay variable back into ram
rjmp _end					;We are done 

;Load currentSteps variable, add 1 to this and store it back into ram
;This is done so we can keep track of how many steps the motor have performed,
;and add this to the total number of steps performed since reset, at the end
;of the movement.
;
;50 clock cycles has been used to reach this point !
;
_rdy:
ldd r19,z+CURRENTSTEP				
ldd r20,z+CURRENTSTEP+1
ldd r21,z+CURRENTSTEP+2
ldd r22,z+CURRENTSTEP+3
ldi r18,1
ldi r23,0
add r19,r18
adc r20,r23
adc r21,r23
adc r22,r23
std z+CURRENTSTEP,r19
std z+CURRENTSTEP+1,r20
std z+CURRENTSTEP+2,r21
std z+CURRENTSTEP+3,r22
/*
ldi r19,37
add r30,r19
clr r19
adc r31,r19*/

ldd r19,z+STEPSSINCERESET				
ldd r20,z+STEPSSINCERESET+1
ldd r21,z+STEPSSINCERESET+2
ldd r22,z+STEPSSINCERESET+3

ldd r18,z+DIRECTION
cpi r18,0
brne _subStepCnt

ldi r18,1
ldi r23,0
add r19,r18
adc r20,r23
adc r21,r23
adc r22,r23
rjmp _storeStepCnt

_subStepCnt:
subi r19,1
sbci r20,0
sbci r21,0
sbci r22,0

_storeStepCnt:
std z+STEPSSINCERESET,r19
std z+STEPSSINCERESET+1,r20
std z+STEPSSINCERESET+2,r21
std z+STEPSSINCERESET+3,r22

/*subi r30,37
sbci r31,0
*/
;
;72 clock cycles has been used to reach this point !
;
;Check which state the state machine is in
_stateCheck:
sbrc r16,_INITDECELBIT		;If this bit is not set, state is not equal to INITDECEL, and we should skip the next line
rjmp _initDecelAlgorithm	;If state variable is equal to INITDECEL, we should jump to the initial deceleration algorithm
sbrc r16,_DECELBIT			;If this bit is not set, state is not equal to DECEL, and we should skip the next line
rjmp _decelAlgorithm		;If state variable is equal to DECEL, we should jump to the deceleration algorithm
sbrc r16,_ACCELBIT			;If this bit is not set, state is not equal to ACCEL, and we should skip the next line
rjmp _accelAlgorithm		;If state variable is equal to ACCEL, we should jump to the accel algorithm
rjmp _cruiseAlgorithm		;If state variable is not equal to any of the above states, the state has to be equal to CRUISE

_stopMotor:					;Motor should stand still
;clear currentSteps variable and store this back into ram
clr r15
std z+CURRENTSTEP,r15
std z+CURRENTSTEP+1,r15
std z+CURRENTSTEP+2,r15
std z+CURRENTSTEP+3,r15

lds r16,0x70				;Load contents TIMSK2 register into r16
andi r16,0xFD				;Clear TOIE2 in TIMSK2 (kill timer)
sts 0x70,r16				;Store r16 back into TIMSK2 register
ldd r16,z+HOLD				;Load hold variable into r16
sbrc r16,0					;If lsb of hold variable is cleared the motor should not block, and we therefore skip next line 
rjmp _end					;If the lsb of hold variable, is NOT cleared, the motor should block and we jump directly to _end

sbi 0x05,0					;Pull ENA pin high in order to release the motor driver

rjmp _end

_accelAlgorithm:
;79 clock cycle has been used to reach this point !
ldd r16,z+ACCELSTEPS				;Load lower part of accelSteps variable into r16
ldd r17,z+ACCELSTEPS+1				;Load middle-lower part of accelSteps variable into r17
ldd r18,z+ACCELSTEPS+2				;Load middle-upper part of accelSteps variable into r18
ldd r19,z+ACCELSTEPS+3				;Load upper part of accelSteps variable into r19

;Make sure we don't execute the acceleration algorithm if accelSteps variable is equal to 0.
;if this happens, the code will decrement the variable, resulting in the variable overflowing and attaining a value of 2³² - 1, which is obviously wrong, and should be avoided
cpi r16,0					;Check if lower part of accelSteps is equal to 0
brne _accelAlgorithmStart	;If not, we can safely execute acceleration algorithm
cpi r17,0					;Check if lower-middle part of accelSteps is equal to 0
brne _accelAlgorithmStart	;If not, we can safely execute acceleration algorithm
cpi r18,0					;Check if upper-middle part of accelSteps is equal to 0
brne _accelAlgorithmStart	;If not, we can safely execute acceleration algorithm
cpi r19,0					;Check if upper-middle part of accelSteps is equal to 0
brne _accelAlgorithmStart	;If not, we can safely execute acceleration algorithm

ldi r19,_CRUISE 				
std z+STATE,R19 			;If accelSteps is equal to 0, we are done accelerating, and the motor should change to cruise state

rjmp _cruiseAlgorithm		;Perform algorithm for cruise state

_accelAlgorithmStart:
;A maximum of 96 clock cycles has been used to reach this point !
subi r16,1					;Decrement lower 8 bits of accelSteps			
sbci r17,0					;If carry flag was set, subtract 1 from middle-lower 8 bits of accelSteps
sbci r18,0					;If carry flag was set, subtract 1 from middle-upper 8 bits of accelSteps
sbci r19,0					;If carry flag was set, subtract 1 from middle-upper 8 bits of accelSteps

std z+ACCELSTEPS,r16		;Store lower part of accelSteps variable back into ram
std z+ACCELSTEPS+1,r17		;store middle-lower part of accelSteps variable back into ram
std z+ACCELSTEPS+2,r18		;Store middle-upper part of accelSteps variable back into ram
std z+ACCELSTEPS+3,r19		;Store upper part of accelSteps variable back into ram

rjmp _algorithm

_decelAlgorithm:
ldd r16,z+DECELSTEPS		;Load lower part of decelSteps variable into r16
ldd r17,z+DECELSTEPS+1		;Load middle-lower part of decelSteps variable into r17
ldd r18,z+DECELSTEPS+2		;Load middle-upper part of decelSteps variable into r18
ldd r19,z+DECELSTEPS+3		;Load upper part of decelSteps variable into r19

;Make sure we don't execute the deceleration algorithm if decelSteps variable is equal to 0.
;if this happens, the code will decrement the variable, resulting in the variable overflowing and attaining a value of 2³² - 1, which is obviously wrong, and should be avoided
cpi r16,0					;Check if lower part of decelSteps is equal to 0
brne _decelAlgorithmStart	;If not, we can safely execute deceleration algorithm
cpi r17,0					;Check if lower-middle part of decelSteps is equal to 0
brne _decelAlgorithmStart	;If not, we can safely execute deceleration algorithm
cpi r18,0					;Check if upper-middle part of decelSteps is equal to 0
brne _decelAlgorithmStart	;If not, we can safely execute deceleration algorithm
cpi r19,0					;Check if upper part of decelSteps is equal to 0
brne _decelAlgorithmStart	;If not, we can safely execute deceleration algorithm

ldi r19,_STOP
std z+STATE,R19 			;If decelsteps is equal to 0, we are done decelerating, and the motor should be stopped
rjmp _stopMotor				;Stop motor	

_decelAlgorithmStart:
subi r16,1					;Decrement lower 8 bits of decelSteps			
sbci r17,0					;If carry flag was set, subtract 1 from middle-lower 8 bits of decelSteps
sbci r18,0					;If carry flag was set, subtract 1 from middle-upper 8 bits of decelSteps
sbci r19,0					;If carry flag was set, subtract 1 from upper 8 bits of decelSteps

std z+DECELSTEPS,r16		;Store lower part of decelSteps variable back into ram
std z+DECELSTEPS+1,r17		;store middle-lower part of decelSteps variable back into ram
std z+DECELSTEPS+2,r18		;Store middle-upper part of decelSteps variable back into ram
std z+DECELSTEPS+3,r19 		;Store upper part of decelSteps variable back into ram

;A maximum of 110 clock cycles (the next "rjmp" instruction IS included in these clock cycles) has been used to reach this point !
rjmp _algorithm

_initDecelAlgorithm:
ldd r16,z+INITIALDECELSTEPS		;Load lower part of initialDecelSteps variable into r16
ldd r17,z+INITIALDECELSTEPS+1	;Load middle-lower part of initialDecelSteps variable into r17
ldd r18,z+INITIALDECELSTEPS+2	;Load middle-upper part of initialDecelSteps variable into r18
ldd r19,z+INITIALDECELSTEPS+3	;Load upper part of initialDecelSteps variable into r19

;Make sure we dont run initDecel algorithm if initDecelSteps variable is equal to 0
cpi r16,0
brne _initDecelAlgorithmStart
cpi r17,0
brne _initDecelAlgorithmStart
cpi r18,0
brne _initDecelAlgorithmStart
cpi r19,0
brne _initDecelAlgorithmStart

;If we are done with initialDeceleration part, we should start accelerating in the desired direction 
ldd r16,z+DIRECTION			;Load direction variable into r16
sbrs r16,0					;If bit zero is set, we should set DIR pin, and therefore skip next line
cbi 0x0B,7					;If bit zero is cleared, we should clear DIR pin
sbrc r16,0					;if bit zero is cleared, we should clear DIR pin, and therefore skip next line
sbi 0x0B,7					;If bit zero is set, we should set DIR pin

ldi r19,_ACCEL
std z+STATE,r19 			;If initDecelSteps is equal to 0, we are done decelerating, and the state should be changed to acceleration
rjmp _accelAlgorithm

_initDecelAlgorithmStart:
subi r16,1					;Decrement lower 8 bits of initialDecelSteps			
sbci r17,0					;If carry flag was set, subtract 1 from middle-lower 8 bits of initialDecelSteps
sbci r18,0					;If carry flag was set, subtract 1 from middle-upper 8 bits of initialDecelSteps
sbci r19,0					;If carry flag was set, subtract 1 from upper 8 bits of initialDecelSteps

std z+INITIALDECELSTEPS,r16					;Store lower part of initialDecelSteps variable back into ram
std z+INITIALDECELSTEPS+1,r17				;store middle-lower part of initialDecelSteps variable back into ram
std z+INITIALDECELSTEPS+2,r18				;Store middle-upper part of initialDecelSteps variable back into ram
std z+INITIALDECELSTEPS+3,r19 				;Store upper part of initialDecelSteps variable back into ram


_algorithm:
; Algorithm: Delay = p = (P+P^3*multiplier)

;exactDelay variable is stored as a double precision floating point value. The format of this variable is NOT IEEE-754 format, but instead in a format I have defined myself.
;The format is as follows:

;Bits 63:57 - Not used
;Bits 56	- Sign bit (1 for negative, 0 for positive)
;bits 55-48 - Exponent (A value of 0x7F corresponds to 2^0)
;Bits 47:0	- Mantissa (The mantissa bits only contains the fraction part, just as in the IEEE format)

;The reason i have chosen to use this format instead of the IEEE foramt, is that it allows me to save a few clock cycles in the following computations,
;by not having to shift out the sign bit.
;Although i have 49 bits of precision in this format, i only need the full precision in the final step of the algorithm (the addition).
;This is because of the multiplier variable containing a very small value, and at some point, the exactDelay variable gets so small that I will have
;to subtract/add a value which is so small that (in single precision float), will be equal to zero. Therefore, in order to be able subtract/add all possible 
;values in the end of this algorithm, the full precision is needed in this step.

;calculating p^3 (exponent in r16, mantissa in r18):

;build mantissa - I only use the upper 8 bits of the mantissa from these floats, since it doesn't make a noticeable difference compared to the performance gain
;Vertical dashed line indicates where the decimal point is located in the variables
;	 	 _____ | _____
;		| 	  | |     |
;		|  1  ||| r15 |				<--- P variable
;		|_____| |_____|
;		   	   |
;	 	 _____   _____
;		| 	  |||     |
;		|  1  | | r15 |				<--- P variable
;		|_____|||_____|
;-----------------------------------------------
;	 	 	   | _____________
;				| 	  	      |
;			   ||  	r15*r15	  |
;				|_____________|
;	 		   | _____ 
;				| 	  |
;			   || r15 |
;				|_____|
;			   | _____ 
;				| 	  |
;			   || r15 |
;				|_____|
;	 	 _____ |
;		| 	  |
;		|  1  ||
;		|_____|
;			   |
;

ldd r15,z+EXACTDELAY+5	;Load mantissa bits 40:47
ldd r16,z+EXACTDELAY+6	;Load exponent bits

mul r15,r15				;Multiply
ldi r17,0x01 			;Load the "hidden" 1 into r17
clr r0 					;We need a register with value zero
adc r1,r15 				;We should add the mantissa register
adc r17,r0 				;increment r17 if overflow occured 
adc r1,r15 				;We should add the mantissa register
adc r17,r0 				;increment r17 if overflow occured

;Vertical dashed line indicates where the decimal point is located in the variables
;	 	 _____ | _____
;		| 	  | |     |
;		| r17 ||| r18 |				<--- P^2 variable
;		|_____| |_____|
;		   	   |
;	 	 _____   _____
;		| 	  |||     |
;		|  1  | | r15 |				<--- P variable
;		|_____|||_____|
;-----------------------------------------------
;	 	 	   | _____________
;				| 	  	      |
;			   ||  	r15*r18	  |
;				|_____________|
;	 	 ______|______
;		| 	          |
;		|  r17 |*r15  |
;		|_____________|
;			   | _____ 
;				| 	  |
;			   || r18 |
;				|_____|
;	 	 _____ |
;		| 	  |
;		| r17 ||
;		|_____|
;			   |

mov r18,r1
mul r15,r18
add r18,r1
clr r1
mov r19,r17
adc r17,r1
mul r15,r19
add r18,r0
adc r17,r1

;Building exponent
mov r23,r16
add r23,r16			;Add the exponent once from the first multiplication
subi r23,0x7F		;Subtract offset of 0x7F to align new exponent
add r16,r23			;Add the exponent once more from the second multiplication
subi r16,0x7F		;Subtract offset of 0x7F to align new exponent

;r17 contains the "hidden 1" of the floating point mantissa. if this register is greater than 1, we should shift the 
;computed mantissa right one bit and increment the exponent in order to normalize the mantissa

cpi r17,0x02		;It is not possible for r17 to contain a number higher than 6. 
brlo _pCubedNormalized
	lsr r17
	ror r18
	inc r16

cpi r17,0x02		;At this point it is not possible for r17 to contain a number higher than 3. 
brlo _pCubedNormalized
	lsr r17
	ror r18
	inc r16

_pCubedNormalized:


;Calculating P^3*m. (result in r23:r19+r17)

;in this calculation I am required to use the full precision of the mantissa of the multiplier in order for the algorithm to work.
;In the drawing of the calculation below, multiple boxes contains "r15". This should NOT be understood as if the variable contains the same
;number in all of these blocks. Instead it indicates that the respective block is loaded from ram when it is needed, rather than loading 
;all the blocks before performing the multiplication. This is done to lower the amount of general purpose registers needed, which in turn
;increases the performance of the overall interrupt code. 
;
;Again, i only need the 8 upper bits of the mantissa of the P^3 variable.
;
;Vertical dashed line indicates where the decimal point is located in the variables
;	 	 _____ | _____  _____  _____  _____  _____  _____
;		| 	  | |     ||     ||     ||     ||     ||     |
;		|  1  ||| r15 || r15 || r15 || r15 || r15 || r15 |				<--- Multiplier variable 
;		|_____| |_____||_____||_____||_____||_____||_____|
;		   	   |              
;	 	 _____   _____        
;		| 	  |||     |       
;		|  1  | | r18 |       											<--- P^3 variable
;		|_____|||_____|       
;------------------------------------------------------------------
;	 	 	   | 			  						____________
;							  					   |     	    |
;			   |			  					   |  r15*r18   |
;							  					   |____________|
;			   |                                    _____
;												   |     |
;			   |								   | r15 |
;												   |_____|
;	 	 	   | 			  			     ____________
;							  				|        	 |
;			   |			  				|  r15*r18   |
;							  				|____________|
;			   |                             _____
;										    |     |
;			   |						    | r15 |
;										    |_____|
;			   | 			  		  ____________
;							  		 |     	      |
;			   |			  		 |  r15*r18   |
;							  		 |____________|
;			   |                      _____
;									 |     |
;			   |					 | r15 |
;							    	 |_____|
;			   | 			   ____________
;							  |     	   |
;			   |			  |  r15*r18   |
;							  |____________|
;			   |               _____
;							  |     |
;			   |			  | r15 |
;					     	  |_____|
;			   | 	    ____________
;				       |      	    |
;			   |	   |  r15*r18   |
;					   |____________|
;			   |        _____
;				 	   |     |
;			   |	   | r15 |
;					   |_____|
;			   | ____________
;				|     	     |
;			   ||  r15*r18   |
;				|____________|
;	 	       | _____
;		        |     |
;		       || r18 |
;		        |_____|
;			   | _____ 
;				| 	  |
;			   || r15 |
;				|_____|
;	 	 _____ |
;		| 	  |
;		|  1  ||
;		|_____|
;			   |
;
;Build mantissa
ldd r15,z+MULTIPLIER
mul r18,r15
add r1,r15
clr r19
adc r19,r19
mov r17,r1
ldd r15,z+MULTIPLIER+1
mul r18,r15
add r17,r0
adc r1,r15
clr r20
adc r20,r20
add r19,r1
clr r21
adc r20,r21
ldd r15,z+MULTIPLIER+2
mul r18,r15
add r19,r0
adc r1,r15
adc r21,r21
add r20,r1
clr r22
adc r21,r22
ldd r15,z+MULTIPLIER+3
mul r18,r15
add r20,r0
adc r1,r15
adc r22,r22
add r21,r1
clr r23
adc r22,r23
ldd r15,z+MULTIPLIER+4
mul r18,r15
add r21,r0
adc r1,r15
adc r23,r23
add r21,r1
clr r15
adc r23,r15
ldd r15,z+MULTIPLIER+5
mul r18,r15
add r23,r18
clr r18
adc r18,r18
add r22,r0
adc r1,r15
clr r15
adc r18,r15
add r23,r1
adc r18,r15
inc r15
add r18,r15

;Build exponent
ldd r15,z+MULTIPLIER+6
add r16,r15				;Add exponents
subi r16,0x7F			;Subtract offset of 0x7F to align exponent

;Normalize result
cpi r18,0x02
brlo _pCubeMulNormalized
	lsr r18
	ror r23
	ror r22
	ror r21
	ror r20
	ror r19
	ror r17
	inc r16
_pCubeMulNormalized:

;(Acceleration)A maximum of 227 clock cycles has been used to reach this point !
;
;Calculating p + p^3*m

ldd r18,z+EXACTDELAY+6
sub r18,r16			;Subtract exponents of P and m*P^3, in order to find out how much the lower number mantissa should be shifted

brcs _pSmaller		;If carry was set, P is the smaller number

rjmp _pLarger		;If not, P is equal to or greater than m*P^3

;A closer look into the code under the labels "_pSmaller" and "_pLarger", will reveal that the to code blocks are essentially performing the same task
;and a single code block could therefore have been used instead of "wasting" code space like this. There is however, a good reason to NOT use one
;code block for this. This reason is that, the way it is implemented now, lowers the amount of general purpose registers needed to perform the addition/subtraction
;and hereby increases the performance of the code, by not having to push/pop as many registers onto the stack, upon entering/exiting the interrupt routine.

;Calculate the difference in exponents between the two values in order to find out how much the lower number should be shifted to the 
;right to get equal exponents
_pSmaller:
mov r18,r16						;Move the larger exponent (m*P^3) into r18										
ldd r16,z+EXACTDELAY+6			;Load the exponent of exactDelayVariable once again
sub r18,r16						;Subtract exponents of P and m*P^3, in order to find out how much the lower number mantissa should be shifted

cpi r18,0x00					;If the exponents are different					
brne _pShiftMantissa2			;We should start shifting
clr r16							;If not, we skip this step. r16 should be cleared.
rjmp _pExpSame2

_pShiftMantissa2:
cpi r18,0x30					
brlo _pShiftFourtyEight2		;If we should shift less than 48 times, skip the following instructions
;Shift 48 times.
clr r0
inc r0
clr r1
clr r15

ldi r16,0x01
rjmp _pExpSame2

_pShiftFourtyEight2:
cpi r18,0x29
brlo _pShiftFourtyOne2
ldd r0, z+EXACTDELAY+5
sec
ror r0
clr r1
clr r15
ldi r16,0x01
subi r18,0x29
rjmp _pShiftFour2

_pShiftFourtyOne2:
cpi r18,0x21
brlo _pShiftThirtyThree2
ldd r0,z+EXACTDELAY+4
ldd r1,z+EXACTDELAY+5
sec
ror r1
ror r0
clr r15
ldi r16,0x01
subi r18,0x21
rjmp _pShiftFour2

_pShiftThirtyThree2:
cpi r18,0x19
brlo _pShiftTwentyFive2
ldd r0,z+EXACTDELAY+3
ldd r1,z+EXACTDELAY+4
ldd r15,z+EXACTDELAY+5
ldi r16,0x01
sec
ror r15
ror r1
ror r0
subi r18,0x19
rjmp _pShiftFour2

_pShiftTwentyFive2:
cpi r18,0x11
brlo _pShiftSeventeen2
ldd r0,z+EXACTDELAY+3
ldd r1,z+EXACTDELAY+4
ldd r15,z+EXACTDELAY+5
ldi r16,0x02
sec
ror r15
ror r1
ror r0
subi r18,0x11
rjmp _pShiftFour2

_pShiftSeventeen2:
cpi r18,0x09
brlo _pShiftNine2
ldd r0,z+EXACTDELAY+3
ldd r1,z+EXACTDELAY+4
ldd r15,z+EXACTDELAY+5
ldi r16,0x04
sec
ror r15
ror r1
ror r0
subi r18,0x09
rjmp _pShiftFour2

_pShiftNine2:
ldd r0,z+EXACTDELAY+3
ldd r1,z+EXACTDELAY+4
ldd r15,z+EXACTDELAY+5
sec
ror r15
ror r1
ror r0
dec r18
ldi r16,0x08

cpi r18,0x04
brlo _pShiftFour2
lsr r0
lsr r0
lsr r0
lsr r0
swap r1
mov r16,r1
andi r16,0xF0
or r0,r16
mov r16,r1
andi r16,0x0F
mov r1,r16
swap r15
mov r16,r15
andi r16,0xF0
or r1,r16
mov r16,r15
andi r16,0x0F
mov r15,r16
ldi r16,0x08
subi r18,0x04

_pShiftFour2:
cpi r18,0x00
breq _pExpSame2			;if the exponents are equal, we dont need to shift
	lsr r15
	ror r1
	ror r0
	dec r18
rjmp _pShiftFour2

_pExpSame2:
ldd r18,z+STATE				;Load the state variable
sbrc r18,_ACCELBIT 			;Are we in the acceleration phase?
rjmp _pSubtractMantissa2 	;If not, we should add P^3*m to the exactDelay variable

;If we are currently in the acceleration phase, we should subtract P^3*m from the exactDelay variable

cpi r16,0x00
breq _pAddMantissa20
sbrc r16,0
rjmp _pAddMantissa21
sbrc r16,1
rjmp _pAddMantissa22
sbrc r16,2
rjmp _pAddMantissa23

add r21,r0
adc r22,r1
adc r23,r15
clr r18
adc r18,r18
inc r18 
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pAddMantissa20:
ldd r0,z+EXACTDELAY+3
ldd r1,z+EXACTDELAY+4
ldd r15,z+EXACTDELAY+5

add r21,r0
adc r22,r1
adc r23,r15
ldi r18,0x01
adc r18,r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pAddMantissa21:
add r17,r0
adc r19,r1
adc r20,r15
clr r18
adc r21,r18
adc r22,r18
adc r23,r18
adc r18,r18
inc r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pAddMantissa22:
add r19,r0
adc r20,r1
adc r21,r15
clr r18
adc r22,r18
adc r23,r18
adc r18,r18
inc r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pAddMantissa23:
add r20,r0
adc r21,r1
adc r22,r15
clr r18
adc r23,r18
adc r18,r18
inc r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa2:
cpi r16,0x00
breq _pSubtractMantissa20
sbrc r16,0
rjmp _pSubtractMantissa21
sbrc r16,1
rjmp _pSubtractMantissa22
sbrc r16,2
rjmp _pSubtractMantissa23

sub r21,r0
sbc r22,r1
sbc r23,r15
ldi r18,0x01
sbci r18,0x00
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa20:
ldd r0,z+EXACTDELAY+3
ldd r1,z+EXACTDELAY+4
ldd r15,z+EXACTDELAY+5

sub r21,r0
sbc r22,r1
sbc r23,r15
clr r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa21:
sub r17,r0
sbc r19,r1
sbc r20,r15
ldi r18,0x01
sbci r21,0x00
sbci r22,0x00
sbci r23,0x00
sbci r18,0x00
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa22:
sub r19,r0
sbc r20,r1
sbc r21,r15
ldi r18,0x01
sbci r22,0x00
sbci r23,0x00
sbci r18,0x00
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa23:
sub r20,r0
sbc r21,r1
sbc r22,r15
ldi r18,0x01
sbci r23,0x00
sbci r18,0x00
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pLarger:
mov r16,r18

cpi r16,0x00		;Is exponents of the numbers equal?
brne _pShiftMantissa1

ldi r18,0x00
rjmp _pExpSame1
_pShiftMantissa1:
cpi r16,0x30
brlo _pShiftFourtyEight1

clr r17
inc r17
clr r19
clr r20
ldi r18,0x01
rjmp _pExpSame1

_pShiftFourtyEight1:
cpi r16,0x29
brlo _pShiftFourtyOne1
sec
ror r23
clr r19
clr r20
mov r17,r23

ldi r18,0x01
subi r16,0x29
rjmp _pShiftFour1

_pShiftFourtyOne1:
cpi r16,0x21
brlo _pShiftThirtyThree1

sec
ror r23
ror r22
clr r20
mov r17,r22
mov r19,r23
ldi r18,0x01
subi r16,0x21
rjmp _pShiftFour1

_pShiftThirtyThree1:
cpi r16,0x19
brlo _pShiftTwentyFive1
sec
ror r23
ror r22
ror r21
mov r17,r21
mov r19,r22
mov r20,r23
ldi r18,0x01
subi r16,0x19
rjmp _pShiftFour1

_pShiftTwentyFive1:
cpi r16,0x11
brlo _pShiftSeventeen1

sec
ror r23
ror r22
ror r21
mov r17,r21
mov r19,r22
mov r20,r23
ldi r18,0x02
subi r16,0x11
rjmp _pShiftFour1

_pShiftSeventeen1:
cpi r16,0x09
brlo _pShiftNine1
sec
ror r23
ror r22
ror r21
mov r17,r21
mov r19,r22
mov r20,r23
ldi r18,0x04
subi r16,0x09
rjmp _pShiftFour1

_pShiftNine1:
sec
ror r23
ror r22
ror r21
dec r16
mov r20,r23
mov r19,r22
mov r17,r21
ldi r18,0x08

cpi r16,0x04
brlo _pShiftFour1
lsr r17
lsr r17
lsr r17
lsr r17
swap r19
mov r21,r19
andi r21,0xF0
or r17,r21
andi r19,0x0F
swap r20
mov r21,r20
andi r21,0xF0
or r19,r21
andi r20,0x0F
ldi r18,0x08
subi r16,0x04

_pShiftFour1:
cpi r16,0x00
breq _pExpSame1			;if the exponents are equal, we dont need to shift
	lsr r20
	ror r19
	ror r17
	dec r16
rjmp _pShiftFour1

_pExpSame1:
ldd r16,z+STATE				;Load the state variable
sbrc r16,_ACCELBIT 			;Are we in the deceleration phase?
rjmp _pSubtractMantissa1 	;If we are currently in the deceleration phase, we should add P^3*m to the exactDelay variable

cpi r18,0x00
breq _pAddMantissa10
sbrc r18,0
rjmp _pAddMantissa11
sbrc r18,1
rjmp _pAddMantissa12
sbrc r18,2
rjmp _pAddMantissa13

ldd r21,z+EXACTDELAY+3
add r21,r17
ldd r22,z+EXACTDELAY+4
adc r22,r19
ldd r23,z+EXACTDELAY+5
adc r23,r20
ldd r17,z+EXACTDELAY
ldd r19,z+EXACTDELAY+1
ldd r20,z+EXACTDELAY+2
clr r18
adc r18,r18
inc r18 
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pAddMantissa10:
ldd r0,z+EXACTDELAY
add r17,r0
ldd r0,z+EXACTDELAY+1
adc r19,r0
ldd r0,z+EXACTDELAY+2
adc r20,r0
ldd r0,z+EXACTDELAY+3
add r21,r0
ldd r0,z+EXACTDELAY+4
adc r22,r0
ldd r0,z+EXACTDELAY+5
adc r23,r0
ldi r18,0x01
adc r18,r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pAddMantissa11:

ldd r23,z+EXACTDELAY+5
ldd r22,z+EXACTDELAY+4
ldd r21,z+EXACTDELAY+3
ldd r0,z+EXACTDELAY
add r17,r0
ldd r0,z+EXACTDELAY+1
adc r19,r0 
ldd r0,z+EXACTDELAY+2
adc r20,r0
clr r18
adc r21,r18
adc r22,r18
adc r23,r18
adc r18,r18
inc r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pAddMantissa12:

ldd r23,z+EXACTDELAY+5
ldd r22,z+EXACTDELAY+4
ldd r21,z+EXACTDELAY+3
mov r0,r19
mov r1,r20
ldd r19,z+EXACTDELAY+1
add r19,r17
ldd r20,z+EXACTDELAY+2
adc r20,r0
adc r21,r1
ldd r17,z+EXACTDELAY
clr r18
adc r22,r18
adc r23,r18
adc r18,r18
inc r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pAddMantissa13:
ldd r23,z+EXACTDELAY+5
ldd r22,z+EXACTDELAY+4
ldd r21,z+EXACTDELAY+3
mov r0,r20
ldd r20,z+EXACTDELAY+2
add r20,r17
adc r21,r19
adc r22,r0
ldd r17,z+EXACTDELAY
ldd r19,z+EXACTDELAY+1
clr r18
adc r23,r18
adc r18,r18
inc r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa1:

cpi r18,0x00
breq _pSubtractMantissa10
sbrc r18,0
rjmp _pSubtractMantissa11
sbrc r18,1
rjmp _pSubtractMantissa12
sbrc r18,2
rjmp _pSubtractMantissa13

ldd r21,z+EXACTDELAY+3
sub r21,r17
ldd r22,z+EXACTDELAY+4
sbc r22,r19
ldd r23,z+EXACTDELAY+5
sbc r23,r20
ldd r17,z+EXACTDELAY
ldd r19,z+EXACTDELAY+1
ldd r20,z+EXACTDELAY+2
ldi r18,0x01
sbci r18,0x00
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa10:
mov r0,r17
ldd r17,z+EXACTDELAY
sub r17,r0
mov r0,r19
ldd r19,z+EXACTDELAY+1
sbc r19,r0
mov r0,r20
ldd r20,z+EXACTDELAY+2
sbc r20,r0
mov r0,r21
ldd r21,z+EXACTDELAY+3
sbc r21,r0
mov r0,r22
ldd r22,z+EXACTDELAY+4
sbc r22,r0
mov r0,r23
ldd r23,z+EXACTDELAY+5
sbc r23,r0
clr r18
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa11:
mov r0,r17
ldd r17,z+EXACTDELAY
sub r17,r0
mov r0,r19
ldd r19,z+EXACTDELAY+1
sbc r19,r0
mov r0,r20
ldd r20,z+EXACTDELAY+2
sbc r20,r0
ldd r21,z+EXACTDELAY+3
ldd r22,z+EXACTDELAY+4
ldd r23,z+EXACTDELAY+5
sbci r21,0x00
sbci r22,0x00
sbci r23,0x00
ldi r18,0x01
sbci r18,0x00
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa12:
mov r0,r17
mov r1,r19
mov r15,r20
ldd r17,z+EXACTDELAY
ldd r19,z+EXACTDELAY+1
ldd r20,z+EXACTDELAY+2
ldd r21,z+EXACTDELAY+3
ldd r22,z+EXACTDELAY+4
ldd r23,z+EXACTDELAY+5
sub r19,r0
sbc r20,r1
sbc r21,r15
sbci r22,0x00
sbci r23,0x00
ldi r18,0x01
sbci r18,0x00
rjmp _pMantissaCalculated	;we are done calulating the mantissa

_pSubtractMantissa13:
mov r0,r17
mov r1,r19
mov r15,r20
ldd r17,z+EXACTDELAY
ldd r19,z+EXACTDELAY+1
ldd r20,z+EXACTDELAY+2
ldd r21,z+EXACTDELAY+3
ldd r22,z+EXACTDELAY+4
ldd r23,z+EXACTDELAY+5
sub r20,r0
sbc r21,r1
sbc r22,r15
sbci r23,0x00
ldi r18,0x01
sbci r18,0x00
rjmp _pMantissaCalculated	;we are done calulating the mantissa

;Now we need to renormalize the mantissa, so the "hidden 1" gets into the correct place (r22 should contain this, and should therefore equal exactly 1)
;We do this normalization by shifting the radix point left or right till it fits, and incrementing/decrementing the exponent as needed
_pMantissaCalculated:
ldd r16,z+EXACTDELAY+6
cpi r18,0x01
brlo _pShiftRadixLeft
cpi r18,0x02
brsh _pShiftRadixRight
rjmp _pRadixDone

_pShiftRadixLeft:
	lsl r17
	rol r19
	rol r20
	rol r21
	rol r22
	rol r23
	rol r18
	dec r16
	cpi r18,0x01
	brne _pShiftRadixLeft
	rjmp _pRadixDone

_pShiftRadixRight:
	lsr r18
	ror r23
	ror r22
	ror r21
	ror r20
	ror r19
	ror r17
	inc r16
	cpi r18,0x02
	brsh _pShiftRadixRight

_pRadixDone:
;store the exactDelay variable back into ram
	std z+EXACTDELAY,r17
	std z+EXACTDELAY+1,r19
	std z+EXACTDELAY+2,r20
	std z+EXACTDELAY+3,r21
	std z+EXACTDELAY+4,r22
	std z+EXACTDELAY+5,r23
	std z+EXACTDELAY+6,r16

;subtract 0.5 to exactDelay variable in order to round correctly!
;Usually one would add 0.5 instead of subtracting, however in our case we need to subtract 1 from the calculated delay
;meaning that the addition of 0.5 turns into a subtraction of 0.5

subi r16,0x7F
sec
ror r23
ror r22
ror r21
inc r16

cpi r16,0x11
brsh _storeDelayLimitInRam

_storeDelay3:
cpi r16,0x04
brsh _storeDelayCmp
mov r18,r23
clr r19
clr r20
rjmp _storeDelayFinishShift

_storeDelayCmp:
cpi r16,0x10
brlo _storeDelayCmp12
mov r20,r23
mov r19,r22
mov r18,r21
rjmp _storeDelayInRam

_storeDelayCmp12:
cpi r16,0x0C
brlo _storeDelayCmp8
swap r23
mov r20,r23
andi r23,0xF0
andi r20,0x0F
swap r22
mov r19,r22
andi r22,0xF0
andi r19,0x0F
or r19,r23
swap r21
mov r18,r21
andi r18,0x0F
or r18,r22
subi r16,0x0C
rjmp _storeDelayFinishShift

_storeDelayCmp8:
cpi r16,0x08
brlo _storeDelayCmp4
mov r19,r23
mov r18,r22
clr r20
subi r16,0x08
rjmp _storeDelayFinishShift

_storeDelayCmp4:
cpi r16,0x04
brlo _storeDelayFinishShift
swap r23
mov r19,r23
andi r19,0x0F
andi r23,0xF0
mov r18,r23
clr r20
subi r16,0x04
_storeDelayFinishShift:

cpi r16,0x00
breq _storeDelayInRam
lsl r18
rol r19
rol r20
dec r16
rjmp _storeDelayFinishShift

_storeDelayLimitInRam:
ldi r20,0xFF
ldi r19,0xFF
ldi r18,0xFF

;Store integer part of rounded exactDelay variable in delay variable
_storeDelayInRam:

subi r18,0x80
sbci r19,0
sbci r20,0

std z+DELAY,r19
std z+DELAY+1,r20
rjmp _end	

_cruiseAlgorithm:
ldd r16,z+CONTINOUS				;Load continous variable into r16
cpi r16,0					;If continous variable is equal to 0, we should decrement cruisesteps (skip next line)
brne _continous 			;If motor is running in continous mode, we should not decrement cruiseSteps

	_nonContinous:
		ldd r16,z+CRUISESTEPS		;Load lower part of cruiseSteps variable into r16
		ldd r17,z+CRUISESTEPS+1		;Load lower-middle part of cruiseSteps variable into r17
		ldd r18,z+CRUISESTEPS+2		;Load upper-middle part of cruiseSteps variable into r18
		ldd r19,z+CRUISESTEPS+3		

		cpi r16,0
		brne _cruiseAlgorithmStart
		cpi r17,0
		brne _cruiseAlgorithmStart
		cpi r18,0
		brne _cruiseAlgorithmStart
		cpi r19,0
		brne _cruiseAlgorithmStart

		ldi r20,_DECEL
		
		;Store the exactDelayDecel variable into the exactDelay variable in order to decelerate correctly
		std z+STATE,r20
		ldd r16,z+EXACTDELAY+3
		std z+EXACTDELAY+3,r16
		ldd r16,z+EXACTDELAY+2
		std z+EXACTDELAY+2,r16
		ldd r16,z+EXACTDELAY+1
		std z+EXACTDELAY+1,r16
		ldd r16,z+EXACTDELAY
		std z+EXACTDELAY,r16
		
		rjmp _decelAlgorithm

		_cruiseAlgorithmStart:
		subi r16,1			;Decrement lower 8 bits of cruiseSteps			
		sbci r17,0			;If carry flag was set, subtract 1 from lower-middle 8 bits of cruiseSteps
		sbci r18,0			;If carry flag was set, subtract 1 from upper-middle 8 bits of cruiseSteps
		sbci r19,0
		std z+CRUISESTEPS,r16		;Store lower part of cruiseSteps variable back into ram
		std z+CRUISESTEPS+1,r17		;store lower-middle part of cruiseSteps variable back into ram
		std z+CRUISESTEPS+2,r18		;store upper-middle part of cruiseSteps variable back into ram
		std z+CRUISESTEPS+3,r19

	_continous:
		
		ldd r16,z+CRUISEDELAY			;Load lower part of cruiseDelay variable
		ldd r17,z+CRUISEDELAY+1			;Load upper part of cruiseDelay variable
		std z+DELAY,r16					;Store lower part of cruiseDelay into lower part of delay variable
		std z+DELAY+1,r17				;Store upper part of cruiseDelay into upper part of delay variable 
		rjmp _end						;We are done

_end:
;We are done, and should restore general purpose registers from the stack, in order for the software to continue its operation properly!

pop r23
pop r22
pop r21
pop r20
pop r19
pop r18
pop r17
pop r15
pop r1
pop r0
pop r31
pop r30
pop r16
out _SREG,r16
pop r16
cbi 0x0B,4				;Pull step pin low, so we are ready to issue next step

;Return from interrupt routine
reti