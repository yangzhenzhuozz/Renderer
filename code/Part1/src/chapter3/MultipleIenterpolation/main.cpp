#include <GraphicsDevice.h>
#include <GraphicsLibrary.h>
#include "stdio.h"
int main()
{
	GraphicsDevice gd(640, 480);
	GraphicsLibrary gl(gd);
	Point2 a(0, 0);
	Point2 b(240, 240);
	Point2 c(0, 480);
	a.ValueArray = { 255,0,0 };
	b.ValueArray = { 0,255,0 };
	c.ValueArray = { 0,0,255 };
	gl.MultipleInterpolationArrayIn2D(a, b, c,3,
		[](std::vector<double>& vs) {
			return RGB((int)vs[0], (int)vs[1], (int)vs[2]);
		}
	);
	getchar();
	return 0;
}