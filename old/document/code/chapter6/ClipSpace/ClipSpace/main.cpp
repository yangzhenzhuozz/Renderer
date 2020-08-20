#include "Graphics.h"
#include "Point2.h"
#include "Point3.h"
#include "Point4.h"
#include <stdio.h>
#include <list>//为了方便，使用了STL的list，这样就不用自己手写链表了，本文主要是教图形学而不是数据结构
#include <deque>
#include "Vector3.h"
struct Edge//边结构
{
	double X;
	double DX;
	double YMAX;
	Edge(double x, double dx, double ymax) :X(x), DX(dx), YMAX(ymax)
	{}
};
//边排序代码，给std::list排序时调用
bool SortEdge(Edge const& E1, Edge const& E2)//将边表排序，按X增序排序，如果X一样，则按照dx增序排序
{
	if (E1.X != E2.X)
	{
		return E1.X < E2.X;
	}
	else
	{
		return E1.DX < E2.DX;
	}
}
//使用多边形填充函数简单的改造了一下，把所有的count换成了3
void FillTriangle(Graphics& gp, Point2* Triangle, double* Colors)
{
	Point2 ps[3];
	for (int i = 0; i < 3; i++)
	{
		ps[i].X = (Triangle[i].X + 1) / 2 * (gp.WIDTH - 1);//实际行号取值范围为[0,Height-1]
		ps[i].Y = (Triangle[i].Y + 1) / 2 * (gp.HEIGHT - 1);//实际列号取值范围为[0,Height-1]
	}
	Vector3 ab(ps[1].X - ps[0].X, ps[1].Y - ps[0].Y, 0.0);//ps[0]->ps[1]
	Vector3 bc(ps[2].X - ps[1].X, ps[2].Y - ps[1].Y, 0.0);//ps[1]->ps[2]
	Vector3 ca(ps[0].X - ps[2].X, ps[0].Y - ps[2].Y, 0.0);//ps[2]->ps[0]
	Vector3 ac(ps[2].X - ps[0].X, ps[2].Y - ps[0].Y, 0.0);//ps[2]->ps[0]
	double square = ab.X * ac.Y - ab.Y * ac.X;//得到三角形有向面积的2倍
	if (square == 0)//三角形面积为0则不进行后面的绘制处理
	{
		return;
	}


	std::list<Edge>* NET = new std::list<Edge>[gp.HEIGHT];//创建NET
	int YMIN = (int)ps[0].Y;
	int YMAX = (int)ps[0].Y;
	//往NET中填充数据
	for (int i = 0; i < 3; i++)//对每条边都会被判断两次，选择Y值小的点作为起点，Y值大的点作为终点，平行于扫描线的边被放弃,并且因为放弃了与扫描线平行的边，所以也不会出现dx等于无穷大的情况
	{
		double x, dx, ymax;
		if (ps[i].Y > ps[(i + 1) % 3].Y)//以pi+1作为起点,pi作为终点
		{
			x = ps[(i + 1) % 3].X;
			dx = (ps[(i + 1) % 3].X - ps[i].X) / (ps[(i + 1) % 3].Y - ps[i].Y);
			ymax = ps[i].Y;
			NET[(int)ps[(i + 1) % 3].Y].push_back(Edge(x, dx, ymax));
		}
		else if (ps[i].Y < ps[(i + 1) % 3].Y)//以pi作为起点，pi+1作为终点
		{
			x = ps[i].X;
			dx = (ps[(i + 1) % 3].X - ps[i].X) / (ps[(i + 1) % 3].Y - ps[i].Y);
			ymax = ps[(i + 1) % 3].Y;
			NET[(int)ps[i].Y].push_back(Edge(x, dx, ymax));
		}
		else//平行于扫描线的边
		{
		}
		YMIN = (int)min(ps[i].Y, YMIN);//记录多边形的最小Y值
		YMAX = (int)max(ps[i].Y, YMAX);//记录多边形的最大Y值
	}
	std::list<Edge> AEL;
	for (int y = YMIN; y < YMAX; y++)//针对多边形覆盖区域的每条扫描线处理
	{
		if (!NET[y].empty())//如果当前扫描线对应的NET不为空
		{
			AEL.splice(AEL.end(), NET[y]);//将其添加到AEL中
			AEL.sort(SortEdge);//将交点排序
		}
		std::list<Edge>::iterator edgeStar, edgeEnd;//成对取出的交点
		int counterOfedge = 0;//取出交点的计数器，方便成对取出交点
		for (std::list<Edge>::iterator it = AEL.begin(); it != AEL.end();)//遍历AEL中的每个交点
		{
			if ((int)it->YMAX <= y)
			{
				it = AEL.erase(it);//当前扫描线已经超过it这条边的Ymax,将it边删除，因为执行erase会自动将指针往后移动一个元素，所以这里就不用执行it++了
			}
			else
			{
				if (counterOfedge == 0)//当前取出的是一对交点中的前一个
				{
					edgeStar = it;
					counterOfedge++;
				}
				else//当前取的是一对交点点中的后一个，已经配成对了
				{
					edgeEnd = it;
					counterOfedge = 0;
					for (int x = (int)edgeStar->X; x < edgeEnd->X; x++)//绘制这对交点组成的线段
					{
						double WeightA, WeightB, WeightC;
						Vector3 bp(x - ps[1].X, y - ps[1].Y, 0.0);
						Vector3 ap(x - ps[0].X, y - ps[0].Y, 0.0);
						Vector3 cp(x - ps[2].X, y - ps[2].Y, 0.0);
						WeightA = (bc.X * bp.Y - bc.Y * bp.X) / square;
						WeightB = (ca.X * cp.Y - ca.Y * cp.X) / square;
						WeightC = (ab.X * ap.Y - ab.Y * ap.X) / square;//得到三个顶点的权值
						BYTE cr, cg, cb;
						cr = (BYTE)(WeightA * Colors[0] + WeightB * Colors[3] + WeightC * Colors[6]);
						cg = (BYTE)(WeightA * Colors[1] + WeightB * Colors[4] + WeightC * Colors[7]);
						cb = (BYTE)(WeightA * Colors[2] + WeightB * Colors[5] + WeightC * Colors[8]);//计算RGB的插值结果
						gp.setPixel(x, y, RGB(cr, cg, cb));//绘制像素
					}
					//将这对交点的x值增加dx
					edgeStar->X += edgeStar->DX;
					edgeEnd->X += edgeEnd->DX;
				}
				it++;
			}
		}
	}
	delete[] NET;
}
//投影函数,分别传递一个点和near平面，计算出新投影点并将结果保存至一个齐次坐标中
void Projection(Point3& p, double l, double r, double b, double t, double n, double f, Point4& result)
{
	result.X = (2 * n * p.X) / (r - l) - p.Z * (r + l) / (r - l);
	result.Y = (2 * n * p.Y) / (t - b) - p.Z * (t + b) / (t - b);
	result.Z = (f + n) / (f - n) * p.Z + 2 * f * n / (n - f);
	result.W = p.Z;
}
//对直线进行裁剪,返回-1表示平凡拒绝，返回0表示平凡接受，返回1表示对A点进行裁剪，返回2表示对B点进行裁剪，最后两个参数result和t表示裁剪点坐标和t值
//其中参数flag从0到5分别表示边界为left、right、bottom、top、near、far
int ClipLine(int flag, Point4& A, Point4& B, Point4& result, double& t)
{
	double aClipCondition = 0;
	double bClipCondition = 0;
	switch (flag)
	{
	case 0:
		aClipCondition = A.W + A.X;
		bClipCondition = B.W + B.X;
		break;
	case 1:
		aClipCondition = A.W - A.X;
		bClipCondition = B.W - B.X;
		break;
	case 2:
		aClipCondition = A.W + A.Y;
		bClipCondition = B.W + B.Y;
		break;
	case 3:
		aClipCondition = A.W - A.Y;
		bClipCondition = B.W - B.Y;
		break;
	case 4:
		aClipCondition = A.W + A.Z;
		bClipCondition = B.W + B.Z;
		break;
	case 5:
		aClipCondition = A.W - A.Z;
		bClipCondition = B.W - B.Z;
		break;
	default:
		return 0;
	}
	if (aClipCondition < 0 && bClipCondition < 0)//平凡拒绝
	{
		return -1;
	}
	if (aClipCondition >= 0 && bClipCondition >= 0)//平凡接受
	{
		return 0;
	}
	t = aClipCondition / (aClipCondition - bClipCondition);//计算交点的t值
	result.X = A.X + (B.X - A.X) * t;//计算交点
	result.Y = A.Y + (B.Y - A.Y) * t;
	result.Z = A.Z + (B.Z - A.Z) * t;
	result.W = A.W + (B.W - A.W) * t;
	if (aClipCondition < 0)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}
