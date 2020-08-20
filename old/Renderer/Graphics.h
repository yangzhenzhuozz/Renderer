#ifndef _GRAPHICSMLIBRARY
#define _GRAPHICSMLIBRARY
#include <graphics.h>      // 引用图形库头文件
#include <list>
#include "Point2.h"
#include "Point4.h"
#include "Edge.h"
#include "Matrix4.h"
/*
不支持ddy和ddy，所以没有Mipmap，因为实现起来比较麻烦
ddy和ddy是指在屏幕空间上求vbo的偏导数
真正的GPU绘制的时候是在屏幕同时绘制多个像素，GPU并行处理能力很强
比如当前GPU同时在屏幕上绘制四个点(x,y),(x+1,y),(x,y+1),(x+1,y+1)分别记为p1,p2,p3,p4
设前面三个点p(即不包含点(x+1,y+1))的纹理坐标分别为(u1,v1),(u2,v2),(u3,v3)
则点p1上的ddx(u)=(u2-u1)/1,ddy(u)=(u3-u1)/1,这个计算值可以用来计算Mipmap
同时现在主流游戏引擎上面的法线贴图都是用MikktSpace计算切线空间，也需要求偏导数，但是对我们现在的这个渲染器来说，这些可以暂时不考虑

返回bool的函数表示有成功或者失败的问题，如果执行失败，可以通过errmsg读取，任何返回bool的函数在进入的时候都会清空errmsg，所以一旦出现问题，请立马读取errmsg

CVV坐标系是上面为Y正方向,右面为X正方向,屏幕向内为Z正方向，右手坐标系

视口空间和屏幕空间的宽高单位一样，仅仅是增加了一个x,y的偏移
*/
class GraphicsLibrary
{
	/*坐标系是上面为Y正方向，右面为X正方向，屏幕向外为Z正方向*/
public:
	char errmsg[1024] = { '0' };//错误信息，如果执行出错则可以读取本信息
	void (*VertexShader)(const double* vbo, double* varying, Point4& Position);//顶点着色器，概念和OpenGL类似，但是参数有区别，下面是ABO的说明
	/*
	Position对应了OpenGL的gl_Position，
	VBO来当前顶点的属性，
	varying表示需要通过顶点着色器传递给片元着色器的变量，会在顶点处插值被传递给片元着色器
	*/
	void (*FragmentShader)(double* varying, COLORREF& FragColor);//片源，概念和OpenGL类似，但是参数有区别，下面是ABO的说明
	/*
	FragColor对应于OpenGL的gl_FragColor，只是没有了透明度，只有RGB
	顶点着色器中的Varying经过1/w插值之后传递给片元着色器
	*/
	bool enable_CW = true;//是否启用顺时针逆时针三角形剔除
	bool CW_CCW = false;//默认逆时针,true为顺时针
	GraphicsLibrary(unsigned int w, unsigned int h);
	void setVBO(double* buffer, int numOfvertex, int count);
	~GraphicsLibrary();
	void fast_putpixel(int x, int y, COLORREF c);
	COLORREF fast_getpixel(int x, int y);
	bool loadBMP(const char* filename);//加载bmp文件到纹理,返回false表示失败（只能加载24位，无压缩，纵轴正向BMP）
	void flush();// 使针对绘图窗口的显存操作生效
	bool Draw();//返回false表示绘制失败
	void clear();
	void clearDepth(double v);//清理深度缓冲区的值为v
	void Swap();//对于EasyX是用BeginBatchDraw和EndBatchDraw实现的
	void setVaryingCount(int count);//设置Varying变量的数量，如果有使用Varying，则一定要调用本函数
	COLORREF texture2D(double x, double y);//读取纹理中的颜色
private:
	unsigned int ScreenWidth = 0, ScreenHeight = 0;
	int bmpHeight = 0;
	int bmpwidth = 0;//位图宽高
	unsigned char* bmpData = NULL;//位图数据区
	unsigned char* textureBuffer = NULL;//纹理缓冲区，保存bmp位图
	int TextureHeight, TextureWidth;//纹理宽高
	void DrawTriangle(Point4* pointArray,double* varying);//使用扫描线填充算法绘制三角形
	DWORD* g_pBuf;//显存指针
	double* Z_Buffer = NULL;//深度缓冲区
	double* vboBuffer = NULL;//vob
	int vboCount = 0;//顶点数量
	int NumOfVertexVBO = 0;//每个顶点的顶点数量
	int CountOfVarying = 0;//Varying变量数量
	std::list<Edge>* NET = NULL;//新边表和ScreenHeight大小一样
	double* interpolationVarying = NULL;//当前线程在绘制当前顶点插值之后的varying，因为单线程，所以这里只需要一个就行了
	int clipEdge(Point4& A, Point4& B, Point4& tmpA, Point4& tmpB, double& proportionA, double& proportionB);//对边进行裁剪(在四维空间中)
	int clipEdgeByParallelFace(Point4& A, Point4& B, Point4& tmpA, Point4& tmpB, double& proportionA, double& proportionB,int flag);//使用一组平行平面裁剪边,flag为1，2，3时分别使用left right、bottom top、near far平面裁剪
	void clipFaceByParallelFace(Point4* ps, int pCount, Point4* resultPoint, int& resultCount, double* varying, int countofvarying, double* resultvarying,int flag);//使用CVV对平面裁剪,resultCount 为裁剪之后的顶点数量
};
#endif // !_GRAPHICSMLIBRARY
