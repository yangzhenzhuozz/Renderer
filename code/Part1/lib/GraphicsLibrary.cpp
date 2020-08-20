#include "include/GraphicsLibrary.h"

Point2::Point2(double x, double y) : X(x), Y(y)
{
}

GraphicsLibrary::GraphicsLibrary(GraphicsDevice &gd) : graphicsdevice(gd)
{
}

void GraphicsLibrary::DrawLine(const Point2 &p1, const Point2 &p2, COLORREF color)
{
	double k = (p1.Y - p2.Y) / (p1.X - p2.X); //浮点数除以0得到无穷大，正常数字除以无穷大等于0，所以这个运算是安全的
	if (ABS(k) < 1)
	{
		double dy = k;
		int x = 0;
		double y = 0.0;
		int ex = 0;
		if (p1.X < p2.X) //把起点的x,y值赋给sx,sy
		{
			x = (int)p1.X;
			y = p1.Y;
			ex = (int)p2.X;
		}
		else
		{
			x = (int)p2.X;
			y = p2.Y;
			ex = (int)p1.X;
		}
		for (; x < ex; x++)
		{
			graphicsdevice.SetPixel(x, (int)y, color);
			y += dy;
		}
	}
	else
	{
		double dx = 1 / k;
		double x = 0.0;
		int y = 0;
		int ey = 0;
		if (p1.Y < p2.Y)
		{
			x = p1.X;
			y = (int)p1.Y;
			ey = (int)p2.Y;
		}
		else
		{
			x = p2.X;
			y = (int)p2.Y;
			ey = (int)p1.Y;
		}
		for (; y < ey; y++)
		{
			graphicsdevice.SetPixel((int)x, y, color);
			x += dx;
		}
	}
}

void GraphicsLibrary::DrawTriangle2D(const Point2 &a, const Point2 &b, const Point2 &c, COLORREF color)
{
	//判断面积是否为0
	if (abs(a.X * b.Y + b.X * c.Y + c.X * a.Y - a.X * c.Y - b.X * a.Y - c.X * b.Y) < 1e-15)
	{
		return; //放弃本三角形的绘制
	}
	//按照文中的方法进行排序
	const Point2 *pa = &a;
	const Point2 *pb = &b;
	const Point2 *pc = &c;
	//冒泡排序，三个数要比三次
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point2 *tmp = pa;
		pa = pb;
		pb = tmp;
	}
	if (pb->Y > pc->Y) //使得p3的y值不小于p2的y值
	{
		const Point2 *tmp = pb;
		pb = pc;
		pc = tmp;
	}
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point2 *tmp = pa;
		pa = pb;
		pb = tmp;
	}
	Point2 p = Point2((pc->X - pa->X) * ((pb->Y - pa->Y) / (pc->Y - pa->Y)) + pa->X, pb->Y); //得到P'
	const Point2 *_p = &p;
	if (pb->X > _p->X)
	{
		const Point2 *tmp = pb;
		pb = _p;
		_p = tmp;
	}
	//排序完毕

	const Point2 &P1 = *pa;
	const Point2 &P2 = *pb;
	const Point2 &P3 = *pc;
	const Point2 &P_ = *_p;
	double DXleft[2] = {(P1.X - P2.X) / (P1.Y - P2.Y), (P2.X - P3.X) / (P2.Y - P3.Y)};
	double DXright[2] = {(P1.X - P_.X) / (P1.Y - P_.Y), (P_.X - P3.X) / (P_.Y - P3.Y)};
	double Start_x[2] = {P1.X, P2.X};
	double End_x[2] = {P1.X, P_.X};
	double Start_y[2] = {P1.Y, P2.Y};
	double End_y[2] = {P2.Y, P3.Y};
	double edge_left_ex[] = {P2.X, P3.X}; //记录每条边结束的x值
	double edge_right_ex[] = {P_.X, P3.X};
	for (int i = 0; i < 2; i++)
	{
		double dx_left = DXleft[i];
		double dx_right = DXright[i];
		double sx = Start_x[i]; //扫描线起点X值
		double ex = End_x[i];	//扫描线终点X值
		double sy = Start_y[i]; //三角形起始扫描线
		double ey = End_y[i];	//三角形结束扫描线
		for (int y = (int)sy; y < ey; y++)
		{
			if (dx_left < 0) //sx随着y增大而减小
			{
				sx = _max(sx, edge_left_ex[i]); //将sx限定为不小于终点x
			}
			else if (dx_left > 0) //sx随着y增大而增大
			{
				sx =_min(sx, edge_left_ex[i]); //将sx限定为不大于终点x
			}
			if (dx_right < 0) //ex随着y增大而减小
			{
				ex = _max(ex, edge_right_ex[i]); //将ex限定为不小于终点x
			}
			else if (dx_right > 0) //ex随着y增大而增大
			{
				ex =_min(ex, edge_right_ex[i]); //将ex限定为不大于终点x
			}
			//sx到ex相当于扫描线的一部分
			for (int x = (int)sx; x <= ex; x++)
			{
				graphicsdevice.SetPixel(x, y, color);
			}
			sx = sx + dx_left;
			ex = ex + dx_right;
		}
	}
}

