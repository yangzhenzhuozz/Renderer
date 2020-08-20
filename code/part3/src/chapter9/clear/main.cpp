#include <GraphicsDevice.h>
#include <GraphicsLibrary.h>
#include "stdio.h"

#include <iostream>
#include <ctime>
#include <thread>
std::clock_t start, end;


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
		row =_min(_max(0, row), (int)heigth - 1);
		column =_min(_max(0, column), (int)width - 1);
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
double near_face = 5;
void VS(double* vbo1, double* vbo2, Point4& gl_Vertex, std::vector<double>& varying)
{
	Point4 p(vbo1[0], vbo1[1], vbo1[2], 1);
	gl_Vertex = Persective(p, -320, 320, -240, 240, near_face, 20);
	varying[0] = vbo2[0];
	varying[1] = vbo2[1];
}
void FS(std::vector<double>& varying, COLORREF& gl_Color)
{
	gl_Color = texture.texture2D(varying[0], varying[1]);
};
int main()
{
	GraphicsDevice gd(640, 480);
	GraphicsLibrary gl(gd);

	double vbo1[] = {
		-512, 384, 10,
		-512, -384, 10,
		512, -384, 4,
	};
	double vbo2[] = {
		0,1,
		0,0,
		1,0,
	};
	gl.setVBO(vbo1, 3, vbo2, 2, 3);
	gl.setVaryingSize(2);
	gl.VertexShader = VS;
	gl.FragmentShader = FS;
	gl.FrontFace = _FrontFace_CCW;
	for (; near_face > 0;)
	{
		start = clock();
		gl.clear_depth(1.0);//将深度缓冲区中的所有值设置为1.0，因为我们在cvv裁剪中限定了far平面的深度值为1.0，再远的像素不再绘制
		gl.clear_color(RGB(0, 0, 0));//将所有像素值设置为黑色
		gl.draw();//渲染画面
		end = clock();
		double endtime = (double)((double)end - (double)start) / CLOCKS_PER_SEC;
		std::cout << "Total time:" << endtime * 1000 << "ms" << std::endl;	//ms为单位
		near_face -= 0.01;//将near平面深度值减小
		std::this_thread::sleep_for(std::chrono::milliseconds(50));//休眠50ms
	}
	getchar();
	return 0;
}