/********************************************************************************************
* 	 	File: 		uStepper.cpp 															*
*		Version:    0.4.2                                           						*
*      	date: 		August 3rd, 2016	                                    				*
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

volatile uint16_t i = 0;
volatile int32_t stepCnt = 0;
volatile bool control = 0;

extern "C" {

	void INT1_vect(void)
	{
		if((PIND & (0x20)))			//CCW
		{
			if(control == 0)
			{
				PORTD |= (1 << 7);	//Set dir to CCW

				PORTD |= (1 << 4);		//generate step pulse
				delayMicroseconds(1);	//wait a small time
				PORTD &= ~(1 << 4);		//pull step pin low again
				pointer->stepsSinceReset--;
			}			
			stepCnt--;				//DIR is set to CCW, therefore we subtract 1 step from step count (negative values = number of steps in CCW direction from initial postion)
		}
		else						//CW
		{
			if(control == 0)
			{
				
				PORTD &= ~(1 << 7);	//Set dir to CW

				PORTD |= (1 << 4);		//generate step pulse
				delayMicroseconds(1);	//wait a small time
				PORTD &= ~(1 << 4);		//pull step pin low again
				pointer->stepsSinceReset++;
			}

			stepCnt++;				//DIR is set to CW, therefore we add 1 step to step count (positive values = number of steps in CW direction from initial postion)	
		}
		pointer->stepsInLoop++;
	}

	void INT0_vect(void)
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

		if(i < pointer->delay)		//This value defines the speed at which the motor rotates when compensating for lost steps. This value should be tweaked to the application
		{
			i++;
		}
		else
		{	
			PORTD |= (1 << 4);
			delayMicroseconds(1);
			PORTD &= ~(1 << 4);	
			i = 0;
			if((PIND & (0x20)))
			{
				pointer->stepsSinceReset--;
			}
			else
			{
				pointer->stepsSinceReset++;
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
	
	void TIMER1_COMPA_vect(void)
	{
		float error;
		float deltaAngle, newSpeed;
		static float curAngle, oldAngle = 0.0, deltaSpeedAngle = 0.0, oldSpeed = 0.0;
		static uint8_t loops = 0, stepSpeedCnt = 0;
		static float stepSpeed = 0;
		static float revolutions = 0.0;
		uint8_t data[2];

		sei();
		if(I2C.getStatus() != I2CFREE)
		{
			return;
		}

		I2C.read(ENCODERADDR, ANGLE, 2, data);
		curAngle = (float)((((uint16_t)data[0]) << 8 ) | (uint16_t)data[1])*0.087890625;
		pointer->encoder.angle = curAngle;
		curAngle -= pointer->encoder.encoderOffset;

		curAngle = fmod(curAngle + 360.0, 360.0);

		deltaAngle = (oldAngle - curAngle);
		//count number of revolutions, on angle overflow
		if(deltaAngle < -180.0)
		{
			revolutions -= 1.0;
			deltaAngle += 360;
		}
		
		else if(deltaAngle > 180.0)
		{
			revolutions += 1.0;
			deltaAngle -= 360.0;
		}
		
		if( loops < 10)
		{
			loops++;
			deltaSpeedAngle += deltaAngle;
		}
		else
		{
			newSpeed = (deltaSpeedAngle*ENCODERSPEEDCONSTANT);
			newSpeed = oldSpeed*ALPHA + newSpeed*BETA;					//Filter
			oldSpeed = newSpeed;
			pointer->encoder.curSpeed = newSpeed;
			loops = 0;
			deltaSpeedAngle = 0.0;
		}

		if(pointer->stepsInLoop)
		{
			stepSpeed = (float)pointer->stepsInLoop;
			stepSpeed *= ENCODERINTFREQ/(float)stepSpeedCnt;
			pointer->stepsInLoop = 0;
			stepSpeedCnt = 0;
		}
		
		else
		{
			stepSpeedCnt++;
		}

		pointer->encoder.angleMoved = curAngle + (360.0*revolutions);
		oldAngle = curAngle;
		pointer->encoder.oldAngle = curAngle;

		cli();
			error = (float)stepCnt;  //atomic read to ensure no fuckups happen from the external interrupt routine
		sei();
		error *= pointer->stepResolution;
		error -= pointer->encoder.angleMoved;

		if(pointer->dropIn)
		{
			pointer->pidDropIn(error, (uint16_t)stepSpeed);
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
{/*
	float deltaAngle, angle;

	TIMSK1 &= ~(1 << OCIE1A);
	
	angle = this->getAngle() - this->encoderOffset;
	

	if(angle < 0.0)
	{
		angle += 360.0;
	}

	deltaAngle = this->oldAngle - angle;

	if(deltaAngle < -180.0)
	{
		pointer->encoder.angleMoved += (deltaAngle + 360.0); 
	}

	else if(deltaAngle > 180.0)
	{
		pointer->encoder.angleMoved -= (360.0 - deltaAngle); 
	}

	else
	{
		this->angleMoved += deltaAngle;
	}

	this->oldAngle = angle;
	
	TIMSK1 |= (1 << OCIE1A);*/

	return this->angleMoved;
}

