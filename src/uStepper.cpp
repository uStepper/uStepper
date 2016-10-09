/********************************************************************************************
* 	 	File: 		uStepper.cpp 															*
*		Version:    0.4.5             	                             						*
*      	date: 		August 11th, 2016	                                    				*
*      	Author: 	Thomas Hørring Olsen                                   					*
*                                                   										*	
*********************************************************************************************
*			            uStepper class 					   									*
* 																							*
*	This file contains the implementation of the class methods, incorporated in the  		*
*	uStepper arduino library. The library is used by instantiating an uStepper object 		*
*	by calling either of the two overloaded constructors: 									*
*																							*
*		example:																			*
*																							*
*		uStepper stepper; 																	*
*																							*
*		OR 																					*
*																							*
*		uStepper stepper(500, 2000);														*
*																							*
*	The first instantiation above creates a uStepper object with default acceleration 		*
*	and maximum speed (1000 steps/s^2 and 1000steps/s respectively).						*
*	The second instantiation overwrites the default settings of acceleration and 			*
*	maximum speed (in this case 500 steps/s^2 and 2000 steps/s, respectively);				*
*																							*
*	after instantiation of the object, the object setup function should be called within 	*
*	arduino's setup function:																*
*																							*
*		example:																			*
*																							*
*		uStepper stepper;																	*
*																							*
*		void setup()																		*
*		{																					*
*			stepper.setup();																*
*		} 																					*
*																							*
*		void loop()																			*
*		{																					*
*																							*
*		}																					*
*																							*
*	After this, the library is ready to control the motor!									*
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
/**	\file uStepper.cpp
*	\brief Class implementations for the uStepper library
*	
*	This file contains the implementations of the classes defined in uStepper.h
*	
*	\author Thomas Hørring Olsen (thomas@ustepper.com)
*/
#include <uStepper.h>
#include <math.h>

uStepper *pointer;
i2cMaster I2C;

