#include "include/GraphicsLibrary.h"

Point4::Point4(double x, double y, double z, double w) : X(x), Y(y), Z(z), W(w)
{
}

Point4 Point4::Normalize() const
{
	return Point4(this->X / this->W, this->Y / this->W, this->Z / this->W, 1);
}

GraphicsLibrary::GraphicsLibrary(GraphicsDevice &gd) : graphicsdevice(gd)
{
}

void GraphicsLibrary::DrawTriangle(const Point4 &sa, const Point4 &sb, const Point4 &sc, int ValueLength, COLORREF (*FragmentShader)(std::vector<double> &values))
{
	//将齐次坐标规范化
	Point4 a = sa.Normalize();
	Point4 b = sb.Normalize();
	Point4 c = sc.Normalize();
	a.ValueArray = sa.ValueArray;
	b.ValueArray = sb.ValueArray;
	c.ValueArray = sc.ValueArray;

	//判断面积是否为0
	if (abs(a.X * b.Y + b.X * c.Y + c.X * a.Y - a.X * c.Y - b.X * a.Y - c.X * b.Y) < 1e-15)
	{
		return; //放弃本三角形的绘制
	}
	//按照文中的方法进行排序
	const Point4 *pa = &a;
	const Point4 *pb = &b;
	const Point4 *pc = &c;
	//冒泡排序，三个数要比三次
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point4 *tmp = pa;
		pa = pb;
		pb = tmp;
	}
	if (pb->Y > pc->Y) //使得p3的y值不小于p2的y值
	{
		const Point4 *tmp = pb;
		pb = pc;
		pc = tmp;
	}
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point4 *tmp = pa;
		pa = pb;
		pb = tmp;
	}

	Point4 p = Point4((pc->X - pa->X) * ((pb->Y - pa->Y) / (pc->Y - pa->Y)) + pa->X, pb->Y, 0, 1); //得到P'
	p.ValueArray = std::vector<double>(ValueLength);											   //创建容器
	for (int i = 0; i < ValueLength; i++)														   //计算出属性集合中每个属性的值
	{
		p.ValueArray[i] = (pc->ValueArray[i] - pa->ValueArray[i]) * ((pb->Y - pa->Y) / (pc->Y - pa->Y)) + pa->ValueArray[i];
	}
	const Point4 *_p = &p;
	if (pb->X > _p->X)
	{
		const Point4 *tmp = pb;
		pb = _p;
		_p = tmp;
	}
	//排序完毕

	const Point4 &P1 = *pa;
	const Point4 &P2 = *pb;
	const Point4 &P3 = *pc;
	const Point4 &P_ = *_p;
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

		double edge_left_ex[] = {P2.X, P3.X}; //记录每条边结束的x值
		double edge_right_ex[] = {P_.X, P3.X};
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
			std::vector<double> vs = svs;
			std::vector<double> dvxs(ValueLength); //计算每一个属性值v在x方向上的增量
			for (int i = 0; i < ValueLength; i++) //计算每一个属性值v在x方向上的增量
			{
				dvxs[i] = (evs[i] - svs[i]) / (ex - sx);
			}
			//sx到ex相当于扫描线的一部分
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
