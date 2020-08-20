#include <GraphicsDevice.h>
#include <GraphicsLibrary.h>
#include "stdio.h"
int main()
{
	GraphicsDevice gd(640, 480);
	GraphicsLibrary gl(gd);
	Point2 a(100, 0);
	Point2 b(100, 300);
	Point2 c(300, 0);
	gl.DrawTriangle2D(a, b, c, RGB(255, 0, 0));
	getchar();
	return 0;
}