extern "C" {

	void interrupt1(void)
	{
		pointer->speedValue[1] = pointer->speedValue[0];
		pointer->speedValue[0] = micros();

		if((PIND & (0x20)))			//CCW
		{
			if(pointer->control == 0)
			{
				PORTD |= (1 << 7);	//Set dir to CCW

				PORTD |= (1 << 4);		//generate step pulse

				pointer->stepsSinceReset--;
				PORTD &= ~(1 << 4);		//pull step pin low again
			}		
			pointer->stepCnt--;				//DIR is set to CCW, therefore we subtract 1 step from step count (negative values = number of steps in CCW direction from initial postion)
		}
		else						//CW
		{
			if(pointer->control == 0)
			{
				
				PORTD &= ~(1 << 7);	//Set dir to CW

				PORTD |= (1 << 4);		//generate step pulse
				pointer->stepsSinceReset++;
				PORTD &= ~(1 << 4);		//pull step pin low again
			}
			pointer->stepCnt++;			//DIR is set to CW, therefore we add 1 step to step count (positive values = number of steps in CW direction from initial postion)	
		}
	}

	void interrupt0(void)
	{
		if(PIND & 0x04)
		{
			
			PORTB |= (1 << 0);
		}
		else
		{
			PORTB &= ~(1 << 0);
		}
	}

	void TIMER2_COMPA_vect(void)
	{	
		asm volatile("push r16 \n\t");
		asm volatile("in r16,0x3F \n\t");
		asm volatile("push r16 \n\t");
		asm volatile("push r30 \n\t");
		asm volatile("push r31 \n\t");
		asm volatile("lds r30,pointer \n\t");
		asm volatile("lds r31,pointer+1 \n\t");
		asm volatile("ldd r16,z+56 \n\t");
		asm volatile("sbrs r16,0 \n\t");
		asm volatile("jmp _AccelerationAlgorithm \n\t");	//Execute the acceleration profile algorithm
		
		asm volatile("push r0 \n\t");
		asm volatile("push r1 \n\t");
		asm volatile("push r2 \n\t");
		asm volatile("push r3 \n\t");
		asm volatile("push r4 \n\t");
		asm volatile("push r5 \n\t");
		asm volatile("push r6 \n\t");
		asm volatile("push r7 \n\t");
		asm volatile("push r8 \n\t");
		asm volatile("push r9 \n\t");
		asm volatile("push r10 \n\t");
		asm volatile("push r11 \n\t");
		asm volatile("push r12 \n\t");
		asm volatile("push r13 \n\t");
		asm volatile("push r14 \n\t");
		asm volatile("push r15 \n\t");
		asm volatile("push r17 \n\t");
		asm volatile("push r18 \n\t");
		asm volatile("push r19 \n\t");
		asm volatile("push r20 \n\t");
		asm volatile("push r21 \n\t");
		asm volatile("push r22 \n\t");
		asm volatile("push r23 \n\t");
		asm volatile("push r24 \n\t");
		asm volatile("push r25 \n\t");
		asm volatile("push r26 \n\t");
		asm volatile("push r27 \n\t");
		asm volatile("push r28 \n\t");
		asm volatile("push r29 \n\t");

		if(pointer->counter < pointer->delay)		//This value defines the speed at which the motor rotates when compensating for lost steps. This value should be tweaked to the application
		{
			pointer->counter++;
		}
		else
		{	
			PORTD |= (1 << 4);
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			asm volatile("nop \n\t");
			PORTD &= ~(1 << 4);
			pointer->counter = 0;
			if(pointer->control > 0)
			{
				pointer->control--;
			}
			else
			{
				pointer->control++;
			}

			if(!pointer->control)
			{
				pointer->stopTimer();
			}
		}
		asm volatile("pop r29 \n\t");
		asm volatile("pop r28 \n\t");
		asm volatile("pop r27 \n\t");
		asm volatile("pop r26 \n\t");
		asm volatile("pop r25 \n\t");
		asm volatile("pop r24 \n\t");
		asm volatile("pop r23 \n\t");
		asm volatile("pop r22 \n\t");
		asm volatile("pop r21 \n\t");
		asm volatile("pop r20 \n\t");
		asm volatile("pop r19 \n\t");
		asm volatile("pop r18 \n\t");
		asm volatile("pop r17 \n\t");
		asm volatile("pop r15 \n\t");
		asm volatile("pop r14 \n\t");
		asm volatile("pop r13 \n\t");
		asm volatile("pop r12 \n\t");
		asm volatile("pop r11 \n\t");
		asm volatile("pop r10 \n\t");
		asm volatile("pop r9 \n\t");
		asm volatile("pop r8 \n\t");
		asm volatile("pop r7 \n\t");
		asm volatile("pop r6 \n\t");
		asm volatile("pop r5 \n\t");
		asm volatile("pop r4 \n\t");
		asm volatile("pop r3 \n\t");
		asm volatile("pop r2 \n\t");
		asm volatile("pop r1 \n\t");
		asm volatile("pop r0 \n\t");
		asm volatile("pop r31 \n\t");
		asm volatile("pop r30 \n\t");
		asm volatile("pop r16 \n\t");
		asm volatile("out 0x3F,r16 \n\t");
		asm volatile("pop r16 \n\t");	
		asm volatile("reti \n\t");
	}

	/*
	void TIMER2_COMPA_vect(void)
	{	
		
		asm volatile("push r16 \n\t");
		asm volatile("in r16,0x3F \n\t");
		asm volatile("push r16 \n\t");
		asm volatile("push r30 \n\t");
		asm volatile("push r31 \n\t");
		asm volatile("lds r30,pointer \n\t");
		asm volatile("lds r31,pointer+1 \n\t");
		asm volatile("ldd r16,z+56 \n\t");
		asm volatile("sbrs r16,0 \n\t");
		asm volatile("jmp _AccelerationAlgorithm \n\t");	//Execute the acceleration profile algorithm

		asm volatile("push r17 \n\t");
		asm volatile("push r18 \n\t");
		asm volatile("push r19 \n\t");
		asm volatile("push r20 \n\t");

		asm volatile("ldi r16,54 \n\t");	//Offset pointer to point to delay variable as first address
		asm volatile("add r30,r16 \n\t");	
		asm volatile("clr r16 \n\t");
		asm volatile("adc r31,r16 \n\t");
		asm volatile("ldd r16,z+0 \n\t");			//Load delay variable
		asm volatile("ldd r17,z+1 \n\t");			//load delay variable
		asm volatile("ldd r18,z+22 \n\t");		//load counter
		asm volatile("ldd r19,z+23 \n\t");
		
		asm volatile("cpse r17,r19 \n\t");			//if high nibble of counter and delay variable is equal, it could be time to step.
		asm volatile("rjmp _notRdyTimer2 \n\t");	//if not, it certainly is not time
		asm volatile("cpse r16,r18 \n\t");			//if low nibble of counter and delay variable is equal, it is time to step.
		asm volatile("rjmp _notRdyTimer2 \n\t");	//if not, it is not time yet
		asm volatile("sbi 0x05,5 \n\t");
		asm volatile("sbi 0x0B,4 \n\t");				//PORTD |= (1 << 4);
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("nop \n\t");
		asm volatile("cbi 0x0B,4 \n\t");				//PORTD &= ~(1 << 4);	

		asm volatile("clr r17 \n\t");
		asm volatile("std z+22,r17 \n\t");
		asm volatile("std z+23,r17 \n\t");
	
		asm volatile("ldd r16,z+28 \n\t");			//Load high nibble of control variable
		asm volatile("ldd r17,z+29 \n\t");			//Load high nibble of control variable
		asm volatile("ldd r18,z+30 \n\t");			//Load high nibble of control variable
		asm volatile("ldd r19,z+31 \n\t");			//Load high nibble of control variable
		asm volatile("cpi r16,0x00 \n\t");
		asm volatile("brne _adjustControl \n\t");
		asm volatile("cpi r17,0x00 \n\t");
		asm volatile("brne _adjustControl \n\t");
		asm volatile("cpi r18,0x00 \n\t");
		asm volatile("brne _adjustControl \n\t");
		asm volatile("cpi r19,0x00 \n\t");
		asm volatile("brne _adjustControl \n\t");
		asm volatile("lds r16,0x0070 \n\t");		//load contents of TIMSK2
		asm volatile("andi r16,0xFD \n\t");			//Clear bit 1 (OCIE2A) of TIMSK2 in order to stop timer 2
		asm volatile("sts 0x0070,r16 \n\t");		//Store new TIMSK2 value into TIMSK2 register

		asm volatile("rjmp _endTimer2 \n\t");

		asm volatile("_adjustControl: \n\t");
		asm volatile("cpi r19,0x80 \n\t");
		asm volatile("brlo _subtractControl \n\t");

		asm volatile("ldi r20,0x01\n\t");
		asm volatile("add r16,r20 \n\t");
		asm volatile("clr r20 \n\t");
		asm volatile("adc r17,r20 \n\t");
		asm volatile("adc r18,r20 \n\t");
		asm volatile("adc r19,r20 \n\t");
		asm volatile("rjmp _endStepTimer2 \n\t");

		asm volatile("_subtractControl: \n\t");
		asm volatile("brlo _subtractControl \n\t");
		asm volatile("subi r16,0x01 \n\t");
		asm volatile("sbci r17,0x00 \n\t");
		asm volatile("sbci r18,0x00 \n\t");
		asm volatile("sbci r19,0x00 \n\t");
		asm volatile("rjmp _endStepTimer2 \n\t");

		asm volatile("_notRdyTimer2: \n\t");
		asm volatile("inc r18 \n\t");
		asm volatile("clr r16 \n\t");
		asm volatile("adc r19,r16 \n\t");
		asm volatile("std z+38,r18 \n\t");
		asm volatile("std z+39,r19 \n\t");
		asm volatile("rjmp _endTimer2 \n\t");

		asm volatile("_endStepTimer2: \n\t");
		asm volatile("std z+4,r16 \n\t");
		asm volatile("std z+5,r17 \n\t");
		asm volatile("std z+6,r18 \n\t");
		asm volatile("std z+7,r19 \n\t");

		asm volatile("_endTimer2: \n\t");

		asm volatile("pop r20 \n\t");
		asm volatile("pop r19 \n\t");
		asm volatile("pop r18 \n\t");
		asm volatile("pop r17 \n\t");
		asm volatile("pop r31 \n\t");
		asm volatile("pop r30 \n\t");
		asm volatile("pop r16 \n\t");
		asm volatile("out 0x3F,r16 \n\t");
		asm volatile("pop r16 \n\t");	
		asm volatile("cbi 0x05,5 \n\t");	
		asm volatile("reti \n\t");

	}*/

	void TIMER1_COMPA_vect(void)
	{
		uint8_t data[2];
		uint16_t curAngle;
		int16_t deltaAngle;
		float newSpeed;
		static float deltaSpeedAngle = 0.0;
		static uint8_t loops = 0;
		static uint16_t oldAngle = 0;
		static int16_t revolutions = 0;
		float error;
		static uint32_t speed = 10000;
		static uint32_t oldMicros = 0;

		sei();
		if(I2C.getStatus() != I2CFREE)
		{
			return;
		}
		
		I2C.read(ENCODERADDR, ANGLE, 2, data);
		cli();
			error = (float)pointer->stepCnt;
		
			if(pointer->speedValue[0] == oldMicros)
			{
				speed += 2000;
				if(speed > 10000)
				{
					speed = 10000;
				}
			}
			else
			{
				speed = pointer->speedValue[0] - pointer->speedValue[1];
			}
			
		sei();
		
		curAngle = (((uint16_t)data[0]) << 8 ) | (uint16_t)data[1];
		pointer->encoder.angle = curAngle;
		curAngle -= pointer->encoder.encoderOffset;
		if(curAngle > 4095)
		{
			curAngle -= 61440;
		}

		deltaAngle = (int16_t)oldAngle - (int16_t)curAngle;

		if(deltaAngle < -2047)
		{
			revolutions--;
			deltaAngle += 4096;
		}
		
		else if(deltaAngle > 2047)
		{
			revolutions++;
			deltaAngle -= 4096;
		}

		if( loops < 10)
		{
			loops++;
			deltaSpeedAngle += (float)deltaAngle;
		}
		else
		{
			newSpeed = deltaSpeedAngle*ENCODERSPEEDCONSTANT;
			if(pointer->dropIn)
			{
				newSpeed *= 0.5;
			}
			pointer->encoder.curSpeed = newSpeed;
			loops = 0;
			deltaSpeedAngle = 0.0;
		}

		pointer->encoder.angleMoved = (int32_t)curAngle + (4096*(int32_t)revolutions);
		oldAngle = curAngle;

		if(pointer->dropIn)
		{
			error = (((float)pointer->encoder.angleMoved * pointer->stepConversion) - error); 
			pointer->pidDropIn(error, speed);
		}
	}
}

