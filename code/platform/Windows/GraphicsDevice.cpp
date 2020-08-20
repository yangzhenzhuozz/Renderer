#include "GraphicsDevice.h"
GraphicsDevice::GraphicsDevice(int w, int h) :width(w), height(h)
{
	initgraph((int)width, (int)height);//创建绘图窗口
	g_pBuf = GetImageBuffer(NULL);//获取显存
	backBuffer = new DWORD[width * height];
}

void GraphicsDevice::SetPixel(int x, int y, COLORREF c)
{
	y = (int)height - 1 - y;//因为easyx的y值0点是最上面，所以将屏幕上下翻转到最下面
	g_pBuf[y * width + x] = BGR(c);
}
void GraphicsDevice::SetPixel_Back(int x, int y, COLORREF c)
{
	y = (int)height - 1 - y;//因为easyx的y值0点是最上面，所以将屏幕上下翻转到最下面
	backBuffer[y * width + x] = BGR(c);
}
void GraphicsDevice::flush()
{
	std::copy(backBuffer, backBuffer + height * width, g_pBuf);
}
void GraphicsDevice::clear_color_bcak(COLORREF c)
{
	std::fill(backBuffer, backBuffer + ((size_t)height * (size_t)width), c);
}
GraphicsDevice::~GraphicsDevice()
{
	delete backBuffer;
	closegraph();
}