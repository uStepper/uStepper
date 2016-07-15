/********************************************************************************************
*    	File: 		uStepper.S 																*
*		Version:    0.3.0                                             						*
*      	date: 		May 27th, 2016                                    						*
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
*																							*
*********************************************************************************************
*								TO DO:														*
*																							*
*		- Remove the quick and dirty fix in the algorithm (line 614-625)		 			*
*																							*
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
/**	\file uStepper.S
*	\brief Stepper acceleration profile
*	
*	This file contains the assembly implementations of the stepper acceleration algorithm.
*	The algorithm is implemented in assembly in order to get the best possible performance.
*	
*	\author Thomas Hørring Olsen (thomas@ustepper.com)
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

;Push all registers used in this assembly code to stack, in order to not fuck up flow of code when leaving the interrupt routine (29 clk cycles total!)

push r0
push r1
in r0,_SREG
push r0
push r15
push r16
push r17
push r18
push r19
push r20
push r21
push r22
push r23
push r30
push r31

lds r30,pointer				;Load Low nibble of object address into Z-register
lds r31,pointer+1			;Load High nibble of object address into Z-register
cbi 0x0B,4					;Pull step pin low, so we are ready to issue next step

ldd r16,z+20				;Load state variable into r16
sbrc r16,_STOPBIT			;Check if state is set to STOP
rjmp _stopMotor				;If state is set to STOP, kill timer interrupt.
ldd r17,z+10				;Load lower part of delay variable
ldd r18,z+11				;Load upper part of delay variable
cpi r17,0					;Check if upper part of delay variable is equal to 0
brne _notRdy				;If upper part of delay variable not equal to 0, we should decrement delay variable and leave routine
cpi r18,0					;Check if lower part of delay variable is equal to 0
brne _notRdy				;If lower part of delay variable not equal to 0, we should decrement delay variable and leave routine

sbi 0x0B,4					;Pull Step pin high, to issue a step
rjmp _rdy

_notRdy:					;Not ready to issue step
subi r17,1					;Decrement delay variable
sbci r18,0					;if lower part of delay variable was 0 (carry flag was set during last subtraction), subtract 1 from upper part
std z+10,r17				;store lower part of delay variable back into ram
std z+11,r18				;Store upper part of delay variable back into ram
rjmp _end					;We are done 

;Load currentSteps variable, add 1 to this and store it back into ram
;This is done so we can keep track of how many steps the motor have performed,
;and add this to the total nu,ber of steps performed since reset, at the end
;of the movement.
_rdy:
ldd r19,z+37				
ldd r20,z+38
ldd r21,z+39
ldd r22,z+40
ldi r18,1
ldi r23,0
add r19,r18
adc r20,r23
adc r21,r23
adc r22,r23
std z+37,r19
std z+38,r20
std z+39,r21
std z+40,r22

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
ldd r15,z+48				;Load the four lower bytes of stepsSinceReset variable
ldd r16,z+49
ldd r17,z+50
ldd r18,z+51

ldd r19,z+37				;Load the currentStep variable
ldd r20,z+38
ldd r21,z+39
ldd r22,z+40

ldd r23,z+47				;Load the direction variable
cpi r23,_CW 				;If the direction is set to clockwise, we should add the currentStep variable to the stepsSinceReset variable
brne _subtractSteps			;If the direction is set to counterclockwise, we should subtract the currentStep variable to the stepsSinceReset variable

;Add currentSteps variable to stepsSinceReset variable
ldi r23,0
add r15,r19
adc r16,r20
adc r17,r21
adc r18,r22

;Load upper four bytes of stepsSinceReset variable
ldd r19,z+52
ldd r20,z+53
ldd r21,z+54
ldd r22,z+55

;Add any overflows that might have happened
adc r19,r23
adc r20,r23
adc r21,r23
adc r22,r23
rjmp _storeSteps			;Store back into ram

_subtractSteps:

;Subtract currentSteps variable from stepsSinceReset variable
ldi r23,0
sub r15,r19
sbc r16,r20
sbc r17,r21
sbc r18,r22

;Load upper four bytes of stepsSinceReset variable
ldd r19,z+52
ldd r20,z+53
ldd r21,z+54
ldd r22,z+55

;Subtract any overflows that might have happened
sbc r19,r23
sbc r20,r23
sbc r21,r23
sbc r22,r23

_storeSteps:
;Store stepsSinceReset variable back into ram
std z+48,r15
std z+49,r16
std z+50,r17
std z+51,r18
std z+52,r19
std z+53,r20
std z+54,r21
std z+55,r22

;cler currentSteps variable and store this back into ram
clr r15
std z+37,r15
std z+38,r15
std z+39,r15
std z+40,r15

lds r16,0x70				;Load contents TIMSK2 register into r16
andi r16,0xFD				;Clear TOIE2 in TIMSK2 (kill timer)
sts 0x70,r16				;Store r16 back into TIMSK2 register
ldd r16,z+46				;Load hold variable into r16
sbrc r16,0					;If lsb of hold variable is cleared the motor should not block, and we therefore skip next line 
rjmp _end					;If the lsb of hold variable, is NOT cleared, the motor should block and we jump directly to _end

sbi 0x05,0					;Pull ENA pin high in order to release the motor driver

rjmp _end

_accelAlgorithm:
ldd r16,z+21				;Load lower part of accelSteps variable into r16
ldd r17,z+22				;Load middle-lower part of accelSteps variable into r17
ldd r18,z+23				;Load middle-upper part of accelSteps variable into r18
ldd r19,z+24				;Load middle-upper part of accelSteps variable into r19

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
std z+20,R19 				;If accelSteps is equal to 0, we are done accelerating, and the motor should change to cruise state

rjmp _cruiseAlgorithm		;Perform algorithm for cruise state

_accelAlgorithmStart:
subi r16,1					;Decrement lower 8 bits of accelSteps			
sbci r17,0					;If carry flag was set, subtract 1 from middle-lower 8 bits of accelSteps
sbci r18,0					;If carry flag was set, subtract 1 from middle-upper 8 bits of accelSteps
sbci r19,0					;If carry flag was set, subtract 1 from middle-upper 8 bits of accelSteps

std z+21,r16				;Store lower part of accelSteps variable back into ram
std z+22,r17				;store middle-lower part of accelSteps variable back into ram
std z+23,r18				;Store middle-upper part of accelSteps variable back into ram
std z+24,r19				;Store middle-upper part of accelSteps variable back into ram

rjmp _algorithm

_decelAlgorithm:
ldd r16,z+25				;Load lower part of decelSteps variable into r16
ldd r17,z+26				;Load middle-lower part of decelSteps variable into r17
ldd r18,z+27				;Load middle-upper part of decelSteps variable into r18
ldd r19,z+28				;Load upper part of decelSteps variable into r19

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
std z+20,R19 				;If decelsteps is equal to 0, we are done decelerating, and the motor should be stopped
rjmp _stopMotor				;Stop motor	

_decelAlgorithmStart:
subi r16,1					;Decrement lower 8 bits of initialDecelSteps			
sbci r17,0					;If carry flag was set, subtract 1 from middle-lower 8 bits of initialDecelSteps
sbci r18,0					;If carry flag was set, subtract 1 from middle-upper 8 bits of initialDecelSteps
sbci r19,0

std z+25,r16				;Store lower part of initialDecelSteps variable back into ram
std z+26,r17				;store middle-lower part of initialDecelSteps variable back into ram
std z+27,r18				;Store middle-upper part of initialDecelSteps variable back into ram
std z+28,r19

rjmp _algorithm

_initDecelAlgorithm:
ldd r16,z+29				;Load lower part of initialDecelSteps variable into r16
ldd r17,z+30				;Load middle-lower part of initialDecelSteps variable into r17
ldd r18,z+31				;Load middle-upper part of initialDecelSteps variable into r18
ldd r19,z+32

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
ldd r16,z+47				;Load direction variable into r16
sbrs r16,0					;If bit zero is set, we should set DIR pin, and therefore skip next line
cbi 0x0B,7					;If bit zero is cleared, we should clear DIR pin
sbrc r16,0					;if bit zero is cleared, we should clear DIR pin, and therefore skip next line
sbi 0x0B,7					;If bit zero is set, we should set DIR pin

ldi r19,_ACCEL
std z+20,r19 				;If decelsteps is equal to 0, we are done decelerating, and the state should be changed to acceleration
rjmp _accelAlgorithm

_initDecelAlgorithmStart:
subi r16,1					;Decrement lower 8 bits of initialDecelSteps			
sbci r17,0					;If carry flag was set, subtract 1 from middle-lower 8 bits of initialDecelSteps
sbci r18,0					;If carry flag was set, subtract 1 from middle-upper 8 bits of initialDecelSteps
sbci r19,0

std z+29,r16				;Store lower part of initialDecelSteps variable back into ram
std z+30,r17				;store middle-lower part of initialDecelSteps variable back into ram
std z+31,r18				;Store middle-upper part of initialDecelSteps variable back into ram
std z+32,r19


_algorithm:
; Algorithm: Delay = p = (P+P^3*multiplier)
ldd r15,z+56
ldd r16,z+57			;Load upper part of exactDelay variable
ldd r17,z+58			;Load upper part of exactDelay variable
ldd r18,z+59			;Load upper part of exactDelay variable

;ExactDelay variable is in IEEE-754 format: bit31 = sign bit, bit 30:23 = exponent, bit 22:0 = mantissa
;The following calculations are performed quite different from what the C compiler would do. This is due to the fact that the variables will always behave in a predictive
;way with regards to signs and value compared to each other. I exploit these known informations in the below calculations in order to speed up the 
;calculation time significantly compared to what the C compiler would do.

;calculating p^2 (result in r23 - r20:r19):
lsl r15
rol r16
rol r17
rol r18

;build mantissa - I only use the upper 15bits of the mantissa from these floats, since it doesn't make a noticeable difference compared to the performance gain
mul r16,r17
mov r19,r1
mov r20,r17
add r19,r1
adc r20,r17
clr r15
ldi r22,0x01
adc r22,r15
add r19,r16
adc r20,r15
adc r22,r15 
add r19,r16
adc r20,r15
adc r22,r15
mul r17,r17
add r19,r0
adc r20,r1
adc r22,r15

;Building exponent
mov r23,r18
add r23,r18
subi r23,0x7F

;r22 contains the "hidden 1" of the floating point mantissa. if this register is greater than 1, we should shift the 
;computed mantissa right one bit and increment the exponent in order to normalize the variable into IEEE-754 format
cpi r22,0x02
brlo _pSquareNormalized
	lsr r22
	ror r20
	ror r19
	inc r23
_pSquareNormalized:

;Calculating P^3. result in (r23 - r19:r18 - r15)

;Building exponent
add r23,r18
subi r23,0x7F

;build mantissa
mul r19,r17
mov r18,r1
mul r20,r16
add r18,r19
mov r19,r20
adc r19,r17
adc r22,r15
add r18,r1
adc r19,r15
adc r22,r15
mul r20,r17
add r18,r0
adc r19,r1
adc r22,r15
add r18,r16
adc r19,r15
adc r22,r15 

cpi r22,0x02
brlo _pCubeNormalized
	lsr r22
	ror r19
	ror r18
	inc r23
_pCubeNormalized:

;Calculating P^3*m. (result in r23 - r19:r18 - r15)
;load multiplier variable
ldd r16,z+12
ldd r17,z+13
ldd r20,z+14
ldd r21,z+15

lsl r16
rol r17
rol r20
rol r21

;Building exponent
add r23,r21
subi r23,0x7F

;in this calculation I am required to use the full precision of the mantissa of the multiplier in order for the algorithm to work.
;Infact, i actually need about 8 more bits in the mantissa of both the multiplier variable and the exactDelay variable to have enough precision for low
;and high speeds. This however is compensated for further down. The extra precision needed in the exactDelay, is only needed in the addition/subtraction step below
;and not int the P^3 calculations.

;Build mantissa
mul r20,r15
lsl r0
adc r15,r1
clr r22
adc r22,r22
mul r18,r17
lsl r0
adc r15,r1
clr r1
adc r22,r1
mul r18,r20
add r15,r0
adc r18,r22
clr r22
adc r22,r22
add r18,r1
clr r1
adc r22,r1
mul r19,r17
add r15,r16
adc r18,r17
adc r22,r20
add r18,r1
adc r22,r19
clr r17
adc r17,r17
mul r19,r20
add r18,r0
adc r22,r1
ldi r20,0x01
adc r17,r20
mul r19,r16
lsl r0
adc r15,r1
clr r20
adc r18,r20
adc r22,r20
adc r17,r20

cpi r17,0x02
brlo _pCubeMulNormalized
	lsr r17
	ror r22
	ror r18
	ror r15
	inc r23
_pCubeMulNormalized:

mov r19,r22				;move r22 to r19 to ease stuff in the next calculation

;Calculating p + p^3*m
;reload the exactDelay variable. in this step i need to use the full precision of the IEEE-754 float
ldd r16,z+56
ldd r17,z+57
ldd r20,z+58
ldd r21,z+59

lsl r16
rol r17
rol r20
rol r21

;If exactDelay is greater than or equal to P^3*m, interchange the two variables.
cp r21,r23
brsh _subExponent
mov r22,r15
mov r15,r16
mov r16,r22
mov r22,r17
mov r17,r18
mov r18,r22
mov r22,r20
mov r20,r19
mov r19,r22
mov r22,r21
mov r21,r23
mov r23,r22

;Calculate the difference in exponents between the two values in order to find out how much the lower number should be shifted to the 
;right to get equal exponents
_subExponent:
mov r22,r21
sub r21,r23

ldi r23,0x01 			;Load the "hidden 1" into r23 in order to be able to shift it right into the mantissa

cpi r21,0x00
breq _pExpSame			;if the exponents are equal, we dont need to shift
;We should shift at least one time to the right to get the "hidden 1" into the mantissa
lsr r23
ror r19
ror r18
ror r15
dec r21

;if the exponents are of same size we are done shifting. else we need to proceed
cpi r21,0x00
brne _pShiftMantissa
rjmp _pExpSame
_pShiftMantissa:
	;if the mantissa needs to be shifted 20 or more places to the right, we swap the nibbles of the most significant mantissa byte,
	;move it to the least significant byte and clear the top 20 bytes of the mantissa
	cpi r21,0x14
	brlo _p16Shifts		;If not, check if we should shift 16 or more times
	swap r19
	andi r19,0x0F
	mov r15,r19
	clr r18
	clr r19
	subi r21,0x14
_p16Shifts:
	cpi r21,0x10
	brlo _p8Shifts		;If not, check if we should shift 8 or more times
	mov r15,r19
	clr r18
	clr r19
	subi r21,0x10
_p8Shifts:
	cpi r21,0x08
	brlo _p4Shifts		;If not, check if we should shift 4 or more times
	mov r15,r18
	mov r18,r19
	clr r19
	subi r21,0x08

_p4Shifts:
	cpi r21,0x04
	brlo _pFinishMantissaShift		;shift the remaining number of bits
	lsr r19
	ror r18
	ror r15

	lsr r19
	ror r18
	ror r15

	lsr r19
	ror r18
	ror r15

	lsr r19
	ror r18
	ror r15
	subi r21,0x04

_pFinishMantissaShift:
	cpi r21,0x00
	breq _pExpSame
_pShiftLast:
	lsr r19
	ror r18
	ror r15
	dec r21
	cpi r21,0x00
	brne _pShiftLast

_pExpSame:
ldd r21,z+20				;Load the state variable
cpi r21,_ACCEL 				;If we are currently in the acceleration phase, we should subtract P^3*m from the exactDelay variable
breq _pSubtractMantissa 	;If we are currently NOT in the acceleration phase, we should add P^3*m to the exactDelay variable
mov r21,r22
mov r22,r23

add r16,r15
adc r17,r18
adc r20,r19
adc r22,r22
inc r22

/*** DIRTY HAX !!! ***/
/*** To compensate for lack of precision in floats (Yes i know it sounds crazy, but its true !) ***/
;This is a rather embarrasing piece of code to have. I know how to fix this, however it takes some time to implement, and since this has an acceptable
;level of performance, I am going to prioritize getting the code for the uStepper robotic arm written, over this fix !
;That being said, I will try to fix this as soon as i get around to it!!
ldi r18,0x01
add r16,r18
clr r18
adc r17,r18
adc r20,r18
adc r22,r18
/*** END OF DIRTY HAX!!! ****/