float2::float2(void)
{

}

float float2::getFloatValue(void)
{
	union
	{
		float f;
		uint32_t i;
	} a;

	a.i = (uint32_t)(this->value >> 25);

	return a.f;
}

uint64_t float2::getRawValue(void)
{
	return this->value;
}

void float2::setValue(float val)
{
	union
	{
		float f;
		uint32_t i;
	} a;

	a.f = val;

	this->value = ((uint64_t)a.i) << 25;
}

bool float2::operator<=(const float &value)
{
	if(this->getFloatValue() > value)
	{
		return 0;
	}

	if(this->getFloatValue() == value)
	{
		if((this->value & 0x0000000000007FFF) > 0)
		{
			return 0;
		}
	}

	return 1;
}

bool float2::operator<=(const float2 &value)
{
	if((this->value >> 56) > (value.value >> 56))				// sign bit of "this" bigger than sign bit of "value"?
	{
		return 1;												//"This" is negative while "value" is not. ==> "this" < "value"
	}

	if((this->value >> 56) == (value.value >> 56))				//Sign bit of "this" == sign bit of "value"?
	{
		if( (this->value >> 48) < (value.value >> 48) )			//Exponent of "this" < exponent of "value"?
		{
			return 1;											//==> "this" is smaller than "value"
		}

		if( (this->value >> 48) == (value.value >> 48) )		//Exponent of "this" == exponent of "value"?
		{
			if((this->value & 0x0000FFFFFFFFFFFF) <= (value.value & 0x0000FFFFFFFFFFFF))		//mantissa of "this" <= mantissa of "value"?
			{
				return 1;										//==> "this" <= "value"
			}
		}
	}

	return 0;													//"this" > "value"
}

float2 & float2::operator=(const float &value)
{
	this->setValue(value);

	return *this;
}

float2 & float2::operator+=(const float &value)
{

}

