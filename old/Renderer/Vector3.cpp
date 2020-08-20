#include "Vector3.h"
#include <math.h>


Vector3::Vector3(Vector3& s, Vector3& d)
{
	value[0] = (d.value[0] - s.value[0]);
	value[1] = (d.value[1] - s.value[1]);
	value[2] = (d.value[2] - s.value[2]);
}

Vector3::Vector3(double x, double y, double z)
{
	value[0] = x;
	value[1] = y;
	value[2] = z;
}

Vector3::Vector3(Vector2& s, double z)
{
	value[0] = (s.value[0]);
	value[1] = (s.value[1]);
	value[2] = (z);
}

double Vector3::dot(Vector3& a, Vector3& b)
{
	double result = 0.0;
	for (int i = 0; i < 3; i++)
	{
		result += a.value[i] * b.value[i];
	}
	return result;
}

void Vector3::Normalize()
{
	double mod = Mod();
	value[0] = value[0] / mod;
	value[1] = value[1] / mod;
	value[2] = value[2] / mod;
}

double Vector3::Mod()
{
	return sqrt(value[0] * value[0] + value[1] * value[1] + value[2] * value[2]);
}

Vector3 Vector3::CrossProduct(Vector3& _a, Vector3& _b)
{
	double l = _a.value[0],
		m = _a.value[1],
		n = _a.value[2];
	double o = _b.value[0],
		p = _b.value[1],
		q = _b.value[2];
	/**叉乘公式如下
	 * |i ,j ,k |
	 * |ax,ay,az|
	 * |bx,by,bz|
	 * |c|=|a|*|b|*sin(θ)
	 */
	return Vector3(m * q - n * p, n * o - l * q, l * p - m * o);
}

Vector3::~Vector3()
{
}
