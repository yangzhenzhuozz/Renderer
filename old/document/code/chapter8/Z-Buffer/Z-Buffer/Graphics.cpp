#include "Graphics.h"

Graphics::Graphics(int w, int h):WIDTH(w), HEIGHT(h)
{
	initgraph(WIDTH, HEIGHT);//创建绘图窗口
	g_pBuf = GetImageBuffer(NULL);
}

void Graphics::setPixel(int x, int y, COLORREF c)
{
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)//如果画点区域不在画布范围内则放弃
	{
		return;
	}
	y = HEIGHT - 1 - y;//因为easyx的y值0点是最上面，所以将屏幕上下翻转到最下面
	g_pBuf[y * WIDTH + x] = BGR(c);
}

void Graphics::clear()
{
	cleardevice();
}

Graphics::~Graphics()
{
	closegraph();// 关闭绘图窗口
}
