#include <GraphicsDevice.h>
#include <GraphicsLibrary.h>
#include "stdio.h"
int main()
{
	GraphicsDevice gd(640, 480);
	GraphicsLibrary gl(gd);
	Point2 a(100, 0);
	Point2 b(0, 100);
	Point2 c(100, 200);
	a.V = 0;
	b.V = 255;
	c.V = 0;
	gl.SingleInterpolationIn2D(a, b, c);
	getchar();
	return 0;
}