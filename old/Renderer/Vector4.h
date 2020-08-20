#ifndef _Vector4
#define _Vector4
#include "Vector.h"
#include "Vector3.h"
class Vector4 :
	public Vector
{
public:
	double value[4] = { 0 };//value[0]表示X,value[1]表示Y,value[2]表示Z,value[3]表示W
	Vector4();
	static double dot(Vector4& a, Vector4& b);
	static double dot(Vector4& a, Vector3& b);
	static double dot(Vector3& a, Vector4& b);
	Vector4(double x, double y, double z, double w);
	virtual void Normalize();
	virtual double Mod();
	~Vector4();
};
#endif // !_Vector4

