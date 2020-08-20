#ifndef _GRAPHICS
#define _GRAPHICS
/*
本文件定义一个画布类，使用EasyX提供SetPixel函数，如果有需求，可以用其他方法提供SetPixel函数，比如GDI、D2D、SDL等实现
*/
#include <graphics.h>      // 引用Easyx的图形库头文件
class Graphics
{
private:
	DWORD* g_pBuf;//显存指针
public:
	int WIDTH, HEIGHT;
	Graphics(int w,int h);//创建画布,w,h分别为画布宽高
	void setPixel(int x, int y, COLORREF c);//往画布指定像素设置一个颜色
	void clear();
	~Graphics();
};
#endif // !_GRAPHICS