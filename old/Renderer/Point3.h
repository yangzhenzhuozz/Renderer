#ifndef _Point3
#define _Point3
class Point3//复制构造和赋值用浅拷贝目前够了
{
public:
	double value[3] = { 0,0,0 };
	Point3();
	Point3(double x, double y, double z);
	~Point3();
};
#endif // !_Point3



