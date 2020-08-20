#ifndef _Point4
#define _Point4
class Point4//复制构造和赋值用浅拷贝目前够了
{
public:
	double value[4] = { 0,0,0,0 };//value[0]表示X,value[1]表示Y,value[2]表示Z,value[3]表示W
	Point4();
	Point4(double x, double y, double z, double w);
	~Point4();
};
#endif // !_Point4

