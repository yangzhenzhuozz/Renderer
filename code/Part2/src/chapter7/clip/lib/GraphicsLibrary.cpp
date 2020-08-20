#include "include/GraphicsLibrary.h"

Point4::Point4(double x, double y, double z, double w) : X(x), Y(y), Z(z), W(w)
{
}

Point4 Point4::Normalize() const
{
	return Point4(this->X / this->W, this->Y / this->W, this->Z / this->W, 1);
}

Point4 Point4::Normalize_special() const
{
	return Point4(this->X / this->W, this->Y / this->W, this->Z / this->W, this->W);
}

GraphicsLibrary::GraphicsLibrary(GraphicsDevice &gd) : graphicsdevice(gd)
{
	Z_Buffer = new double[(size_t)gd.height * (size_t)gd.width];
}

GraphicsLibrary::~GraphicsLibrary()
{
	delete Z_Buffer;
}

void GraphicsLibrary::clean_depth(double v)
{
	std::fill(Z_Buffer, Z_Buffer + ((size_t)graphicsdevice.height * (size_t)graphicsdevice.width), v);
}

int GraphicsLibrary::ClipLine(const Point4 &a, const Point4 &b, Point4 &result, int ValueLength, double depth_boundary)
{
	//使用ω判断深度范围
	if (a.W < depth_boundary && b.W < depth_boundary) //平凡拒绝
	{
		return REIECTION;
	}
	else if (a.W > depth_boundary && b.W > depth_boundary) //平凡接受
	{
		return ACCEPTANCE;
	}
	else //裁剪
	{
		int ret;
		if (a.W > depth_boundary) //裁剪点为出点
		{
			ret = OUT_POINT;
		}
		else //裁剪点为入点
		{
			ret = IN_POINT;
		}
		double t = (depth_boundary - a.W) / (b.W - a.W); //计算t，然后使用t计算线段和边界交点的坐标
		result.X = a.X + (b.X - a.X) * t;
		result.Y = a.Y + (b.Y - a.Y) * t;
		result.Z = a.Z + (b.Z - a.Z) * t;
		result.W = a.W + (b.W - a.W) * t;
		for (int i = 0; i < ValueLength; i++)
		{
			result.ValueArray[i] = a.ValueArray[i] + (b.ValueArray[i] - a.ValueArray[i]) * t;
		}
		return ret;
	}
}

void GraphicsLibrary::ClipAndDraw(const Point4 &sa, const Point4 &sb, const Point4 &sc, int ValueLength, double depth_boundary, COLORREF (*FragmentShader)(std::vector<double> &values))
{

	int stack_index = 0;																				//栈顶部指针
	Point4 stack[4] = {Point4(0, 0, 0, 0), Point4(0, 0, 0, 0), Point4(0, 0, 0, 0), Point4(0, 0, 0, 0)}; //先构造一个栈
	Point4 tmp(0, 0, 0, 0);
	tmp.ValueArray = std::vector<double>(ValueLength); //创建属性容器

	Point4 const *points[] = {&sa, &sb, &sc}; //为了方便循环调用ClipLine函数对边进行裁剪，所以将边放入数组
	for (int i = 0; i < 3; i++)
	{
		int ret = ClipLine(*points[i], *points[(i + 1) % 3], tmp, ValueLength, depth_boundary); //points[0],points[1]为边ab，points[1],points[2]为边bc，points[2],points[0]为边ca
		if (ret == ACCEPTANCE)
		{
			stack[stack_index] = *points[i];
			stack_index++;
		}
		else if (ret == IN_POINT)
		{
			stack[stack_index] = tmp;
			stack_index++;
		}
		else if (ret == OUT_POINT)
		{
			stack[stack_index] = *points[i];
			stack_index++;
			stack[stack_index] = tmp;
			stack_index++;
		}
	}
	if (stack_index != 0) //如果三角形的三条边都被平凡拒绝，则不再需要绘制这个三角形了
	{
		DrawTriangle(stack[0], stack[1], stack[2], ValueLength, FragmentShader);
		if (stack_index == 4)
		{
			DrawTriangle(stack[0], stack[2], stack[3], ValueLength, FragmentShader);
		}
	}
}