float2 & float2::operator+=(const float2 &value)
{
	float2 temp = value;
	uint64_t tempMant, tempExp;
	uint8_t cnt;	//how many times should we shift the mantissa of the smallest number to add the two mantissa's

	if((this->value >> 56) == (temp.value >> 56))
	{
		if(*this <= temp)
		{
			cnt = (temp.value >> 48) - (this->value >> 48);
			if(cnt < 48)
			{
				tempExp = (temp.value >> 48);

				this->value &= 0x0000FFFFFFFFFFFF;
				this->value |= 0x0001000000000000;
				this->value >>= cnt;

				tempMant = (temp.value & 0x0000FFFFFFFFFFFF) | 0x0001000000000000;
				tempMant += this->value;

				while(tempMant > 0x2000000000000)
				{
					tempMant >>= 1;
					tempExp++;
				}

				tempMant &= 0x0000FFFFFFFFFFFF;
				this->value = (tempExp << 48) | tempMant;
			}
			else
			{
				this->value = temp.value;
			}
		}

		else
		{
			cnt = (this->value >> 48) - (temp.value >> 48);

			if(cnt < 48)
			{
				tempExp = (this->value >> 48);

				temp.value &= 0x0000FFFFFFFFFFFF;
				temp.value |= 0x0001000000000000;
				temp.value >>= cnt;

				tempMant = (this->value & 0x0000FFFFFFFFFFFF) | 0x0001000000000000;
				tempMant += temp.value;

				while(tempMant > 0x2000000000000)
				{
					tempMant >>= 1;
					tempExp++;
				}

				tempMant &= 0x0000FFFFFFFFFFFF;
				this->value = (tempExp << 48) | tempMant;
			}
		}
	}	

	else if((this->value >> 56) == 1)
	{
		this->value &= 0x00FFFFFFFFFFFFFF;	//clear sign bit, to consider absolute value

		if(*this <= temp)
		{
			cnt = (temp.value >> 48) - (this->value >> 48);

			if(cnt < 48)
			{
				tempExp = (temp.value >> 48);

				this->value &= 0x0000FFFFFFFFFFFF;
				this->value |= 0x0001000000000000;
				this->value >>= cnt;

				tempMant = (temp.value & 0x0000FFFFFFFFFFFF) | 0x0001000000000000;

				tempMant -= this->value;

				if(tempMant > 0x8000000000000000)
				{

					tempMant &= 0x0000FFFFFFFFFFFF;
					tempExp--;
				}

				while(tempMant < 0x1000000000000)
				{
					tempMant <<= 1;
					tempExp--;
				}

				tempMant &= 0x0000FFFFFFFFFFFF;

				this->value = (tempExp << 48) | tempMant;
			}

			else
			{
				this->value = temp.value;
			}
		}

		else
		{
			cnt = (this->value >> 48) - (temp.value >> 48);
			if(cnt < 48)
			{
				tempExp = (this->value >> 48);

				temp.value &= 0x0000FFFFFFFFFFFF;
				temp.value |= 0x0001000000000000;
				temp.value >>= cnt;

				tempMant = (this->value & 0x0000FFFFFFFFFFFF) | 0x0001000000000000;

				tempMant -= temp.value;

				if(tempMant > 0x8000000000000000)
				{
					tempMant &= 0x0000FFFFFFFFFFFF;
					tempExp--;
				}

				while(tempMant < 0x1000000000000)
				{
					tempMant <<= 1;
					tempExp--;
				}

				tempMant &= 0x0000FFFFFFFFFFFF;

				this->value = (tempExp << 48) | tempMant;
				this->value |= 0x0100000000000000;				
			}
		}
	}

	else
	{
		temp.value &= 0x00FFFFFFFFFFFFFF;	//clear sign bit, to consider absolute value

		if(temp <= *this)
		{
			cnt = (this->value >> 48) - (temp.value >> 48);
			if(cnt < 48)
			{
				tempExp = (this->value >> 48);

				temp.value &= 0x0000FFFFFFFFFFFF;
				temp.value |= 0x0001000000000000;
				temp.value >>= cnt;

				tempMant = (this->value & 0x0000FFFFFFFFFFFF) | 0x0001000000000000;

				tempMant -= temp.value;

				if(tempMant > 0x8000000000000000)
				{
					tempMant &= 0x0000FFFFFFFFFFFF;
					tempExp--;
				}

				while(tempMant < 0x1000000000000)
				{
					tempMant <<= 1;
					tempExp--;
				}

				tempMant &= 0x0000FFFFFFFFFFFF;

				this->value = (tempExp << 48) | tempMant;
			}
		}

		else
		{
			cnt = (temp.value >> 48) - (this->value >> 48);
			if(cnt < 48)
			{
				tempExp = (temp.value >> 48);

				this->value &= 0x0000FFFFFFFFFFFF;
				this->value |= 0x0001000000000000;
				this->value >>= cnt;

				tempMant = (temp.value & 0x0000FFFFFFFFFFFF) | 0x0001000000000000;

				tempMant -= this->value;

				if(tempMant > 0x8000000000000000)
				{
					tempMant &= 0x0000FFFFFFFFFFFF;
					tempExp--;
				}

				while(tempMant < 0x1000000000000)
				{
					tempMant <<= 1;
					tempExp--;
				}

				tempMant &= 0x0000FFFFFFFFFFFF;

				this->value = (tempExp << 48) | tempMant;
				this->value |= 0x0100000000000000;				
			}

			else
			{
				this->value = temp.value;
				this->value |= 0x0100000000000000;
			}
		}
	}

	return *this;

}

uStepperTemp::uStepperTemp(void)
{

}

float uStepperTemp::getTemp(void)
{
	float T = 0.0;
	float Vout = 0.0;
	float NTC = 0.0;

	Vout = analogRead(TEMP)*0.0048828125;	//0.0048828125 = 5V/1024
	NTC = ((R*5.0)/Vout)-R;							//the current NTC resistance
	NTC = log(NTC);
	T = A + B*NTC + C*NTC*NTC*NTC;    				//Steinhart-Hart equation
	T = 1.0/T;

	return T - 273.15;
}

uStepperEncoder::uStepperEncoder(void)
{
	I2C.begin();
}

float uStepperEncoder::getAngleMoved(void)
{
	return (float)this->angleMoved*0.087890625;
}

float uStepperEncoder::getSpeed(void)
{
	return this->curSpeed;
}

void uStepperEncoder::setup(uint8_t mode)
{
	uint8_t data[2];
	TCNT1 = 0;

	if(mode)
	{
		ICR1 = 32000;
	}
	else
	{
		ICR1 = 16000;
	}
	
	TIFR1 = 0;
	TIMSK1 = (1 << OCIE1A);
	TCCR1A = (1 << WGM11);
	TCCR1B = (1 << WGM12) | (1 << WGM13) | (1 << CS10);
	I2C.read(ENCODERADDR, ANGLE, 2, data);
	this->encoderOffset = (((uint16_t)data[0]) << 8 ) | (uint16_t)data[1];

	this->oldAngle = 0;
	this->angleMoved = 0;

	sei();
}

void uStepperEncoder::setHome(void)
{
	cli();
	uint8_t data[2];
	I2C.read(ENCODERADDR, ANGLE, 2, data);
	this->encoderOffset = (((uint16_t)data[0]) << 8 ) | (uint16_t)data[1];

	this->angle = 0;
	this->oldAngle = 0;
	this->angleMoved = 0;
	sei();
}

float uStepperEncoder::getAngle()
{
	return (float)this->angle*0.087890625;
}

uint16_t uStepperEncoder::getStrength()
{
	uint8_t data[2];

	I2C.read(ENCODERADDR, MAGNITUDE, 2, data);

	return (((uint16_t)data[0]) << 8 )| (uint16_t)data[1];
}

uint8_t uStepperEncoder::getAgc()
{
	uint8_t data;

	I2C.read(ENCODERADDR, MAGNITUDE, 1, &data);

	return data;
}

uint8_t uStepperEncoder::detectMagnet()
{
	uint8_t data;

	I2C.read(ENCODERADDR, AGC, 1, &data);

	data &= 0x38;					//For some reason the encoder returns random values on reserved bits. Therefore we make sure reserved bits are cleared before checking the reply !

	if(data == 0x08)
	{
		return 1;					//magnet too strong
	}

	else if(data == 0x10)
	{
		return 2;					//magnet too weak
	}

	else if(data == 0x20)
	{
		return 0;					//magnet detected and within limits
	}

	return 3;						//Something went horribly wrong !
}

uStepper::uStepper(void)
{

	this->state = STOP;

	this->setMaxAcceleration(1000.0);
	this->setMaxVelocity(1000.0);

	pointer = this;

	DDRD |= (1 << 7);		//set direction pin to output
	DDRD |= (1 << 4);		//set step pin to output
	DDRB |= (1 << 0);		//set enable pin to output
}

uStepper::uStepper(float accel, float vel)
{
	this->state = STOP;

	this->setMaxVelocity(vel);
	this->setMaxAcceleration(accel);

	pointer = this;

	DDRD |= (1 << 7);		//set direction pin to output
	DDRD |= (1 << 4);		//set step pin to output
	DDRB |= (1 << 0);		//set enable pin to output
}

