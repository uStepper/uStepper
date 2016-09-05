#include "float2.h"

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