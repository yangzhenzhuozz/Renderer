#include "Vector3.h"

Vector3::Vector3(double x, double y, double z):X(x),Y(y),Z(z)
{
}

Vector3 Vector3::cross(Vector3& a, Vector3& b)
{
	return Vector3((a.Y * b.Z - a.Z * b.Y), (a.Z * b.X - a.X * b.Z), (a.X * b.Y - a.Y * b.X));
}