void GraphicsLibrary::DrawTriangle(const Point4 &sa, const Point4 &sb, const Point4 &sc, int ValueLength, COLORREF (*FragmentShader)(std::vector<double> &values))
{
	//将齐次坐标规范化
	Point4 a = sa.Normalize_special();
	Point4 b = sb.Normalize_special();
	Point4 c = sc.Normalize_special();
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
	double p_t = (pb->Y - pa->Y) / (pc->Y - pa->Y);						   //得到屏幕空间点P'的t'值(c'点的权值)
	double p_omega = (pa->W * pc->W) / (pc->W + (pa->W - pc->W) * p_t);	   //计算P'点的ω值
	double p_z = pa->Z + (pc->Z - pa->Z) * p_t;							   //使用线性插值计算z分量
	Point4 p = Point4(pa->X + (pc->X - pa->X) * p_t, pb->Y, p_z, p_omega); //得到P'
	p.ValueArray = std::vector<double>(ValueLength);					   //创建容器
	for (int i = 0; i < ValueLength; i++)								   //计算出属性集合中每个属性的值
	{
		p.ValueArray[i] = (pc->W * pa->ValueArray[i] + (pa->W * pc->ValueArray[i] - pc->W * pa->ValueArray[i]) * p_t) / (pc->W + (pa->W - pc->W) * p_t);
	}
	const Point4 *_p = &p;
	if (pb->X > _p->X) //让_p永远在右侧
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
	double DXleft[2] = {(P1.X - P2.X) / (P1.Y - P2.Y), (P2.X - P3.X) / (P2.Y - P3.Y)}; //计算x增量
	double DXright[2] = {(P1.X - P_.X) / (P1.Y - P_.Y), (P_.X - P3.X) / (P_.Y - P3.Y)};

	//计算t的增量
	double dt_left[] = {1 / (P2.Y - P1.Y), 1 / (P3.Y - P2.Y)};
	double dt_right[] = {1 / (P_.Y - P1.Y), 1 / (P3.Y - P_.Y)};

	//计算z分量的增量(因为现在可以在屏幕空间使用线性插值了)
	double dz_left[] = {(P2.Z - P1.Z) / (P2.Y - P1.Y), (P3.Z - P2.Z) / (P3.Y - P2.Y)};
	double dz_right[] = {(P_.Z - P1.Z) / (P_.Y - P1.Y), (P3.Z - P_.Z) / (P3.Y - P_.Y)};

	//记录每条边结束的x值
	double edge_left_ex[] = {P2.X, P3.X};
	double edge_right_ex[] = {P_.X, P3.X};

	//用于计算单条扫描线的起始和结束x值
	double Start_x[2] = {P1.X, P2.X};
	double End_x[2] = {P1.X, P_.X};
	double Start_y[2] = {P1.Y, P2.Y};
	double End_y[2] = {P2.Y, P3.Y};

	//保存各条边起始和结束的一些值
	double left_start_w[2] = {P1.W, P2.W};
	double left_end_w[2] = {P2.W, P3.W};
	double right_start_w[2] = {P1.W, P_.W};
	double right_end_w[2] = {P_.W, P3.W};

	double left_start_z[2] = {P1.Z, P2.Z};
	double right_start_z[2] = {P1.Z, P_.Z};

	const std::vector<double> *left_start_value[2] = {&P1.ValueArray, &P2.ValueArray};
	const std::vector<double> *left_end_value[2] = {&P2.ValueArray, &P3.ValueArray};
	const std::vector<double> *right_start_value[2] = {&P1.ValueArray, &P_.ValueArray};
	const std::vector<double> *right_end_value[2] = {&P_.ValueArray, &P3.ValueArray};

	std::vector<double> svs(ValueLength); //扫描线起点属性值集合
	std::vector<double> evs(ValueLength); //扫描线终点属性值集合
	std::vector<double> vs(ValueLength);  //当前像素属性值集合
	for (int i = 0; i < 2; i++)			  //i=0和1时分别表示下半三角形和上半三角形
	{
		double dx_left = DXleft[i];
		double dx_right = DXright[i];

		double sx = Start_x[i]; //扫描线起点X值
		double ex = End_x[i];	//扫描线终点X值

		double sy = Start_y[i]; //三角形起始扫描线
		double ey = End_y[i];	//三角形结束扫描线

		double sz = left_start_z[i];
		double ez = right_start_z[i];

		double t_left = 0;
		double t_right = 0;

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

			double lwa = left_start_w[i];
			double lwb = left_end_w[i];

			double rwa = right_start_w[i];
			double rwb = right_end_w[i];
			//本行扫描线起始和结束的ω值
			double sw = (lwa * lwb) / (lwb + (lwa - lwb) * t_left);
			double ew = (rwa * rwb) / (rwb + (rwa - rwb) * t_right);

			for (int vindex = 0; vindex < ValueLength; vindex++) //计算每条扫描线的起始和结束属性值
			{
				double lva = (*left_start_value[i])[vindex];
				double lvb = (*left_end_value[i])[vindex];
				svs[vindex] = (lwb * lva + (lwa * lvb - lwb * lva) * t_left) / (lwb + (lwa - lwb) * t_left);

				double rva = (*right_start_value[i])[vindex];
				double rvb = (*right_end_value[i])[vindex];
				evs[vindex] = (rwb * rva + (rwa * rvb - rwb * rva) * t_right) / (rwb + (rwa - rwb) * t_right);
			}

			double dt = 1 / (ex - sx); //t在扫描线上面的变化率
			double t = 0;
			double dz_in_line = (ez - sz) / (ex - sx); //z在扫描线上面的增量
			double z = sz;
			//sx到ex相当于扫描线的一部分
			for (int x = (int)sx; x <= ex; x++, t += dt, z += dz_in_line) //更新比例值,更新深度值，因为这两个值在执行continue时也需要继续更新
			{
				double wa = sw;
				double wb = ew;
				if (z > Z_Buffer[y * graphicsdevice.width + x]) //深度测试不通过的话则放弃本像素的渲染
				{
					continue;
				}
				else //否则更新深度值
				{
					Z_Buffer[y * graphicsdevice.width + x] = z;
				}
				for (int vindex = 0; vindex < ValueLength; vindex++) //计算每个顶点的属性值
				{
					double va = svs[vindex];
					double vb = evs[vindex];
					vs[vindex] = (wb * va + (wa * vb - wb * va) * t) / (wb + (wa - wb) * t);
				}
				graphicsdevice.SetPixel(x, y, FragmentShader(vs));
			}
			sx = sx + dx_left;
			ex = ex + dx_right;

			t_left += dt_left[i];
			t_right += dt_right[i];

			sz += dz_left[i];
			ez += dz_right[i];
		}
	}
}