rjmp _pMantissaCalculated	;we are done calulating the mantissa


_pSubtractMantissa:
mov r21,r22
ldi r22,0x01

sub r16,r15
sbc r17,r18
sbc r20,r19
sbc r22,r23

;Now we need to renormalize the mantissa, so the "hidden 1" gets into the correct place (r22 should contain this, and should therefore equal exactly 1)
;We do this normalization by shifting the radix point left or right till it fits, and incrementing/decrementing the exponent as needed
_pMantissaCalculated:
cpi r22,0x01
brlo _pShiftRadixLeft
cpi r22,0x02
brsh _pShiftRadixRight
rjmp _pRadixDone

_pShiftRadixLeft:
	lsl r16
	rol r17
	rol r20
	rol r22
	dec r21
	cpi r22,0x01
	brne _pShiftRadixLeft
	rjmp _pRadixDone

_pShiftRadixRight:
	lsr r22
	ror r20
	ror r17
	ror r16
	inc r21
	cpi r22,0x02
	brsh _pShiftRadixRight

_pRadixDone:
	lsr r21
	ror r20
	ror r17
	ror r16

;store the exactDelay variable back into ram
	std z+56,r16
	std z+57,r17
	std z+58,r20
	std z+59,r21

;subtract 0.5 to exactDelay variable in order to round correctly!
;Usually one would add 0.5 instead of subtracting, however in our case we need to subtract 1 from the calculated delay
;meaning that the addition of 0.5 turns into a subtraction of 0.5

