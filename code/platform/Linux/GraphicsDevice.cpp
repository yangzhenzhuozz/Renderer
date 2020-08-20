#include "GraphicsDevice.h"
GraphicsDevice::GraphicsDevice(int w,int h)//Linux下的宽高指定无效，仅仅是为了统一API
{
    printf("Linux平台不能指定宽高\n");
    //linux FrameBuffer宽高只能获取
    fd = open("/dev/fb0", O_RDWR); //尝试打开framebuffer设备
    if (fd == -1)
    {
        perror("打开FrameBuffer文件失败\n");
        throw errno;
    }
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo))
    {
        close(fd);
        perror("读取设备信息失败\n");
        throw errno;
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo))
    {
        close(fd);
        perror("读取缓冲区信息失败\n");
        throw errno;
    }
    if (vinfo.bits_per_pixel != 32)
    {
        close(fd);
        fprintf(stderr,"像素不是32位真色彩，无法运行\n");
        throw errno;
    }
    printf("宽:%d  高%d, 每像素使用字节数:%d\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
    screenBuffersize = vinfo.yres * finfo.line_length;
    g_pBuf = (BYTE *)mmap(0, screenBuffersize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //把文件映射到内存区域
    if ((size_t)g_pBuf == -1)
    {
        perror("文件映射失败\n");
        throw errno;
    }
    width = vinfo.xres;
    height = vinfo.yres;
    bytePerPixel = vinfo.bits_per_pixel / 8;
    backBuffer=new DWORD[width*height];
}
void GraphicsDevice::SetPixel(int x, int y, COLORREF c)
{
    y = height - 1 - y;//因为屏幕的y值0点是最上面，所以将屏幕上下翻转到最下面
    size_t location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y + vinfo.yoffset) * finfo.line_length; //得到指定像素的显存地址
    *((DWORD *)(g_pBuf + location)) = BGR(c); //写数据
}
GraphicsDevice::~GraphicsDevice()
{
    delete backBuffer;
    munmap(g_pBuf, screenBuffersize);
    close(fd);
}

void GraphicsDevice::SetPixel_Back(int x, int y, COLORREF c)
{
	y = (int)height - 1 - y;//因为屏幕的y值0点是最上面，所以将屏幕上下翻转到最下面
	backBuffer[y * width + x] = BGR(c);
}
void GraphicsDevice::flush()
{
    for(int y=0;y<height;y++)
    {
        size_t location=(vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y + vinfo.yoffset) * finfo.line_length;
        std::copy(backBuffer+(y*width), backBuffer+(y*width)+width,(DWORD*)(g_pBuf+location));
    }
}
void GraphicsDevice::clear_color_bcak(COLORREF c)
{
	std::fill(backBuffer, backBuffer + ((size_t)height * (size_t)width), c);
}
