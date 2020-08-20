#ifndef _GRAPHICSLIBRARY
#define _GRAPHICSLIBRARY
#include <GraphicsDevice.h>
#include <vector>
#define ABS(x) ((x)>0?(x):-(x))
class Point2
{
public:
	double X, Y;
	double V;//属性值,在单属性插值SingleInterpolationIn2D函数中用
	std::vector<double> ValueArray;//本顶点属性集合，在多属性插值MultipleInterpolationArrayIn2D中使用
	Point2(double x, double y);
};

class GraphicsLibrary
{
public:
	GraphicsDevice& graphicsdevice;
	GraphicsLibrary(GraphicsDevice& gd);
	void DrawLine(const Point2& p1, const Point2& p2, COLORREF color);
	void DrawTriangle2D(const Point2& a, const Point2& b, const Point2& c, COLORREF color);
	void SingleInterpolationIn2D(const Point2& a, const Point2& b, const Point2& c);
	void MultipleInterpolationArrayIn2D(const Point2& a, const  Point2& b, const  Point2& c, int ValueLength, COLORREF (*FragmentShader)(std::vector<double>& values));//ValueLength为每个顶点属性个数
};
#endif