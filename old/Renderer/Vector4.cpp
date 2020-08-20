#include "Vector4.h"
#include <math.h>


Vector4::Vector4()
{
}

double Vector4::dot(Vector4& a, Vector4& b)
{
	double result = 0.0;
	for (int i = 0; i < 4; i++)
	{
		result += a.value[i] * b.value[i];
	}
	return result;
}

double Vector4::dot(Vector4& a, Vector3& b)
{
	double result = 0.0;
	for (int i = 0; i < 3; i++)
	{
		result += a.value[i] * b.value[i];
	}
	return result;
}

double Vector4::dot(Vector3& a, Vector4& b)
{
	double result = 0.0;
	for (int i = 0; i < 3; i++)
	{
		result += a.value[i] * b.value[i];
	}
	return result;
}

Vector4::Vector4(double x, double y, double z, double w)
{
	value[0] = x;
	value[1] = y;
	value[2] = z;
	value[3] = w;
}

void Vector4::Normalize()
{
	double mod = Mod();
	value[0] = value[0] / mod;
	value[1] = value[1] / mod;
	value[2] = value[2] / mod;
	value[3] = value[3] / mod;
}

double Vector4::Mod()
{
	return sqrt(value[0] * value[0] + value[1] * value[1] + value[2] * value[2] + value[3] * value[3]);
}

Vector4::~Vector4()
{
}
