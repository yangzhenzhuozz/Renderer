#ifndef _GRAPHICSDEVICE
#define _GRAPHICSDEVICE
#include <algorithm>

typedef unsigned int U32;
typedef unsigned char BYTE;	 //1 byte
typedef unsigned int DWORD;	 //4 byte
typedef unsigned short WORD; //2 byte
typedef int LONG;			 //4byte

#define RGB(r, g, b) ((U32)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define GetBValue(rgb) (BYTE(rgb))
#define GetGValue(rgb) (BYTE(((WORD)(rgb)) >> 8))
#define GetRValue(rgb) (BYTE((rgb) >> 16))
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
	void SetPixel(int x, int y, U32 c);
	~GraphicsDevice();
	void SetPixel_Back(int x, int y, U32 c); //将像素设置到缓冲区中
	void flush();								  //将后缓冲区的数据复制到前缓冲区
	void clear_color_bcak(U32 c);
	U32 *g_pBuf;		  //显存映射
	int fd;				  //frame buffer的fd
	int bytePerPixel;	  //每像素对应的字节数量
	int screenBuffersize; //显存大小
};
#endif