void ClipTriangleAndDraw(Point4 ps[3], double* attribute, Graphics& gp)//裁剪并且绘制三角形
{
	std::deque<Point4> tmp;//把顶点复制到一个临时容器中
	std::deque<double> tmp_a;//把顶点属性复制到一个临时容器中
	for (int i = 0; i < 3; i++)
	{
		tmp.push_back(ps[i]);
		tmp_a.push_back(attribute[i * 3]);//复制属性值
		tmp_a.push_back(attribute[i * 3 + 1]);
		tmp_a.push_back(attribute[i * 3 + 2]);
	}
	//依次使用六个边界轮流裁剪
	for (int clipflag = 0; clipflag < 6; clipflag++)
	{
		std::deque<Point4> points;
		std::deque<double> attributes;
		size_t size = tmp.size();
		//依次裁剪size条边
		for (int i = 0; i < size; i++)
		{
			Point4 result;
			double t;
			int ret = ClipLine(clipflag, tmp[i], tmp[((size_t)i + 1) % size], result, t);//对线段进行裁剪
			if (ret == 0)//平凡接受
			{
				points.push_back(tmp[((size_t)i + 1) % size]);//存放顶点坐标
				attributes.push_back(tmp_a[((size_t)i + 1) % size * 3]);//存放顶点属性
				attributes.push_back(tmp_a[((size_t)i + 1) % size * 3 + 1]);
				attributes.push_back(tmp_a[((size_t)i + 1) % size * 3 + 2]);
			}
			else if (ret == 1)//入点被裁剪
			{
				points.push_back(result);//存放顶点坐标
				attributes.push_back(tmp_a[i * 3] + (tmp_a[((size_t)i + 1) % size * 3] - tmp_a[i * 3]) * t);//存放顶点属性
				attributes.push_back(tmp_a[i * 3 + 1] + (tmp_a[((size_t)i + 1) % size * 3 + 1] - tmp_a[i * 3 + 1]) * t);
				attributes.push_back(tmp_a[i * 3 + 2] + (tmp_a[((size_t)i + 1) % size] - tmp_a[i * 3 + 2]) * t);
				points.push_back(tmp[((size_t)i + 1) % size]);//存放顶点坐标
				attributes.push_back(tmp_a[((size_t)i + 1) % size * 3]);//存放顶点属性
				attributes.push_back(tmp_a[((size_t)i + 1) % size * 3 + 1]);
				attributes.push_back(tmp_a[((size_t)i + 1) % size * 3 + 2]);
			}
			else if (ret == 2)//出点被裁剪
			{
				points.push_back(result);//存放顶点坐标
				attributes.push_back(tmp_a[i * 3] + (tmp_a[((size_t)i + 1) % size * 3] - tmp_a[i * 3]) * t);//存放顶点属性
				attributes.push_back(tmp_a[i * 3 + 1] + (tmp_a[((size_t)i + 1) % size * 3 + 1] - tmp_a[i * 3 + 1]) * t);
				attributes.push_back(tmp_a[i * 3 + 2] + (tmp_a[((size_t)i + 1) % size * 3 + 2] - tmp_a[i * 3 + 2]) * t);
			}
		}
		tmp.clear();
		tmp.assign(points.begin(), points.end());//把本次裁剪的多边形结果用于下条边界的裁剪
		tmp_a.clear();
		tmp_a.assign(attributes.begin(), attributes.end());
	}
	//最后tmp和tmp_a就是裁剪的结果
	for (int i = 1; i < tmp.size() - 1; i++)//将裁剪之后的多边形细分成多个三角形
	{
		Point2 Triangle[3];
		double attributes[9];
		Triangle[0].X = tmp[0].X / tmp[0].W;//执行透视除法
		Triangle[0].Y = tmp[0].Y / tmp[0].W;
		Triangle[1].X = tmp[i].X / tmp[i].W;
		Triangle[1].Y = tmp[i].Y / tmp[i].W;
		Triangle[2].X = tmp[i + 1].X / tmp[i + 1].W;
		Triangle[2].Y = tmp[i + 1].Y / tmp[i + 1].W;

		attributes[0] = tmp_a[0];//填充属性值
		attributes[1] = tmp_a[0 + 1];
		attributes[2] = tmp_a[0 + 2];

		attributes[3] = tmp_a[i * 3];
		attributes[4] = tmp_a[i * 3 + 1];
		attributes[5] = tmp_a[i * 3 + 2];

		attributes[6] = tmp_a[(i + 1) * 3];
		attributes[7] = tmp_a[(i + 1) * 3 + 1];
		attributes[8] = tmp_a[(i + 1) * 3 + 2];

		FillTriangle(gp, Triangle, attributes);
	}
}
int main()
{
	Graphics gp(640, 480);//创建绘图窗口
	double Color[] = {
	255,0,0,
	0,255,0,
	0,0,255
	};
	Point3 ps[] = { {-640,-480,50},{0,480,2},{640,-480,50} };
	Point4 result[3];
	for (int i = 0; i < 3; i++)//执行透视乘法
	{
		Projection(ps[i], -320, 320, -240, 240, 25, 50, result[i]);
	}
	ClipTriangleAndDraw(result, Color, gp);
	getchar();
}