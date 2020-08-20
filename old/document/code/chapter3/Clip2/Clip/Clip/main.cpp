#include "Graphics.h"
#include "Point2.h"
#include <stdio.h>
#include <list>//为了方便，使用了STL的list，这样就不用自己手写链表了，本文主要是教图形学而不是数据结构
#include <deque>
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
void FillPolygon(Graphics& gp, Point2* ps, int count)
{
	if (count < 3)//顶点数量低于3个无法围成一个多边形
	{
		return;
	}
	std::list<Edge>* NET = new std::list<Edge>[gp.HEIGHT];//创建NET
	int YMIN = (int)ps[0].Y;
	int YMAX = (int)ps[0].Y;
	//往NET中填充数据
	for (int i = 0; i < count; i++)//对每条边都会被判断两次，选择Y值小的点作为起点，Y值大的点作为终点，平行于扫描线的边被放弃,并且因为放弃了与扫描线平行的边，所以也不会出现dx等于无穷大的情况
	{
		double x, dx, ymax;
		if (ps[i].Y > ps[(i + 1) % count].Y)//以pi+1作为起点,pi作为终点
		{
			x = ps[(i + 1) % count].X;
			dx = (ps[(i + 1) % count].X - ps[i].X) / (ps[(i + 1) % count].Y - ps[i].Y);
			ymax = ps[i].Y;
			NET[(int)ps[(i + 1) % count].Y].push_back(Edge(x, dx, ymax));
		}
		else if (ps[i].Y < ps[(i + 1) % count].Y)//以pi作为起点，pi+1作为终点
		{
			x = ps[i].X;
			dx = (ps[(i + 1) % count].X - ps[i].X) / (ps[(i + 1) % count].Y - ps[i].Y);
			ymax = ps[(i + 1) % count].Y;
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
						gp.setPixel(x, y, WHITE);
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
/*
使用和坐标轴平行的直线裁剪一条直线
本函数如果裁剪了Line,会修改传入参数line的坐标，修改A点时返回1,修改B点时返回2，平凡接受时返回0，平凡拒绝返回-1
flage取值可以为0,1,2,3，分别表示left top right botom
value表示边界值
line为Point2[2]被裁剪的直线
*/
int clipParallelBoundary(int flage,double value,Point2 *line)
{
	bool clipA = false;//被裁剪线段的a点是否被裁剪标记
	bool clipB = false;//被裁剪线段的b点是否被裁剪标记
	double x;//裁剪之后的x坐标
	double y;//裁剪之后的y坐标
	if(flage==0|| flage == 2)//left和right边界
	{
		if (flage == 0)//边界条件是X<value则裁剪
		{
			if (line[0].X < value)
			{
				clipA = true;
			}
			if (line[1].X < value)
			{
				clipB = true;
			}
		}
		else//边界条件是X>value则裁剪
		{
			if (line[0].X > value)
			{
				clipA = true;
			}
			if (line[1].X > value)
			{
				clipB = true;
			}
		}
		if (!clipA && !clipB)//平凡接受
		{
			return 0;
		}
		else if (clipA && clipB)//平凡拒绝
		{
			return -1;
		}
		//使用x=value对线段进行裁剪
		x = value;//x值为边界
		y = (x - line[0].X) / (line[1].X - line[0].X) * (line[1].Y - line[0].Y)+line[0].Y;//已知x，可以用两点式直接计算出y值
	}
	else if (flage == 1 || flage == 3)
	{
		if (flage == 1)//边界条件是Y>value则裁剪
		{
			if (line[0].Y > value)
			{
				clipA = true;
			}
			if (line[1].Y > value)
			{
				clipB = true;
			}
		}
		else//边界条件是Y<value则裁剪
		{
			if (line[0].Y < value)
			{
				clipA = true;
			}
			if (line[1].Y < value)
			{
				clipB = true;
			}
		}
		if (!clipA && !clipB)//平凡接受
		{
			return 0;
		}
		else if (clipA && clipB)//平凡拒绝
		{
			return -1;
		}
		y = value;//y值为边界
		x = (y - line[0].Y) / (line[1].Y - line[0].Y) * (line[1].X - line[0].X) + line[0].X;//已知y，可以用两点式直接计算出x值

	}
	if (clipA)
	{
		line[0].X = x;
		line[0].Y = y;
		return 1;
	}
	else
	{
		line[1].X = x;
		line[1].Y = y;
		return 2;
	}
}
/*
使用一个凸多边形作为裁剪窗口
*/
std::deque<Point2> clip(double left,double top,double right,double bottom, Point2* Polygon, int countOfpolygon)//countOfwindow：裁剪窗口的顶点数量，countOfpolygon：多边形的顶点数量
{
	std::deque<Point2> tmp;
	for (int i=0;i< countOfpolygon;i++)
	{
		tmp.push_back(Polygon[i]);
	}
	double boundary[] = {left,top,right,bottom};
	for (int j=0;j<4;j++)
	{
		std::deque<Point2> points;
		size_t size = tmp.size();
		//tmp存放的是本次裁剪的多边形顶点集合
		for (size_t i = 0; i < size; i++)//使用和坐标轴平行的一条边作为裁剪边界
		{
			Point2 line[] = { tmp[i], tmp[(i + 1) % size] };//构建一条被裁剪的线段
			int ret = clipParallelBoundary(j, boundary[j], line);//对线段进行裁剪
			if (ret == 0)//平凡接受
			{
				points.push_back(line[1]);
			}
			else if (ret == 1)//入点
			{
				points.push_back(line[0]);
				points.push_back(line[1]);
			}
			else if (ret == 2)//出点
			{
				points.push_back(line[1]);
			}
		}
		tmp.clear();
		tmp.assign(points.begin(), points.end());//把本次裁剪的多边形结果用于下条边界的裁剪
	}
	//最后tmp就是裁剪的结果
	return tmp;
}
int main()
{
	Point2 Polygon[] = { {0,0},{100,0},{100,100},{0,100} };//被裁剪多边形
 	auto tmp=clip(20,40,40,20, Polygon, 4);//裁剪
	int count = (int)tmp.size();
	Point2* result = new Point2[count];//将裁剪结果复制到数组
	for (int i = 0; i < count; i++)
	{
		result[i] = tmp[i];
	}
	Graphics gp(640, 480);//创建绘图窗口
	FillPolygon(gp, result, count);//绘制多边形
	delete[] result;
	getchar();
}