void uStepper::setMaxAcceleration(float accel)
{
	this->acceleration = accel;

	this->stopTimer();			//Stop timer so we dont fuck up stuff !
	this->multiplier.setValue((this->acceleration/(INTFREQ*INTFREQ)));	//Recalculate multiplier variable, used by the acceleration algorithm since acceleration has changed!
	
	if(this->state != STOP)
	{
		if(this->continous == 1)	//If motor was running continously
		{
			this->runContinous(this->direction);	//We should make it run continously again
		}
		else						//If motor still needs to perform some steps
		{
			this->moveSteps(this->totalSteps - this->currentStep + 1, this->direction, this->hold);	//we should make sure the motor gets to execute the remaining steps				
		}
	}
}

float uStepper::getMaxAcceleration(void)
{
	return this->acceleration;
}

void uStepper::setMaxVelocity(float vel)
{
	if(vel < 0.5005)
	{
		this->velocity = 0.5005;			//Limit velocity in order to not overflow delay variable
	}

	else if(vel > 28000.0)
	{
		this->velocity = 28000.0;			//limit velocity in order to not underflow delay variable
	}

	else
	{
		this->velocity = vel;
	}

	this->stopTimer();			//Stop timer so we dont fuck up stuff !
	this->cruiseDelay = (uint16_t)((INTFREQ/this->velocity) - 0.5);	//Calculate cruise delay, so we dont have to recalculate this in the interrupt routine
	
	if(this->state != STOP)		//If motor was running, we should make sure it runs again
	{
		if(this->continous == 1)	//If motor was running continously
		{
			this->runContinous(this->direction);	//We should make it run continously again
		}
		else					//If motor still needs to perform some steps
		{
			this->moveSteps(this->totalSteps - this->currentStep + 1, this->direction, this->hold);	//we should make sure it gets to execute these steps	
		}
	}
}

float uStepper::getMaxVelocity(void)
{
	return this->velocity;
}

void uStepper::runContinous(bool dir)
{
	float curVel;

	if(this->dropIn)
	{
		return;		//Drop in feature is activated. just return since this function makes no sense with drop in activated!
	}
	
	this->stopTimer();				//Stop interrupt timer, so we don't fuck up stuff !
	this->continous = 1;			//Set continous variable to 1, in order to let the interrupt routine now, that the motor should run continously
	
	

	if(state != STOP)										//if the motor is currently running and we want to move the opposite direction, we need to decelerate in order to change direction.
	{
		curVel = INTFREQ/this->exactDelay.getFloatValue();								//Use this to calculate current velocity
		if(dir != digitalRead(DIR))							//If motor is currently running the opposite direction as desired
		{
			this->direction = dir;
			this->state = INITDECEL;							//We should decelerate the motor to full stop before accelerating the speed in the opposite direction
			this->initialDecelSteps = (uint32_t)(((curVel*curVel))/(2.0*this->acceleration));		//the amount of steps needed to bring the motor to full stop. (S = (V^2 - V0^2)/(2*-a)))
			this->accelSteps = (uint32_t)((this->velocity*this->velocity)/(2.0*this->acceleration));			//Number of steps to bring the motor to max speed (S = (V^2 - V0^2)/(2*a)))

			this->exactDelay.setValue(INTFREQ/sqrt((curVel*curVel) + 2.0*this->acceleration));	//number of interrupts before the first step should be performed.

			if(this->exactDelay.getFloatValue() >= 65535.5)
			{
				this->delay = 0xFFFF;
			}
			else
			{
				this->delay = (uint16_t)(this->exactDelay.getFloatValue() - 0.5);		//Truncate the exactDelay variable, since we cant perform fractional steps
			}
		}
		else												//If the motor is currently rotating the same direction as the desired direction
		{
			if(curVel > this->velocity)						//If current velocity is greater than desired velocity
			{
				this->state = INITDECEL;						//We need to decelerate the motor to desired velocity
				this->initialDecelSteps = (uint32_t)(((this->velocity*this->velocity) - (curVel*curVel))/(-2.0*this->acceleration));		//Number of steps to bring the motor down from current speed to max speed (S = (V^2 - V0^2)/(2*-a)))
				this->accelSteps = 0;						//No acceleration phase is needed
			}

			else if(curVel < this->velocity)					//If the current velocity is less than the desired velocity
			{
				this->state = ACCEL;							//Start accelerating
				this->accelSteps = (uint32_t)(((this->velocity*this->velocity) - (curVel*curVel))/(2.0*this->acceleration));	//Number of Steps needed to accelerate from current velocity to full speed
			}

			else											//If motor is currently running at desired speed
			{
				this->state = CRUISE;						//We should just run at cruise speed
			}
		}
	}

	else																						//If motor is currently stopped (state = STOP)
	{
		this->direction = dir;
		this->state = ACCEL;																	//Start accelerating
		if(dir)																	//Set the motor direction pin to the desired setting
		{
			PORTD |= (1 << 7);
		}
		else
		{
			PORTD &= ~(1 << 7);
		}
		this->accelSteps = (velocity*velocity)/(2.0*acceleration);								//Number of steps to bring the motor to max speed (S = (V^2 - V0^2)/(2*a)))
		
		this->exactDelay.setValue(INTFREQ/sqrt(2.0*this->acceleration));	//number of interrupts before the first step should be performed.
		
		if(this->exactDelay.getFloatValue() > 65535.0)
		{
			this->delay = 0xFFFF;
		}
		else
		{
			this->delay = (uint16_t)(this->exactDelay.getFloatValue() - 0.5);		//Truncate the exactDelay variable, since we cant perform fractional steps
		}
	}
	
	this->startTimer();																			//start timer so we can perform steps
	this->enableMotor();																			//Enable motor
}

