#ifndef _Point2
#define _Point2
class Point2//复制构造和赋值用浅拷贝目前够了
{
public:
	double value[2] = { 0,0 };//value[0]表示X,value[1]表示Y,value[2]表示Z,value[3]表示W
	Point2();
	Point2(double x, double y);
	~Point2();
};
#endif // !_POINT