lsl r20
rol r21

lsr r20
mov r23,r20
ori r20,0x80
subi r21,0x7E		;The real offset is 0x7F, however we need to shift an additional place to the left in order to get the "invisible" 1 into the variable correctly
cpi r21,0x10

brlo _convertToInteger
ldi r20,0xFF
std z+10,r20
std z+11,r20
rjmp _end

_convertToInteger:
clr r18
clr r19

cpi r21,0x10
brlo _storeDelayCmp12
mov r19,r20
mov r18,r17
mov r20,r16
rjmp _storeDelayInRam

_storeDelayCmp12:
cpi r21,0x0C
brlo _storeDelayCmp8
mov r19,r20
swap r19
mov r18,r19
andi r19,0x0F
andi r18,0xF0
swap r17
mov r20,r17
andi r20,0xF0
andi r17,0x0F
or r18,r17
swap r16
andi r16,0x0F
or r20,r16
subi r21,0x0C
rjmp _storeDelayFinishShift

_storeDelayCmp8:
cpi r21,0x08
brlo _storeDelayCmp4
mov r18,r20
mov r20,r17
mov r16,r17
subi r21,0x08
rjmp _storeDelayFinishShift

_storeDelayCmp4:
cpi r21,0x04
brlo _storeDelayFinishShift
swap r20
mov r18,r20
andi r18,0x0F
andi r20,0xF0
swap r17
andi r17,0x0F
or r20,r17
subi r21,0x04
_storeDelayFinishShift:

