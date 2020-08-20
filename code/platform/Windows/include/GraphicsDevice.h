#ifndef _GRAPHICSDEVICE
#define _GRAPHICSDEVICE
#include <graphics.h>
#include <algorithm>
#ifndef _max
#define _max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef _min
#define _min(a,b) (((a) < (b)) ? (a) : (b))
#endif
class GraphicsDevice
{
public:
	size_t width, height;//记录屏幕的宽高
	GraphicsDevice(int width, int height);
	void SetPixel(int x, int y, COLORREF c);
	void SetPixel_Back(int x, int y, COLORREF c);//将像素设置到缓冲区中
	void flush();//将后缓冲区的数据复制到前缓冲区
	void clear_color_bcak(COLORREF c);
	~GraphicsDevice();
	DWORD* backBuffer;//后缓冲区
	DWORD* g_pBuf;//显存指针
};
#endif

