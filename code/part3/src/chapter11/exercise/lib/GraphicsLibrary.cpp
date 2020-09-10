#include "include/GraphicsLibrary.h"

Point4::Point4() : X(0), Y(0), Z(0), W(0)
{
}

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

GraphicsLibrary::GraphicsLibrary(GraphicsDevice& gd) : graphicsdevice(gd)
{
	Z_Buffer = new double[(size_t)gd.height * (size_t)gd.width];
}

GraphicsLibrary::~GraphicsLibrary()
{
	delete Z_Buffer;
}

void GraphicsLibrary::setVBO(double* v1, size_t s1, double* v2, size_t s2, size_t pl)
{
	vbo1 = v1;
	vbo1_size = s1;
	vbo2 = v2;
	vbo2_size = s2;
	position_length = pl;
}

void GraphicsLibrary::draw()
{
	Point4 gl_Vertexs[3];																															   //三角形顶点坐标
	std::vector<double> varyings[3] = { std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length) }; //三角形顶点的varying缓冲区,这里可以用变长数组或者alloac
	for (int i = 0; i < position_length; i++)																										   //对每个顶点调用一次VS
	{
		VertexShader(vbo1 + i * vbo1_size, vbo2 + i * vbo2_size, gl_Vertexs[i % 3], varyings[i % 3]);
		if (i % 3 == 2) //每计算出三个顶点，则调用一次三角形绘制函数
		{
			ClipTriangle(gl_Vertexs, varyings);
		}
	}
}

void GraphicsLibrary::setVaryingSize(size_t size)
{
	varying_length = size;
}

void GraphicsLibrary::clear_color(COLORREF c)
{
	graphicsdevice.clear_color_bcak(c);
}

void GraphicsLibrary::clear_depth(double d)
{
	std::fill(Z_Buffer, Z_Buffer + ((size_t)graphicsdevice.height * (size_t)graphicsdevice.width), d);
}

void GraphicsLibrary::flush()
{
	graphicsdevice.flush();
}

int GraphicsLibrary::ClipLine(const Point4& a, std::vector<double>& varying_a, const Point4& b, std::vector<double>& varying_b, Point4& result, std::vector<double>& varying_result, int clip_flag)
{
	double flag_a = 0, flag_b = 0;
	switch (clip_flag)
	{
	case _LEFT:
		flag_a = a.W + a.X;
		flag_b = b.W + b.X;
		break;
	case _RIGHT:
		flag_a = a.W - a.X;
		flag_b = b.W - b.X;
		break;
	case _BOTTOM:
		flag_a = a.W + a.Y;
		flag_b = b.W + b.Y;
		break;
	case _TOP:
		flag_a = a.W - a.Y;
		flag_b = b.W - b.Y;
		break;
	case _NEAR:
		flag_a = a.W + a.Z;
		flag_b = b.W + b.Z;
		break;
	case _FAR:
		flag_a = a.W - a.Z;
		flag_b = b.W - b.Z;
		break;
	}
	//使用不等式2可以少写很多代码
	if (flag_a < 0 && flag_b < 0) //平凡拒绝
	{
		return _REIECTION;
	}
	if (flag_a >= 0 && flag_b >= 0) //平凡接受
	{
		return _ACCEPTANCE;
	}
	double t = flag_a / (flag_a - flag_b); //使用t值2公式同样可以少写很多代码
	result.X = a.X + (b.X - a.X) * t;	   //计算交点
	result.Y = a.Y + (b.Y - a.Y) * t;
	result.Z = a.Z + (b.Z - a.Z) * t;
	result.W = a.W + (b.W - a.W) * t;
	for (int i = 0; i < varying_length; i++) //计算交点的属性值
	{
		varying_result[i] = varying_a[i] + (varying_b[i] - varying_a[i]) * t;
	}
	if (flag_a < 0) //a点被裁则交点为入点
	{
		return _IN_POINT;
	}
	else
	{
		return _OUT_POINT;
	}
}

