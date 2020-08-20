#include "Vector2.h"
#include <math.h>

Vector2::Vector2(double x, double y)
{
	value[0] = x;
	value[1] = y;
}

double Vector2::dot(Vector2& a, Vector2& b)
{
	double result = 0.0;
	for (int i = 0; i < 2; i++)
	{
		result += a.value[i] * b.value[i];
	}
	return result;
}

Vector2::~Vector2()
{
}

double Vector2::Mod()
{
	return sqrt(value[0] * value[0] + value[1] * value[1]);
}

void Vector2::Normalize()
{
	double mod = Mod();
	value[0] = value[0] / mod;
	value[1] = value[1] / mod;
}