void uStepper::moveSteps(uint32_t steps, bool dir, bool holdMode)
{
	float curVel;

	if(this->dropIn)
	{
		return;		//Drop in feature is activated. just return since this function makes no sense with drop in activated!
	}

	this->stopTimer();					//Stop interrupt timer so we dont fuck stuff up !
	steps--;
	this->direction = dir;				//Set direction variable to the desired direction of rotation for the interrupt routine
	this->hold = holdMode;				//Set the hold variable to desired hold mode (block motor or release motor after end movement) for the interrupt routine
	this->totalSteps = steps;			//Load the desired number of steps into the totalSteps variable for the interrupt routine
	this->continous = 0;				//Set continous variable to 0, since the motor should not run continous

	if(state != STOP)					//if the motor is currently running and we want to move the opposite direction, we need to decelerate in order to change direction.
	{
		curVel = INTFREQ/this->exactDelay.getFloatValue();								//Use this to calculate current velocity

		if(dir != digitalRead(DIR))									//If current direction is different from desired direction
		{
			this->state = INITDECEL;									//We should decelerate the motor to full stop
			this->initialDecelSteps = (uint32_t)((curVel*curVel)/(2.0*this->acceleration));		//the amount of steps needed to bring the motor to full stop. (S = (V^2 - V0^2)/(2*-a)))
			this->accelSteps = (uint32_t)((this->velocity * this->velocity)/(2.0*this->acceleration));									//Number of steps to bring the motor to max speed (S = (V^2 - V0^2)/(2*a)))
			this->totalSteps += this->initialDecelSteps;				//Add the steps used for initial deceleration to the totalSteps variable, since we moved this number of steps, passed the initial position, and therefore need to move this amount of steps extra, in the desired direction

			if(this->accelSteps > (this->totalSteps >> 1))			//If we need to accelerate for longer than half of the total steps, we need to start decelerating before we reach max speed
			{
				this->accelSteps = this->decelSteps = (this->totalSteps >> 1);	//Accelerate and decelerate for the same amount of steps (half the total steps)
				this->accelSteps += this->totalSteps - this->accelSteps - this->decelSteps;				//If there are still a step left to perform, due to rounding errors, do this step as an acceleration step	
			}
			else
			{
				this->decelSteps = this->accelSteps;					//If top speed is reached before half the total steps are performed, deceleration period should be same length as acceleration period
				this->cruiseSteps = this->totalSteps - this->accelSteps - this->decelSteps; 			//Perform remaining steps, as cruise steps
			}

			this->exactDelay.setValue(INTFREQ/sqrt((curVel*curVel) + 2.0*this->acceleration));	//number of interrupts before the first step should be performed.

			if(this->exactDelay.getFloatValue() >= 65535.5)
			{
				this->delay = 0xFFFF;
			}
			else
			{
				this->delay = (uint16_t)(this->exactDelay.getFloatValue() - 0.5);		//Truncate the exactDelay variable, since we cant perform fractional steps
			}
		}
		else							//If the motor is currently rotating the same direction as desired, we dont necessarily need to decelerate
		{
			if(curVel > this->velocity)	//If current velocity is greater than desired velocity
			{
				this->state = INITDECEL;	//We need to decelerate the motor to desired velocity
				this->initialDecelSteps = (uint32_t)(((this->velocity*this->velocity) - (curVel*curVel))/(-2.0*this->acceleration));		//Number of steps to bring the motor down from current speed to max speed (S = (V^2 - V0^2)/(2*-a)))
				this->accelSteps = 0;	//No acceleration phase is needed
				this->decelSteps = (uint32_t)((this->velocity*this->velocity)/(2.0*this->acceleration));	//Number of steps needed to decelerate the motor from top speed to full stop
				this->exactDelay.setValue((INTFREQ/sqrt((curVel*curVel) + 2*this->acceleration)));

				if(this->totalSteps <= (this->initialDecelSteps + this->decelSteps))
				{
					this->cruiseSteps = 0;
				}
				else
				{
					this->cruiseSteps = steps - this->initialDecelSteps - this->decelSteps;					//Perform remaining steps as cruise steps
				}

				
			}

			else if(curVel < this->velocity)	//If current velocity is less than desired velocity
			{
				this->state = ACCEL;			//Start accelerating
				this->accelSteps = (uint32_t)(((this->velocity*this->velocity) - (curVel*curVel))/(2.0*this->acceleration));	//Number of Steps needed to accelerate from current velocity to full speed

				if(this->accelSteps > (this->totalSteps >> 1))			//If we need to accelerate for longer than half of the total steps, we need to start decelerating before we reach max speed
				{
					this->accelSteps = this->decelSteps = (this->totalSteps >> 1);	//Accelerate and decelerate for the same amount of steps (half the total steps)
					this->accelSteps += this->totalSteps - this->accelSteps - this->decelSteps;				//If there are still a step left to perform, due to rounding errors, do this step as an acceleration step	
					this->cruiseSteps = 0;
				}
				else
				{
					this->decelSteps = this->accelSteps;					//If top speed is reached before half the total steps are performed, deceleration period should be same length as acceleration period
					this->cruiseSteps = this->totalSteps - this->accelSteps - this->decelSteps; 			//Perform remaining steps, as cruise steps
				}

				this->cruiseSteps = steps - this->accelSteps - this->decelSteps;	//Perform remaining steps as cruise steps
				this->initialDecelSteps = 0;								//No initial deceleration phase needed
			}

			else						//If current velocity is equal to desired velocity
			{
				this->state = CRUISE;	//We are already at desired speed, therefore we start at cruise phase
				this->decelSteps = (uint32_t)((this->velocity*this->velocity)/(2.0*this->acceleration));	//Number of steps needed to decelerate the motor from top speed to full stop
				this->accelSteps = 0;	//No acceleration phase needed
				this->initialDecelSteps = 0;		//No initial deceleration phase needed

				if(this->decelSteps >= this->totalSteps)
				{
					this->cruiseSteps = 0;
				}
				else
				{
					this->cruiseSteps = steps - this->decelSteps;	//Perform remaining steps as cruise steps
				}
			}
		}
	}
	
	else								//If motor is currently at full stop (state = STOP)
	{
		if(dir)																	//Set the motor direction pin to the desired setting
		{
			PORTD |= (1 << 7);
		}
		else
		{
			PORTD &= ~(1 << 7);
		}
		this->state = ACCEL;
		this->accelSteps = (uint32_t)((this->velocity * this->velocity)/(2.0*this->acceleration));	//Number of steps to bring the motor to max speed (S = (V^2 - V0^2)/(2*a)))
		this->initialDecelSteps = 0;		//No initial deceleration phase needed

		if(this->accelSteps > (steps >> 1))	//If we need to accelerate for longer than half of the total steps, we need to start decelerating before we reach max speed
		{
			this->cruiseSteps = 0; 		//No cruise phase needed
			this->accelSteps = this->decelSteps = (steps >> 1);				//Accelerate and decelerate for the same amount of steps (half the total steps)
			this->accelSteps += steps - this->accelSteps - this->decelSteps;	//if there are still a step left to perform, due to rounding errors, do this step as an acceleration step	
		}

		else								
		{
			this->decelSteps = this->accelSteps;	//If top speed is reached before half the total steps are performed, deceleration period should be same length as acceleration period
			this->cruiseSteps = steps - this->accelSteps - this->decelSteps;	//Perform remaining steps as cruise steps
		}
		this->exactDelay.setValue(INTFREQ/sqrt(2.0*this->acceleration));	//number of interrupts before the first step should be performed.

		if(this->exactDelay.getFloatValue() > 65535.0)
		{
			this->delay = 0xFFFF;
		}
		else
		{
			this->delay = (uint16_t)(this->exactDelay.getFloatValue() - 0.5);		//Truncate the exactDelay variable, since we cant perform fractional steps
		}
	}

	this->startTimer();									//start timer so we can perform steps
	this->enableMotor();									//Enable motor driver
}

