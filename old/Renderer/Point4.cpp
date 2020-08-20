#include "Point4.h"
#include <math.h>


Point4::Point4()
{
}

Point4::Point4(double x, double y, double z, double w)
{
	value[0] = x;
	value[1] = y;
	value[2] = z;
	value[3] = w;
}

Point4::~Point4()
{
}