void GraphicsLibrary::ClipTriangle(Point4* points, std::vector<double>* varyings)
{
	//用于接受裁剪结果的容器
	Point4 tp1[3] = { points[0], points[1], points[2] }; //初始容器
	Point4 tp2[4];
	Point4 tp3[5];
	Point4 tp4[6];
	Point4 tp5[7];
	Point4 tp6[8];
	Point4 tp7[9];
	std::vector<double> tv1[3] = { varyings[0], varyings[1], varyings[2] }; //这个地方如果用gcc编译，可以用动态数组，就不需要用vector来代替了，起码看起来不会这么难看，如果不支持动态的varying长度，直接定长，也可以写得比较优雅，也可以用alloca来做，反正就是有很多优雅的实现方式，我选择了比较难看的写法
	std::vector<double> tv2[4] = { std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length) };
	std::vector<double> tv3[5] = { std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length) };
	std::vector<double> tv4[6] = { std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length) };
	std::vector<double> tv5[7] = { std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length) };
	std::vector<double> tv6[8] = { std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length) };
	std::vector<double> tv7[9] = { std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length), std::vector<double>(varying_length) };
	Point4* result_point_stack[7] = { tp1, tp2, tp3, tp4, tp5, tp6, tp7 };				//7个顶点容器
	std::vector<double>* result_varying_stack[7] = { tv1, tv2, tv3, tv4, tv5, tv6, tv7 }; //7个varying点容器
	int result_stack_index[7] = { 3, 0, 0, 0, 0, 0, 0 };									//栈顶指针,同时还等于当前多边形的边数量

	Point4 result_point;									  //用于保存裁剪点坐标的临时变量
	std::vector<double> result_varying_point(varying_length); //用于保存裁剪点varying的临时变量

	for (int clip_flag = 0; clip_flag < 6; clip_flag++) //依次调用六个边界对多边形进行裁剪
	{
		for (int i = 0; i < result_stack_index[clip_flag]; i++) //依次对n条边进行裁剪
		{
			int ret = ClipLine(result_point_stack[clip_flag][i], result_varying_stack[clip_flag][i], result_point_stack[clip_flag][(i + 1) % result_stack_index[clip_flag]], result_varying_stack[clip_flag][(i + 1) % result_stack_index[clip_flag]], result_point, result_varying_point, clip_flag); //points[0],points[1]为边ab，points[1],points[2]为边bc，points[2],points[0]为边ca
			if (ret == _ACCEPTANCE)
			{
				result_point_stack[clip_flag + 1][result_stack_index[clip_flag + 1]] = result_point_stack[clip_flag][i];
				result_varying_stack[clip_flag + 1][result_stack_index[clip_flag + 1]] = result_varying_stack[clip_flag][i];
				result_stack_index[clip_flag + 1]++;
			}
			else if (ret == _IN_POINT)
			{
				result_point_stack[clip_flag + 1][result_stack_index[clip_flag + 1]] = result_point;
				result_varying_stack[clip_flag + 1][result_stack_index[clip_flag + 1]] = result_varying_point;
				result_stack_index[clip_flag + 1]++;
			}
			else if (ret == _OUT_POINT)
			{
				result_point_stack[clip_flag + 1][result_stack_index[clip_flag + 1]] = result_point_stack[clip_flag][i];
				result_varying_stack[clip_flag + 1][result_stack_index[clip_flag + 1]] = result_varying_stack[clip_flag][i];
				result_stack_index[clip_flag + 1]++;

				result_point_stack[clip_flag + 1][result_stack_index[clip_flag + 1]] = result_point;
				result_varying_stack[clip_flag + 1][result_stack_index[clip_flag + 1]] = result_varying_point;
				result_stack_index[clip_flag + 1]++;
			}
		}
	}
	if (result_stack_index[6] != 0) //如果三角形的三条边被所有边界平凡拒绝，则不再需要绘制这个三角形了
	{
		for (int i = 0; i < result_stack_index[6] - 2; i++)
		{
			DrawTriangle(result_point_stack[6][0], result_point_stack[6][i + 1], result_point_stack[6][i + 2], result_varying_stack[6][0], result_varying_stack[6][i + 1], result_varying_stack[6][i + 2]);
		}
	}
}

