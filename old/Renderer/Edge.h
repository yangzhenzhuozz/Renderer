#ifndef _EdgeTable
#define _EdgeTable
#include <stdio.h>
class Edge//活性边表项
{
public:
	double x = 0, dx = 0, Ymax = 0;
	Edge(double X, double DX, double YMAX);
	~Edge();
};
#endif // !_ActiveEdgeTable

