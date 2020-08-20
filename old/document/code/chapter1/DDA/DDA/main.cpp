#include "Graphics.h"
#include "Point2.h"
#include <stdio.h>
void DDA(Graphics& gp,Point2& A, Point2& B)
{
	double k = (A.Y - B.Y) / (A.X - B.X);//如果A.X-B.X==0的话，使用int会出问题，会报除0异常，但是double有inf(infinite)和nan(not a number)，并且1/0=inf,1/inf=0,1/inf=nan，inf大于任何有效数字，-inf小于任何有效数字，所以这里可以不管分母为0的情况
	Point2 tmp;
	if (k > 1 || k < -1)//按照Y步进
	{
		if (A.Y > B.Y)////使A的y小于B的y
		{
			tmp = A;
			A = B;
			B = tmp;
		}
		int y = (int)A.Y;
		double x = A.X;
		for (; y <= B.Y; y++)
		{
			gp.setPixel((int)x, y, WHITE);
			x += 1 / k;
		}
	}
	else//按照X步进
	{
		if (A.X > B.X)////使A的x小于B的x
		{
			tmp = A;
			A = B;
			B = tmp;
		}
		int x = (int)A.X;
		double y = A.Y;
		for (; x <= B.X; x++)
		{
			gp.setPixel(x, (int)y, WHITE);
			y += k;
		}
	}
}

int main()
{
	Graphics gp(640,480);//创建一个画布，里面提供了画点函数
	Point2 ps[2] = { {-10,400},{10,0} };//创建两个点
	DDA(gp,ps[0], ps[1]);//执行DDA算法
	getchar();//等待任意键退出
}