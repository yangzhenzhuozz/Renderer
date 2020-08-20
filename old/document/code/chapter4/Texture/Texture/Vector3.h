#ifndef _VECTOR3
#define _VECTOR3
class Vector3
{
public:
	double X, Y, Z;
	Vector3(double x = 0, double y = 0, double z = 0);
	static Vector3 cross(Vector3& a,Vector3& b);//计算向量的叉积
};
#endif // !_VECTOR3