float uStepperEncoder::getSpeed(void)
{
	return this->curSpeed;
}

void uStepperEncoder::setup(void)
{
	uint8_t data[2];
	TCCR1A = 0;
	TCNT1 = 0;
	OCR1A = 16000;
	TIFR1 = 0;
	TIMSK1 = (1 << OCIE1A);
	TCCR1B = (1 << WGM12) | (1 << CS10);
	I2C.read(ENCODERADDR, ANGLE, 2, data);
	this->encoderOffset = (float)((((uint16_t)data[0]) << 8 ) | (uint16_t)data[1])*0.087890625;

	this->oldAngle = 0.0;
	this->angleMoved = 0.0;

	sei();

}

void uStepperEncoder::setHome(void)
{
	cli();
	uint8_t data[2];
	I2C.read(ENCODERADDR, ANGLE, 2, data);
	this->encoderOffset = (float)((((uint16_t)data[0]) << 8 ) | (uint16_t)data[1])*0.087890625;

	this->angle = 0.0;
	this->oldAngle = 0.0;
	this->angleMoved = 0.0;
	sei();
}

float uStepperEncoder::getAngle()
{
	return this->angle;
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
{/*
	Serial.print(this->exactDelay.getFloatValue(),15);
	Serial.print("\t");
	Serial.print(this->delay);
	Serial.print("\t");
	Serial.print((uint8_t)((this->exactDelay.value >> 48) & 0x00000000000000FF),HEX);
	Serial.print(" ");
	Serial.print((uint8_t)((this->exactDelay.value >> 40) & 0x00000000000000FF),HEX);
	Serial.print(" ");
	Serial.print((uint8_t)((this->exactDelay.value >> 32) & 0x00000000000000FF),HEX);
	Serial.print(" ");
	Serial.print((uint8_t)((this->exactDelay.value >> 24) & 0x00000000000000FF),HEX);
	Serial.print(" ");
	Serial.print((uint8_t)((this->exactDelay.value >> 16) & 0x00000000000000FF),HEX);
	Serial.print(" ");
	Serial.print((uint8_t)((this->exactDelay.value >> 8) & 0x00000000000000FF),HEX);
	Serial.print(" ");
	Serial.print((uint8_t)((this->exactDelay.value) & 0x00000000000000FF),HEX);
	Serial.print("\t");
	Serial.print(this->delay);
	Serial.print("\t");
	Serial.println(this->totalSteps);*/
	/*cli();
	float tmp=this->exactDelay.getFloatValue();
	sei();
	return tmp;*/
	return this->acceleration;
}

void uStepper::setMaxVelocity(float vel)
{
	if(vel < 0.5005)
	{
		this->velocity = 0.5005;			//Limit velocity in order to not overflow delay variable
	}

	else if(vel > 32000.0)
	{
		this->velocity = 32000.0;			//limit velocity in order to not underflow delay variable
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
	this->direction = dir;
	

	if(state != STOP)										//if the motor is currently running and we want to move the opposite direction, we need to decelerate in order to change direction.
	{
		curVel = INTFREQ/this->exactDelay.getFloatValue();								//Use this to calculate current velocity

		if(dir != digitalRead(DIR))							//If motor is currently running the opposite direction as desired
		{
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

void uStepper::setup(bool mode, uint8_t microStepping, float faultSpeed, uint32_t faultTolerance)
{
	this->dropIn = mode;
	if(mode)
	{
		pinMode(2,INPUT);
		pinMode(3,INPUT);
		pinMode(4,INPUT);
		digitalWrite(2,HIGH);
		digitalWrite(3,HIGH);
		digitalWrite(4,HIGH);
		this->stepResolution = 360.0/((float)(200*microStepping));
		this->tolerance = ((float)faultTolerance)*this->stepResolution;
		this->faultStepDelay = (uint16_t)((INTFREQ/faultSpeed) - 1);
		EICRA = 0x0D;		//int0 generates interrupt on any change and int1 generates interrupt on rising edge
		EIMSK = 0x03;		//enable int0 and int1 interrupt requests
	}

	TCCR2B &= ~((1 << CS20) | (1 << CS21) | (1 << CS22) | (1 << WGM22));
	TCCR2A &= ~((1 << WGM20) | (1 << WGM21));
	TCCR2B |= (1 << CS21);				//Enable timer with prescaler 8. interrupt base frequency ~ 7.8125kHz
	TCCR2A |= (1 << WGM21);				//Switch timer 2 to CTC mode, to adjust interrupt frequency
	OCR2A = 70;							//Change top value to 60 in order to obtain an interrupt frequency of 33.333kHz
	this->encoder.setup();
}

void uStepper::startTimer(void)
{
	TCNT2 = 0;							//Clear counter value, to make sure we get correct timing
	TIFR2 |= (1 << OCF2A);				//Clear compare match interrupt flag, if it is set.
	TIMSK2 |= (1 << OCIE2A);			//Enable compare match interrupt

	sei();
}

void uStepper::stopTimer(void)
{
	TIMSK2 &= ~(1 << OCF2A);			//disable compare match interrupt
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

void uStepper::pidDropIn(float error, uint16_t stepSpeed)
{
	static float oldError = 0.0;
	int32_t appliedStepError;
	float output = 0.0;
	static float accumError = 0.0;

	appliedStepError = stepCnt - (int32_t)pointer->stepsSinceReset;

	error -= (float)appliedStepError;

	if(!error)
	{
		control = 1;
		accumError += (float)error*ITERM;
		output = PTERM*error;
		output += accumError;
		output += DTERM*(error - oldError);
		oldError = error;
		if(error < 0)
		{
			PORTD |= (1 << 7);
			stepSpeed -= (int16_t)output;
			if(stepSpeed < 1050)
			{
				stepSpeed = 1050;
			}
			pointer->delay = (uint16_t)(INTFREQ/(float)stepSpeed);
			pointer->startTimer();	
		}
		
		else
		{
			PORTD &= ~(1 << 7);
			stepSpeed -= (int16_t)output;
			if(stepSpeed < 1050)
			{
				stepSpeed = 1050;
			}
			pointer->delay = (uint16_t)(INTFREQ/(float)stepSpeed);
			pointer->startTimer();	
		}
		PORTB &= ~(1 << 0);
	}

	else
	{
		control = 0;
		accumError = 0.0;
		PORTB |= (PIND & 0x04) >> 2;
		pointer->stopTimer();	
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