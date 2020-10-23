#include "include/GraphicsDevice.h"
#include <emscripten.h>
GraphicsDevice::GraphicsDevice(int w, int h) : width(w), height(h)
{
	g_pBuf = new U32[width * height]; //申请
}

void GraphicsDevice::SetPixel(int x, int y, U32 c)
{
	y = (int)height - 1 - y; //因为y值0点是最上面，所以将屏幕上下翻转到最下面
	g_pBuf[y * width + x] = BGR(c);
}
void GraphicsDevice::SetPixel_Back(int x, int y, U32 c)
{
	y = (int)height - 1 - y; //因为easyx的y值0点是最上面，所以将屏幕上下翻转到最下面
	g_pBuf[y * width + x] = BGR(c)|0xff000000;//alpha值置1，否则就是透明的了
}
void GraphicsDevice::flush()
{
	EM_ASM_INT_V(update());
}
void GraphicsDevice::clear_color_bcak(U32 c)
{
	std::fill(g_pBuf, g_pBuf + ((size_t)height * (size_t)width), c);
}
GraphicsDevice::~GraphicsDevice()
{
	delete g_pBuf;
}