cpi r21,0x00
breq _storeDelayInRam
lsl r20
rol r18
rol r19
dec r21
rjmp _storeDelayFinishShift

;Store integer part of rounded exactDelay variable in delay variable
_storeDelayInRam:

subi r20,0x80
sbci r18,0
sbci r19,0

std z+10,r18
std z+11,r19
rjmp _end	

_cruiseAlgorithm:
ldd r16,z+45				;Load continous variable into r16
cpi r16,0					;If continous variable is equal to 0, we should decrement cruisesteps (skip next line)
brne _continous 			;If motor is running in continous mode, we should not decrement cruiseSteps

	_nonContinous:
		ldd r16,z+33		;Load lower part of cruiseSteps variable into r16
		ldd r17,z+34		;Load lower-middle part of cruiseSteps variable into r17
		ldd r18,z+35		;Load upper-middle part of cruiseSteps variable into r18
		ldd r19,z+36		

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
		std z+20,r20
		ldd r16,z+19
		std z+59,r16
		ldd r16,z+18
		std z+58,r16
		ldd r16,z+17
		std z+57,r16
		ldd r16,z+16
		std z+56,r16
		
		rjmp _decelAlgorithm

		_cruiseAlgorithmStart:
		subi r16,1			;Decrement lower 8 bits of cruiseSteps			
		sbci r17,0			;If carry flag was set, subtract 1 from lower-middle 8 bits of cruiseSteps
		sbci r18,0			;If carry flag was set, subtract 1 from upper-middle 8 bits of cruiseSteps
		sbci r19,0
		std z+33,r16		;Store lower part of cruiseSteps variable back into ram
		std z+34,r17		;store lower-middle part of cruiseSteps variable back into ram
		std z+35,r18		;store upper-middle part of cruiseSteps variable back into ram
		std z+36,r19

	_continous:
		
		ldd r16,z+0			;Load lower part of cruiseDelay variable
		ldd r17,z+1			;Load upper part of cruiseDelay variable
		std z+10,r16		;Store lower part of cruiseDelay into lower part of delay variable
		std z+11,r17		;Store upper part of cruiseDelay into upper part of delay variable 
		rjmp _end			;We are done

_end:
;We are done, and should restore general purpose registers from the stack, in order for the software to continue its operation properly!
cbi 0x0B,4

pop r31
pop r30
pop r23
pop r22
pop r21
pop r20
pop r19
pop r18
pop r17
pop r16
pop r15
pop r0
out _SREG,r0
pop r1
pop r0

;Return from interrupt routine
reti