void GraphicsLibrary::MultipleInterpolationArrayIn2D(const Point2 &a, const Point2 &b, const Point2 &c, int ValueLength, COLORREF (*FragmentShader)(std::vector<double> &values))
{
	//判断面积是否为0
	if (abs(a.X * b.Y + b.X * c.Y + c.X * a.Y - a.X * c.Y - b.X * a.Y - c.X * b.Y) < 1e-15)
	{
		return; //放弃本三角形的绘制
	}
	//按照文中的方法进行排序
	const Point2 *pa = &a;
	const Point2 *pb = &b;
	const Point2 *pc = &c;
	//冒泡排序，三个数要比三次
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point2 *tmp = pa;
		pa = pb;
		pb = tmp;
	}
	if (pb->Y > pc->Y) //使得p3的y值不小于p2的y值
	{
		const Point2 *tmp = pb;
		pb = pc;
		pc = tmp;
	}
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point2 *tmp = pa;
		pa = pb;
		pb = tmp;
	}

	Point2 p = Point2((pc->X - pa->X) * ((pb->Y - pa->Y) / (pc->Y - pa->Y)) + pa->X, pb->Y); //得到P'
	p.ValueArray = std::vector<double>(ValueLength);										 //创建容器
	for (int i = 0; i < ValueLength; i++)													 //计算出属性集合中每个属性的值
	{
		p.ValueArray[i] = (pc->ValueArray[i] - pa->ValueArray[i]) * ((pb->Y - pa->Y) / (pc->Y - pa->Y)) + pa->ValueArray[i];
	}
	const Point2 *_p = &p;
	if (pb->X > _p->X)
	{
		const Point2 *tmp = pb;
		pb = _p;
		_p = tmp;
	}
	//排序完毕

	const Point2 &P1 = *pa;
	const Point2 &P2 = *pb;
	const Point2 &P3 = *pc;
	const Point2 &P_ = *_p;
	double DXleft[2] = {(P1.X - P2.X) / (P1.Y - P2.Y), (P2.X - P3.X) / (P2.Y - P3.Y)};
	double DXright[2] = {(P1.X - P_.X) / (P1.Y - P_.Y), (P_.X - P3.X) / (P_.Y - P3.Y)};

	std::vector<double> dvy_left1(ValueLength);
	std::vector<double> dvy_left2(ValueLength);
	std::vector<double> dvy_right1(ValueLength);
	std::vector<double> dvy_right2(ValueLength);
	for (int i = 0; i < ValueLength; i++) //计算出属性集合中每个属性的值
	{
		dvy_left1[i] = (P1.ValueArray[i] - P2.ValueArray[i]) / (P1.Y - P2.Y);
		dvy_left2[i] = (P2.ValueArray[i] - P3.ValueArray[i]) / (P2.Y - P3.Y);
		dvy_right1[i] = (P1.ValueArray[i] - P_.ValueArray[i]) / (P1.Y - P_.Y);
		dvy_right2[i] = (P_.ValueArray[i] - P3.ValueArray[i]) / (P_.Y - P3.Y);
	}
	std::vector<double> DVylefts[2] = {dvy_left1, dvy_left2};
	std::vector<double> DVyrights[2] = {dvy_right1, dvy_right2};

	double Start_x[2] = {P1.X, P2.X};
	double End_x[2] = {P1.X, P_.X};
	double Start_y[2] = {P1.Y, P2.Y};
	double End_y[2] = {P2.Y, P3.Y};

	std::vector<double> Start_vs[2] = {P1.ValueArray, P2.ValueArray};
	std::vector<double> End_vs[2] = {P1.ValueArray, P_.ValueArray};

	for (int i = 0; i < 2; i++)
	{
		double dx_left = DXleft[i];
		double dx_right = DXright[i];
		std::vector<double> dvy_lefts = DVylefts[i];
		std::vector<double> dvy_rights = DVyrights[i];
		double sx = Start_x[i];				   //扫描线起点X值
		std::vector<double> svs = Start_vs[i]; //扫描线起点属性值
		double ex = End_x[i];				   //扫描线终点X值
		std::vector<double> evs = End_vs[i];   //扫描线终点属性值
		double sy = Start_y[i];				   //三角形起始扫描线
		double ey = End_y[i];				   //三角形结束扫描线
		for (int y = (int)sy; y < ey; y++)
		{
			//sx到ex相当于扫描线的一部分
			std::vector<double> vs = svs;
			std::vector<double> dvxs(ValueLength); //计算每一个属性值v在x方向上的增量
			for (int i = 0; i < ValueLength; i++)  //计算每一个属性值v在x方向上的增量
			{
				dvxs[i] = (evs[i] - svs[i]) / (ex - sx);
				;
			}
			for (int x = (int)sx; x <= ex; x++)
			{
				graphicsdevice.SetPixel(x, y, FragmentShader(vs));
				for (int i = 0; i < ValueLength; i++)
				{
					vs[i] = vs[i] + dvxs[i];
				}
			}
			sx = sx + dx_left;

			ex = ex + dx_right;
			for (int i = 0; i < ValueLength; i++)
			{
				svs[i] = svs[i] + dvy_lefts[i];
				evs[i] = evs[i] + dvy_rights[i];
			}
		}
	}
}

