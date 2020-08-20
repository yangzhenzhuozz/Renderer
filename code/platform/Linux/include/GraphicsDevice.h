#ifndef _GRAPHICSDEVICE
#define _GRAPHICSDEVICE
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <algorithm>

typedef unsigned char BYTE;
typedef unsigned int COLORREF;
typedef unsigned int DWORD;
typedef unsigned char byte;
typedef unsigned short WORD;
typedef int LONG;

#define RGB(r, g, b) ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define GetBValue(rgb) (BYTE(rgb))
#define GetGValue(rgb) (BYTE(((WORD)(rgb)) >> 8))
#define GetRValue(rgb) (BYTE((rgb) >> 16))
//framebuffer在bit_per_pixel=32时，一个像素占用4字节，第0字节为b，第1字节为g，第2字节为r，第3字节暂未使用，BGR宏用于将rgb值转换为符合framebuffer排列的数据
#define BGR(color) ((((color)&0xFF) << 16) | ((color)&0xFF00FF00) | (((color)&0xFF0000) >> 16))
#ifndef _max
#define _max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef _min
#define _min(a, b) (((a) < (b)) ? (a) : (b))
#endif

class GraphicsDevice
{
public:
	int width, height;			  //记录屏幕的宽高
	GraphicsDevice(int w, int h); //Linux下的宽高指定无效，仅仅是为了统一API
	void SetPixel(int x, int y, COLORREF c);
	~GraphicsDevice();
	void SetPixel_Back(int x, int y, COLORREF c); //将像素设置到缓冲区中
	void flush();								  //将后缓冲区的数据复制到前缓冲区
	void clear_color_bcak(COLORREF c);
    DWORD *backBuffer;//后缓冲区
	BYTE *g_pBuf;		  //显存映射
	int fd;				  //frame buffer的fd
	int bytePerPixel;	  //每像素对应的字节数量
	int screenBuffersize; //显存大小

	struct fb_fix_screeninfo finfo; //帧缓冲区的固定信息
	struct fb_var_screeninfo vinfo; //帧缓冲区的可变信息
};
#endif
