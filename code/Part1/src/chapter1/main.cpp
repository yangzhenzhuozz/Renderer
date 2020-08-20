#include <GraphicsDevice.h>
#include "stdio.h"
int main()
{
	GraphicsDevice gd(640, 480);
	gd.SetPixel(200, 200, RGB(255, 0, 0));
	getchar();//等待用户观察
	return 0;
}