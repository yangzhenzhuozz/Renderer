#ifndef _GRAPHICSLIBRARY
#define GRAPHICSLIBRARY
#include <GraphicsDevice.h>
#include <vector>
#include <algorithm>
#define ABS(x) ((x)>0?(x):-(x))
#define _REIECTION -1 //平凡拒绝
#define _ACCEPTANCE 0 //平凡接受
#define _OUT_POINT 1 //出点
#define _IN_POINT 2 //入点
#define _LEFT 0//左边界
#define _RIGHT 1//右边界
#define _BOTTOM 2//下边界
#define _TOP 3//上边界
#define _NEAR 4//near边界
#define _FAR 5//far边界
class Point4
{
public:
	double X, Y, Z, W;
	Point4();
	Point4(double x, double y, double z, double w);
	Point4 Normalize() const;//将其次坐标规范化后返回,齐次坐标变成三维坐标
	Point4 Normalize_special() const;//将其次坐标规范化后返回,齐次坐标变成三维坐标，但是本函数计算后将会保留ω分量，而不是将其置为1
};

class GraphicsLibrary
{
public:
	GraphicsLibrary(GraphicsDevice& gd);
	~GraphicsLibrary();
	void clean_depth(double v);//用于批量设置深度缓冲区的值
	void setVBO(double* vbo1, size_t size_1, double* vbo2, size_t size_2, size_t p_length);
	void draw();
	void setVaryingSize(size_t size);//设置每个顶点Varying变量的数量
	void (*VertexShader)(double* vbo1, double* vbo2, Point4& gl_Vertex, std::vector<double>& varying) = nullptr;//指向VS的指针
	void (*FragmentShader)(std::vector<double>& varying, COLORREF &gl_Color) = nullptr;//指向FS的指针
private:
	int ClipLine(const Point4& a, std::vector<double>& varying_a, const Point4& b, std::vector<double>& varying_b, Point4& result, std::vector<double>& varying_result, int clip_flag); // 对a->b线段进行裁剪，返回0表示平凡接受， - 1表示平凡拒绝，1表示交点为出点，2表示交点为入点, 把裁剪结果放入result中，flag表示裁剪边界，取值为LEFT、RIGHT、BOTTOM、TOP、NEAR、FAR
	void ClipTriangle(Point4* points, std::vector<double>* varyings);//裁剪三角形
	void DrawTriangle(Point4& spa, Point4& spb, Point4& spc, std::vector<double>& sva, std::vector<double>& svb, std::vector<double>& svc);//绘制三角形
	GraphicsDevice& graphicsdevice;
	double* Z_Buffer;//深度缓冲区

	//定义两个VBO
	double* vbo1 = nullptr;
	double* vbo2 = nullptr;
	size_t vbo1_size = 0;//vbo1中每个顶点拥有的属性数量
	size_t vbo2_size = 0;//vbo2中每个顶点拥有的属性数量
	size_t position_length = 0;//总顶点数量
	size_t varying_length = 0;//每个顶点Varying变量的数量
};
#endif