#ifndef _GRAPHICSLIBRARY
#define _GRAPHICSLIBRARY
#include <GraphicsDevice.h>
#include <vector>
#define ABS(x) ((x)>0?(x):-(x))
class Point4
{
public:
	double X, Y, Z, W;
	std::vector<double> ValueArray;//本顶点属性集合，在多属性插值MultipleInterpolationArrayIn2D中使用
	Point4(double x, double y, double z, double w);
	Point4 Normalize() const;//将其次坐标规范化后返回,齐次坐标变成三维坐标
};

class GraphicsLibrary
{
public:
	GraphicsDevice& graphicsdevice;
	GraphicsLibrary(GraphicsDevice& gd);
	void DrawTriangle(const Point4& sa, const Point4& sb, const Point4& sc, int ValueLength, COLORREF(*FragmentShader)(std::vector<double>& values));//ValueLength为顶点的属性值数量
};
#endif