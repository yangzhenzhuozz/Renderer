#ifndef _Vector3
#define _Vector3
#include "Point3.h"
#include "Vector.h"
#include "Vector2.h"
class Vector3 :public Vector
{
public:
	double value[3];//value[0]表示X,value[1]表示Y,value[2]表示Z,value[3]表示W
	Vector3(Vector3& s, Vector3& d);//构造一个s指向d的向量
	Vector3(double x, double y, double z);//构造一个向量
	Vector3(Vector2& s, double z);//构造一个向量
	static double dot(Vector3& a, Vector3& b);
	virtual void Normalize();
	virtual double Mod();
	static Vector3 CrossProduct(Vector3& a, Vector3& b);//计算两个向量的叉积
	~Vector3();
};
#endif // !_Vector