void GraphicsLibrary::DrawTriangle(Point4& spa, Point4& spb, Point4& spc, std::vector<double>& sva, std::vector<double>& svb, std::vector<double>& svc)
{
	//将齐次坐标规范化
	Point4 a = spa.Normalize_special();
	Point4 b = spb.Normalize_special();
	Point4 c = spc.Normalize_special();

	double square = a.X * b.Y + b.X * c.Y + c.X * a.Y - a.X * c.Y - b.X * a.Y - c.X * b.Y; //得到有向面积的2倍
	if (abs(square) < 1e-15)//当面积为0时放弃本三角形的绘制
	{
		return;
	}
	if (FrontFace == _FrontFace_CW && square > 0) //当需要保留顺时针三角形但是有向面积为正数时放弃
	{
		return;
	}
	else if (FrontFace == _FrontFace_CCW && square < 0) //当需要保留逆时针三角形但是有向面积为负数时放弃
	{
		return;
	}

	//将[-1,1]变换成[0,width-1]和[0,height-1]
	a.X = (a.X + 1) / 2 * (graphicsdevice.width - 1); //因为屏幕宽度坐标为0～width-1,高度为0～height-1,所以这些运算都有-1
	a.Y = (a.Y + 1) / 2 * (graphicsdevice.height - 1);
	b.X = (b.X + 1) / 2 * (graphicsdevice.width - 1);
	b.Y = (b.Y + 1) / 2 * (graphicsdevice.height - 1);
	c.X = (c.X + 1) / 2 * (graphicsdevice.width - 1);
	c.Y = (c.Y + 1) / 2 * (graphicsdevice.height - 1);

	//按照文中的方法进行排序
	const Point4* pa = &a;
	const Point4* pb = &b;
	const Point4* pc = &c;

	const std::vector<double>* va = &sva;
	const std::vector<double>* vb = &svb;
	const std::vector<double>* vc = &svc;
	//冒泡排序，三个数要比三次
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point4* tmp_p = pa;
		const std::vector<double>* tmp_v = va;
		pa = pb;
		va = vb;
		pb = tmp_p;
		vb = tmp_v;
	}
	if (pb->Y > pc->Y) //使得p3的y值不小于p2的y值
	{
		const Point4* tmp_p = pb;
		const std::vector<double>* tmp_v = vb;
		pb = pc;
		vb = vc;
		pc = tmp_p;
		vc = tmp_v;
	}
	if (pa->Y > pb->Y) //使得p2的y值不小于p1的y值
	{
		const Point4* tmp_p = pa;
		const std::vector<double>* tmp_v = va;
		pa = pb;
		va = vb;
		pb = tmp_p;
		vb = tmp_v;
	}
	double p_t = (pb->Y - pa->Y) / (pc->Y - pa->Y);						   //得到屏幕空间点P'的t'值(c'点的权值)
	double p_omega = (pa->W * pc->W) / (pc->W + (pa->W - pc->W) * p_t);	   //使用线性插值计算ω分量
	double p_z = pa->Z + (pc->Z - pa->Z) * p_t;							   //使用线性插值计算z分量
	Point4 p = Point4(pa->X + (pc->X - pa->X) * p_t, pb->Y, p_z, p_omega); //得到P'
	std::vector<double> varying_p = std::vector<double>(varying_length);
	for (int i = 0; i < varying_length; i++) //使用透视校正插值p的每个属性值
	{
		varying_p[i] = (pc->W * (*va)[i] + (pa->W * (*vc)[i] - pc->W * (*va)[i]) * p_t) / (pc->W + (pa->W - pc->W) * p_t);
	}
	const Point4* _p = &p;
	const std::vector<double>* _v = &varying_p;
	if (pb->X > _p->X) //让_p永远在右侧
	{
		const Point4* tmp_p = pb;
		const std::vector<double>* tmp_v = vb;
		pb = _p;
		vb = _v;
		_p = tmp_p;
		_v = tmp_v;
	}
	const Point4& P1 = *pa;
	const Point4& P2 = *pb;
	const Point4& P3 = *pc;
	const Point4& P_ = *_p;

	const std::vector<double>& V1 = *va;
	const std::vector<double>& V2 = *vb;
	const std::vector<double>& V3 = *vc;
	const std::vector<double>& V_ = *_v;
	//排序完毕

	//单条扫描线起始和结束x值
	double start_xs[2] = { P1.X, P2.X };
	double end_xs[2] = { P1.X, P_.X };

	//记录每条边结束的x值
	double edge_left_ex[] = { P2.X, P3.X };
	double edge_right_ex[] = { P_.X, P3.X };

	//记录每条边起始和结束y值
	double start_ys[2] = { P1.Y, P2.Y };
	double end_ys[2] = { P2.Y, P3.Y };

	//记录每条边起始和结束z值
	double start_zs[2] = { P1.Z, P2.Z };
	double end_zs[2] = { P1.Z, P_.Z };

	//计算各边x在y上的增量
	double dx_lefts[2] = { (P1.X - P2.X) / (P1.Y - P2.Y), (P2.X - P3.X) / (P2.Y - P3.Y) };
	double dx_rights[2] = { (P1.X - P_.X) / (P1.Y - P_.Y), (P_.X - P3.X) / (P_.Y - P3.Y) };

	//计算各边z在y上的增量
	double dz_lefts[] = { (P2.Z - P1.Z) / (P2.Y - P1.Y), (P3.Z - P2.Z) / (P3.Y - P2.Y) };
	double dz_rights[] = { (P_.Z - P1.Z) / (P_.Y - P1.Y), (P3.Z - P_.Z) / (P3.Y - P_.Y) };

	//计算各边t在y上的增量
	double dt_lefts[] = { 1 / (P2.Y - P1.Y), 1 / (P3.Y - P2.Y) };
	double dt_rights[] = { 1 / (P_.Y - P1.Y), 1 / (P3.Y - P_.Y) };

	//记录每条边起始和结束的ω值
	double left_start_ws[2] = { P1.W, P2.W };
	double left_end_ws[2] = { P2.W, P3.W };
	double right_start_ws[2] = { P1.W, P_.W };
	double right_end_ws[2] = { P_.W, P3.W };

	//记录每条边起始和结束的varying
	const std::vector<double>* left_start_varyings[2] = { &V1, &V2 };
	const std::vector<double>* left_end_varyings[2] = { &V2, &V3 };
	const std::vector<double>* right_start_varyings[2] = { &V1, &V_ };
	const std::vector<double>* right_end_varyings[2] = { &V_, &V3 };

	std::vector<double> svs(varying_length); //扫描线起点varying集合
	std::vector<double> evs(varying_length); //扫描线终点varying集合
	std::vector<double> vs(varying_length);	 //当前像素varying集合

	for (int i = 0; i < 2; i++) //i=0和1时分别表示下半三角形和上半三角形
	{
		double dx_left = dx_lefts[i];
		double dx_right = dx_rights[i];

		double sx = start_xs[i]; //扫描线起点X值
		double ex = end_xs[i];	 //扫描线终点X值

		double sy = start_ys[i]; //三角形起始扫描线
		double ey = end_ys[i];	 //三角形结束扫描线

		double sz = start_zs[i];
		double ez = end_zs[i];

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
				sx = _min(sx, edge_left_ex[i]); //将sx限定为不大于终点x
			}
			if (dx_right < 0) //ex随着y增大而减小
			{
				ex = _max(ex, edge_right_ex[i]); //将ex限定为不小于终点x
			}
			else if (dx_right > 0) //ex随着y增大而增大
			{
				ex = _min(ex, edge_right_ex[i]); //将ex限定为不大于终点x
			}
			double lwa = left_start_ws[i];
			double lwb = left_end_ws[i];

			double rwa = right_start_ws[i];
			double rwb = right_end_ws[i];
			//本行扫描线起始和结束的ω值
			double sw = (lwa * lwb) / (lwb + (lwa - lwb) * t_left);
			double ew = (rwa * rwb) / (rwb + (rwa - rwb) * t_right);

			for (int vindex = 0; vindex < varying_length; vindex++) //计算每条扫描线的起始和结束属性值
			{
				double lva = (*left_start_varyings[i])[vindex];
				double lvb = (*left_end_varyings[i])[vindex];
				svs[vindex] = (lwb * lva + (lwa * lvb - lwb * lva) * t_left) / (lwb + (lwa - lwb) * t_left);

				double rva = (*right_start_varyings[i])[vindex];
				double rvb = (*right_end_varyings[i])[vindex];
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
				for (int vindex = 0; vindex < varying_length; vindex++) //计算每个顶点的属性值
				{
					double va = svs[vindex];
					double vb = evs[vindex];
					vs[vindex] = (wb * va + (wa * vb - wb * va) * t) / (wb + (wa - wb) * t);
				}
				COLORREF color;
				FragmentShader(vs, color);
				graphicsdevice.SetPixel_Back(x, y, color);
			}
			sx = sx + dx_left;
			ex = ex + dx_right;

			t_left += dt_lefts[i];
			t_right += dt_rights[i];

			t_left = min(1, t_left);//非常简单的bug修复，因为按照分析，t值在递增后可能大于1(在计算最后一条扫描线的时候),所以我们将t限制在小于1即可
			t_right = min(1, t_right);

			sz += dz_lefts[i];
			ez += dz_rights[i];
		}
	}
}