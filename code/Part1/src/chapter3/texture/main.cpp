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
		return img[(int)(v * width)][(int)(u * width)];//因为u表示横轴，在我们这定义里面应该是最低维的数据，所以应该是img[v][u]表示纹素坐标
	}
};
Texture texture;
COLORREF FS(std::vector<double>& values)
{
	return texture.texture2D(values[0],values[1]);
};
int main()
{
	GraphicsDevice gd(640, 480);
	GraphicsLibrary gl(gd);
	Point2 a(0, 0);
	Point2 b(480, 0);
	Point2 c(0, 480);
	a.ValueArray = { 0,0 };
	b.ValueArray = { 1,0 };
	c.ValueArray = { 0,1 };
	gl.MultipleInterpolationArrayIn2D(a, b, c, 2, FS);
	getchar();
	return 0;
}