#ifndef _GRAPHICSLIBRARY
#define _GRAPHICSLIBRARY
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
	std::vector<double> ValueArray;//本顶点属性集合，在多属性插值MultipleInterpolationArrayIn2D中使用
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
	int ClipLine(const Point4& a, const Point4& b,Point4& result, int ValueLength, int clip_flag);//对a->b线段进行裁剪，返回0表示平凡接受，-1表示平凡拒绝，1表示交点为出点，2表示交点为入点,把裁剪结果放入result中，flag表示裁剪边界，取值为LEFT、RIGHT、BOTTOM、TOP、NEAR、FAR
	void ClipAndDraw(const Point4& sa, const Point4& sb, const Point4& sc, int ValueLength,double depth_boundary, COLORREF(*FragmentShader)(std::vector<double>& values));//裁剪并绘制三角形,depth_boundary为裁剪平面的深度值
private:
	void DrawTriangle(const Point4& sa, const Point4& sb, const Point4& sc, int ValueLength, COLORREF(*FragmentShader)(std::vector<double>& values));//ValueLength为顶点的属性值数量
	GraphicsDevice& graphicsdevice;
	double *Z_Buffer;//深度缓冲区
};
#endif