void uStepper::hardStop(bool holdMode)
{
	if(this->dropIn)
	{
		return;		//Drop in feature is activated. just return since this function makes no sense with drop in activated!
	}

	this->stopTimer();			//Stop interrupt timer, since we shouldn't perform more steps
	this->hold = holdMode;

	if(state != STOP)
	{
		this->state = STOP;			//Set current state to STOP

		this->startTimer();
	}

	else
	{
		if(holdMode == SOFT)
		{
			this->disableMotor();
		}
		
		else if (holdMode == HARD)
		{
			this->enableMotor();
		}
	}
}

void uStepper::softStop(bool holdMode)
{
	float curVel;

	if(this->dropIn)
	{
		return;		//Drop in feature is activated. just return since this function makes no sense with drop in activated!
	}

	this->stopTimer();			//Stop interrupt timer, since we shouldn't perform more steps
	this->hold = holdMode;		

	if(state != STOP)
	{
		curVel = INTFREQ/this->exactDelay.getFloatValue();								//Use this to calculate current velocity

		this->decelSteps = (uint32_t)((curVel*curVel)/(2.0*this->acceleration));		//Number of steps to bring the motor down from current speed to max speed (S = (V^2 - V0^2)/(2*-a)))	
		this->accelSteps = this->initialDecelSteps = this->cruiseSteps = 0;	//Reset amount of steps in the different phases	
		this->state = DECEL;

		this->exactDelay.setValue(INTFREQ/sqrt(2.0*this->acceleration));	//number of interrupts before the first step should be performed.

		if(this->exactDelay.getFloatValue() > 65535.0)
		{
			this->delay = 0xFFFF;
		}
		else
		{
			this->delay = (uint16_t)(this->exactDelay.getFloatValue() - 0.5);		//Truncate the exactDelay variable, since we cant perform fractional steps
		}

		this->startTimer();
	}

	else
	{
		if(holdMode == SOFT)
		{
			this->disableMotor();
		}
		
		else if (holdMode == HARD)
		{
			this->enableMotor();
		}
	}
}

void uStepper::setup(	bool mode, 
						uint8_t microStepping, 
						float faultTolerance,
						float faultHysteresis, 
						float pTerm, 
						float iTerm, 
						float dterm)
{
	this->dropIn = mode;
	if(mode)
	{
		//Set Enable, Step and Dir signal pins from 3dPrinter controller as inputs
		pinMode(2,INPUT);		
		pinMode(3,INPUT);
		pinMode(4,INPUT);
		//Enable internal pull-up resistors on the above pins
		digitalWrite(2,HIGH);
		digitalWrite(3,HIGH);
		digitalWrite(4,HIGH);

		this->stepConversion = ((float)(200*microStepping))/4096.0;	//Calculate conversion coefficient from raw encoder data, to actual moved steps
		this->tolerance = faultTolerance;		//Number of steps missed before controller kicks in
		this->hysteresis = faultHysteresis;
		attachInterrupt(digitalPinToInterrupt(2), interrupt0, CHANGE);
		attachInterrupt(digitalPinToInterrupt(3), interrupt1, FALLING);
		
		//Scale supplied controller coefficents. This is done to enable the user to use easier to manage numbers for these coefficients.
		this->pTerm = pTerm/10000.0;	  
		this->iTerm = iTerm/10000.0;
		this->dTerm = dTerm/10000.0;
	}

	TCCR2B &= ~((1 << CS20) | (1 << CS21) | (1 << CS22) | (1 << WGM22));
	TCCR2A &= ~((1 << WGM20) | (1 << WGM21));
	TCCR2B |= (1 << CS21)| (1 << WGM22);				//Enable timer with prescaler 8 - interrupt base frequency ~ 2MHz
	TCCR2A |= (1 << WGM21) | (1 << WGM20);				//Switch timer 2 to Fast PWM mode, to enable adjustment of interrupt frequency, while being able to use PWM
	OCR2A = 70;											//Change top value to 70 in order to obtain an interrupt frequency of 28.571kHz
	this->enableMotor();
	this->encoder.setup(mode);
	
}

void uStepper::startTimer(void)
{
	while(TCNT2);						//Wait for timer to overflow, to ensure correct timing.
	TIFR2 |= (1 << OCF2A);				//Clear compare match interrupt flag, if it is set.
	TIMSK2 |= (1 << OCIE2A);			//Enable compare match interrupt

	sei();
}

void uStepper::stopTimer(void)
{
	TIMSK2 &= ~(1 << OCIE2A);			//disable compare match interrupt
}

void uStepper::enableMotor(void)
{
	PORTB &= ~(1 << 0);				//Enable motor driver
}

void uStepper::disableMotor(void)
{
	PORTB |= (1 << 0);			//Disable motor driver
}

bool uStepper::getCurrentDirection(void)
{
	return this->direction;
}

bool uStepper::getMotorState(void)
{
	if(this->state != STOP)
	{
		return 1;		//Motor running
	}

	return 0;			//Motor not running
}

int64_t uStepper::getStepsSinceReset(void)
{
	if(this->direction == CW)
	{
		return this->stepsSinceReset + this->currentStep;
	}
	else
	{
		return this->stepsSinceReset - this->currentStep;
	}
}

void uStepper::pwmD8(float duty)
{
	pinMode(8,OUTPUT);
	TCCR1A |= (1 << COM1B1);

	if(duty > 100.0)
	{
		duty = 100.0;
	}
	else if(duty < 0.0)
	{
		duty = 0.0;
	}

	duty *= 160.0;

	OCR1B = (uint16_t)(duty + 0.5);
}

