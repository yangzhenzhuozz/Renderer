#ifndef _Vector2
#define _Vector2
#include "Vector.h"
class Vector2 :public Vector
{
public:
	double value[2];//value[0]表示X,value[1]表示Y,value[2]表示Z,value[3]表示W
	Vector2(double x, double y);
	static double dot(Vector2& a, Vector2& b);
	virtual double Mod();
	virtual void Normalize();//单位化
	~Vector2();
};
#endif // !_Vector2

