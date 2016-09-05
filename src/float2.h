#ifndef _FLOAT2_H_
#define _FLOAT2_H_
#include <inttypes.h>
extern "C" void TIMER2_COMPA_vect(void);
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
		friend void TIMER2_COMPA_vect(void);
		
};
#endif