void uStepper::pwmD3(float duty)
{
	pinMode(3,OUTPUT);
	TCCR2A |= (1 << COM2B1);

	if(duty > 100.0)
	{
		duty = 100.0;
	}
	else if(duty < 0.0)
	{
		duty = 0.0;
	}

	duty *= 0.7;

	OCR2B = (uint16_t)(duty + 0.5);
}

void uStepper::pidDropIn(float error, uint32_t speed)
{
	static float oldError = 0.0;
	float integral;
	float output = 1.0;
	static float accumError = 0.0;
	float limit;

	if(error < -pointer->tolerance)
	{
		cli(); //Do Atomic copy, in order to not fuck up variables
			pointer->control = (int32_t)error;	//Move current error into control variable, for Timer2 and int0 routines to decide who should provide steps
		sei();

		error = -error;		//error variable should always be positive when calculating controller output
	
		integral = error*pointer->iTerm;	//Multiply current error by integral term
		accumError += integral;				//And accumulate, to get integral action
		
		//The output of each PID part, should be subtracted from the output variable.
		//This is true, since in case of no error the motor should run at a higher speed
		//to catch up, and since the speed variable contains the number of microseconds
		//between each step, the we need to multiply with a number < 1 to increase speed
		output -= pointer->pTerm*error;		
		output -= accumError;
		output -= pointer->dTerm*(error - oldError);

		oldError = error;		//Save current error for next sample, for use in differential part

		PORTD &= ~(1 << 7);		//change direction to CW
		
		output *= (float)speed;	//Calculate stepSpeed

		if(output < 54.0)		//If stepSpeed is lower than possible
		{
			output = 54.0;		//Set stepSpeed to lowest possible
			accumError -= integral;	//and subtract current integral part from accumerror (anti-windup)
		}
		
		cli();		//Atomic copy
			pointer->delay = (uint16_t)((output*INTPIDDELAYCONSTANT) - 0.5);	//calculate Number of interrupt Timer 2 should do before generating step pulse
		sei();

		pointer->startTimer();		//Start Timer 2
		PORTB &= ~(1 << 0);			//enable motor
	}

	else if(error > pointer->tolerance)
	{
		pointer->control = (int32_t)error;		
		
		integral = error*pointer->iTerm;
		accumError += integral;

		output -= pointer->pTerm*error;
		output -= accumError;
		output -= pointer->dTerm*(error - oldError);

		oldError = error;
		
		PORTD |= (1 << 7);				//change direction to CCW
		output *= (float)speed;
		
		if(output < 54.0)
		{
			output = 54.0;
			accumError -= integral;
		}
		
		cli();
			pointer->delay = (uint16_t)((output*INTPIDDELAYCONSTANT) - 0.5);
		sei();

		pointer->startTimer();	
		PORTB &= ~(1 << 0);
	}
	
	else
	{
		if(error >= -pointer->hysteresis && error <= pointer->hysteresis)	//If error is less than 1 step
		{
			pointer->control = 0;			//Set control variable to 0, in order to make sure int0 routine generates step pulses
			accumError = 0.0;				//Clear accumerror
			PORTB |= (PIND & 0x04) >> 2;	//Set enable pin to whatever is demanded by the 3Dprinter controller
			pointer->stopTimer();			//Stop timer 2
		}
	}	
}

void i2cMaster::cmd(uint8_t cmd)
{
	uint16_t i = 0;
	// send command
	TWCR = cmd;
	// wait for command to complete
	while (!(TWCR & (1 << TWINT)));
	
	// save status bits
	status = TWSR & 0xF8;	
}

bool i2cMaster::read(uint8_t slaveAddr, uint8_t regAddr, uint8_t numOfBytes, uint8_t *data)
{
	uint8_t i, buff[numOfBytes];

	TIMSK1 &= ~(1 << OCIE1A);

	I2C.start(slaveAddr, WRITE);

	I2C.writeByte(regAddr);

	I2C.restart(slaveAddr, READ);

	for(i = 0; i < (numOfBytes - 1); i++)
	{
		I2C.readByte(ACK, &data[i]);
	}

	I2C.readByte(NACK, &data[numOfBytes-1]);

	I2C.stop();

	TIMSK1 |= (1 << OCIE1A);
	
	return 1; 
}

bool i2cMaster::write(uint8_t slaveAddr, uint8_t regAddr, uint8_t numOfBytes, uint8_t *data)
{
	uint8_t i;

	TIMSK1 &= ~(1 << OCIE1A);

	I2C.start(slaveAddr, WRITE);
	I2C.writeByte(regAddr);
	
	for(i = 0; i < numOfBytes; i++)
	{
		I2C.writeByte(*(data + i));
	}
	I2C.stop();

	TIMSK1 |= (1 << OCIE1A);

	return 1;
}

bool i2cMaster::readByte(bool ack, uint8_t *data)
{
	if(ack)
	{
		this->cmd((1 << TWINT) | (1 << TWEN) | (1 << TWEA));
	}
	
	else
	{
		this->cmd((1 << TWINT) | (1 << TWEN));
	}

	*data = TWDR;

	return 1;
}

bool i2cMaster::start(uint8_t addr, bool RW)
{
	// send START condition
	this->cmd((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));

	if (this->getStatus() != START && this->getStatus() != REPSTART) 
	{
		return false;
	}

	// send device address and direction
	TWDR = (addr << 1) | RW;
	this->cmd((1 << TWINT) | (1 << TWEN));
	
	if (RW == READ) 
	{
		return this->getStatus() == RXADDRACK;
	} 

	else 
	{
		return this->getStatus() == TXADDRACK;
	}
}

bool i2cMaster::restart(uint8_t addr, bool RW)
{
	return this->start(addr, RW);
}

bool i2cMaster::writeByte(uint8_t data)
{
	TWDR = data;

	this->cmd((1 << TWINT) | (1 << TWEN));

	return this->getStatus() == TXDATAACK;
}

bool i2cMaster::stop(void)
{
	uint16_t i = 0;
	//	issue stop condition
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

	// wait until stop condition is executed and bus released
	while (TWCR & (1 << TWSTO));

	status = I2CFREE;

	return 1;
}

uint8_t i2cMaster::getStatus(void)
{
	return status;
}

void i2cMaster::begin(void)
{
	// set bit rate register to 12 to obtain 400kHz scl frequency (in combination with no prescaling!)
	TWBR = 12;
	// no prescaler
	TWSR &= 0xFC;
}

i2cMaster::i2cMaster(void)
{

}