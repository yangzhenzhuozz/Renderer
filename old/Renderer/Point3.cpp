#include "Point3.h"
#include <math.h>



Point3::Point3()
{
}

Point3::Point3(double x, double y, double z)
{
	value[0] = x;
	value[1] = y;
	value[2] = z;
}

Point3::~Point3()
{
}
