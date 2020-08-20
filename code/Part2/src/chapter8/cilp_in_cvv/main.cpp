#include <GraphicsDevice.h>
#include <GraphicsLibrary.h>
#include "stdio.h"
class Texture
{
private:
	COLORREF img[2][2] = {
	{RGB(255,0,0),RGB(0,255,0)},
	{RGB(0,0,255),RGB(255,123,172)}
	};
	double width = 2, heigth = 2;
public:
	COLORREF texture2D(double u, double v)
	{
		int row = (int)(v * width);
		int column = (int)(u * width);
		row =_min(_max(0, row), (int)heigth -1);
		column =_min(_max(0, column), (int)width-1);
		return img[row][column];//因为u表示横轴，在我们这定义里面应该是最低维的数据，所以应该是img[v][u]表示纹素坐标
	}
};
Texture texture;

Point4 Persective(Point4& p, double l, double r, double b, double t, double n, double f)
{
	auto pn = p.Normalize();//先将齐次坐标变成三维坐标
	//执行透视变换
	double x = (2 * n * pn.X) / (r - l) - (r + l) / (r - l) * pn.Z;
	double y = (2 * n * pn.Y) / (t - b) - (t + b) / (t - b) * pn.Z;
	double z = (f + n) / (f - n) * pn.Z + (2 * n * f) / (n - f);
	double w = pn.Z;
	return Point4(x, y, z, w);
}

COLORREF FS(std::vector<double>& values)
{
	return texture.texture2D(values[0], values[1]);
};
int main()
{
	GraphicsDevice gd(640, 480);
	GraphicsLibrary gl(gd);

	gl.clean_depth(1000);//先将深度值设置为一个较大的值

	Point4 a1(-512, 384, 10, 1);
	Point4 b1(-512, -384, 10, 1);
	Point4 c1(512, -384, 4, 1);
	auto ta1 = Persective(a1, -320, 320, -240, 240, 5, 20);
	auto tb1 = Persective(b1, -320, 320, -240, 240, 5, 20);
	auto tc1 = Persective(c1, -320, 320, -240, 240, 5, 20);
	ta1.ValueArray = { 0,1 };
	tb1.ValueArray = { 0,0 };
	tc1.ValueArray = { 1,0 };
	gl.ClipAndDraw(ta1, tb1, tc1, 2, 5, FS);//绘制第一个三角形

	getchar();
	return 0;
}