#include "Graphics.h"
#include "Point2.h"
#include <stdio.h>
#include <list>//为了方便，使用了STL的list，这样就不用自己手写链表了，本文主要是教图形学而不是数据结构
#include <deque>
#include "Vector3.h"
#include <chrono>
#pragma warning(disable:4996)
struct Edge//边结构
{
	double X;
	double DX;
	double YMAX;
	//uv分别表示纹理的uv坐标
	double U;//属性的起始值
	double V;
	double DU;//记录当前边上属性在不同扫描线上面的增量
	double DV;
	Edge(double x, double dx, double ymax, double u, double v, double du, double dv) :X(x), DX(dx), YMAX(ymax), U(u), V(v), DU(du), DV(dv)
	{}
};
COLORREF picture[2][2] = {//创建一个2*2的二值图片
	{RGB(255,0,0),RGB(255,255,255)},
	{RGB(0,255,0),RGB(0,0,255)}
};
//获取图片颜色
COLORREF getPixel(int x, int y)
{
	return picture[x][y];
}
//把uv坐标转换成xy坐标
void UV2XY(double u, double v, double& x, double& y, double w, double h)
{
	x = u * w;
	y = v * h;
	x = min(max(x, 0), w);//把x限制在[0,w]区间
	y = min(max(y, 0), h);//把y限制在[0,h]区间
}
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
void FillTriangle(Graphics& gp, Point2* ps, double* coordinate)
{
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
		double u, v, du, dv;//纹理uv在不同扫描线上面的增量
		if (ps[i].Y > ps[(i + 1) % 3].Y)//以pi+1作为起点,pi作为终点
		{
			x = ps[(i + 1) % 3].X;
			dx = (ps[(i + 1) % 3].X - ps[i].X) / (ps[(i + 1) % 3].Y - ps[i].Y);
			ymax = ps[i].Y;
			u = coordinate[((i + 1) % 3) * 2];
			v = coordinate[((i + 1) % 3) * 2 + 1];
			du = (coordinate[((i + 1) % 3) * 2] - coordinate[(i % 3) * 2]) / (ps[(i + 1) % 3].Y - ps[i].Y);
			dv = (coordinate[((i + 1) % 3) * 2 + 1] - coordinate[(i % 3) * 2 + 1]) / (ps[(i + 1) % 3].Y - ps[i].Y);
			NET[(int)ps[(i + 1) % 3].Y].push_back(Edge(x, dx, ymax, u, v, du, dv));
		}
		else if (ps[i].Y < ps[(i + 1) % 3].Y)//以pi作为起点，pi+1作为终点
		{
			x = ps[i].X;
			dx = (ps[(i + 1) % 3].X - ps[i].X) / (ps[(i + 1) % 3].Y - ps[i].Y);
			ymax = ps[(i + 1) % 3].Y;
			u = coordinate[(i % 3) * 2];
			v = coordinate[(i % 3) * 2 + 1];
			du = (coordinate[((i + 1) % 3) * 2] - coordinate[(i % 3) * 2]) / (ps[(i + 1) % 3].Y - ps[i].Y);
			dv = (coordinate[((i + 1) % 3) * 2 + 1] - coordinate[(i % 3) * 2 + 1]) / (ps[(i + 1) % 3].Y - ps[i].Y);
			NET[(int)ps[i].Y].push_back(Edge(x, dx, ymax, u, v, du, dv));
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
					double su = edgeStar->U;//取得当前扫描线上面X,U,V初始值和结束值
					double eu = edgeEnd->U;
					double sv = edgeStar->V;
					double ev = edgeEnd->V;
					double sx = edgeStar->X;
					double ex = edgeEnd->X;

					double dxu, dxv;
					dxu = (eu - su) / (ex - sx);//计算属性在扫描线上面的增量
					dxv = (ev - sv) / (ex - sx);

					int x = sx;//初始化当前像素的x,u,v属性值
					double u = su;
					double v = sv;
					for (; x < ex; x++)//绘制这对交点组成的线段
					{
						double px, py;//图片的x,y
						UV2XY(u, v, px, py, 2, 2);//将uv转换成xy坐标
						COLORREF c = getPixel((int)px, (int)py);
						gp.setPixel(x, y, c);//绘制像素
						u += dxu;
						v += dxv;
					}

					//绘制完成一条扫描线，将这对交点的x、u、v属性值增加dx
					edgeStar->X += edgeStar->DX;
					edgeEnd->X += edgeEnd->DX;

					edgeStar->U += edgeStar->DU;
					edgeEnd->U += edgeEnd->DU;
					edgeStar->V += edgeStar->DV;
					edgeEnd->V += edgeEnd->DV;
				}
				it++;
			}
		}
	}
	delete[] NET;
}

int main()
{
	Point2 Polygon[] = { {0,0},{400,0},{400,400} };
	double coordinate[] = {//纹理坐标
		0,0,1,0,1,1
	};
	Graphics gp(640, 480);//创建绘图窗口
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	FillTriangle(gp, Polygon, coordinate);//绘制三角形
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	double useTime = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;//花费时间
	char msg[256];//往调试器打印信息用的缓冲区
	sprintf(msg, "Release版本，双线性插值绘制耗时:%lf 毫秒\n", useTime * 1000);
	OutputDebugStringA(msg);//往调试器输出两帧绘制时间间隔
	getchar();
}