void GraphicsLibrary::SingleInterpolationIn2D(const Point2 &a, const Point2 &b, const Point2 &c)
{
	//判断面积是否为0
	if (abs(a.X * b.Y + b.X * c.Y + c.X * a.Y - a.X * c.Y - b.X * a.Y - c.X * b.Y) < 1e-15)
	{
		return; //放弃本三角形的绘制
	}
	//按照文中的方法进行排序
	const Point2 *pa = &a;
	const Point2 *pb = &b;
	const Point2 *pc = &c;
	//冒泡排序，三个数要比三次
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point2 *tmp = pa;
		pa = pb;
		pb = tmp;
	}
	if (pb->Y > pc->Y) //使得p3的y值不小于p2的y值
	{
		const Point2 *tmp = pb;
		pb = pc;
		pc = tmp;
	}
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point2 *tmp = pa;
		pa = pb;
		pb = tmp;
	}
	Point2 p = Point2((pc->X - pa->X) * ((pb->Y - pa->Y) / (pc->Y - pa->Y)) + pa->X, pb->Y); //得到P'
	p.V = (pc->V - pa->V) * ((pb->Y - pa->Y) / (pc->Y - pa->Y)) + pa->V;					 //计算P点属性值

	const Point2 *_p = &p;
	if (pb->X > _p->X)
	{
		const Point2 *tmp = pb;
		pb = _p;
		_p = tmp;
	}
	//排序完毕

	const Point2 &P1 = *pa;
	const Point2 &P2 = *pb;
	const Point2 &P3 = *pc;
	const Point2 &P_ = *_p;
	double DXleft[2] = {(P1.X - P2.X) / (P1.Y - P2.Y), (P2.X - P3.X) / (P2.Y - P3.Y)};
	double DXright[2] = {(P1.X - P_.X) / (P1.Y - P_.Y), (P_.X - P3.X) / (P_.Y - P3.Y)};
	double DVyleft[2] = {(P1.V - P2.V) / (P1.Y - P2.Y), (P2.V - P3.V) / (P2.Y - P3.Y)};
	double DVyright[2] = {(P1.V - P_.V) / (P1.Y - P_.Y), (P_.V - P3.V) / (P_.Y - P3.Y)};
	double Start_x[2] = {P1.X, P2.X};
	double Start_v[2] = {P1.V, P2.V};
	double End_x[2] = {P1.X, P_.X};
	double End_v[2] = {P1.V, P_.V};
	double Start_y[2] = {P1.Y, P2.Y};
	double End_y[2] = {P2.Y, P3.Y};
	double edge_left_ex[] = {P2.X, P3.X}; //记录每条边结束的x值
	double edge_right_ex[] = {P_.X, P3.X};
	for (int i = 0; i < 2; i++)
	{
		double dx_left = DXleft[i];
		double dx_right = DXright[i];
		double dvy_left = DVyleft[i];
		double dvy_right = DVyright[i];
		double sx = Start_x[i]; //扫描线起点X值
		double sv = Start_v[i]; //扫描线起点属性值
		double ex = End_x[i];	//扫描线终点X值
		double ev = End_v[i];	//扫描线终点属性值
		double sy = Start_y[i]; //三角形起始扫描线
		double ey = End_y[i];	//三角形结束扫描线
		for (int y = (int)sy; y < ey; y++)
		{
			//sx到ex相当于扫描线的一部分
			double v = sv;
			double dvx = (ev - sv) / (ex - sx); //计算属性值v在x方向上的增量
			for (int x = (int)sx; x <= ex; x++)
			{
				graphicsdevice.SetPixel(x, y, RGB((int)v, (int)v, (int)v));
				v = v + dvx;
			}
			sx = sx + dx_left;
			sv = sv + dvy_left;
			ex = ex + dx_right;
			ev = ev + dvy_right;
		}
	}
}