#include "Graphics.h"
#include "Point2.h"
#include <stdio.h>
#include <list>//为了方便，使用了STL的list，这样就不用自己手写链表了，本文主要是教图形学而不是数据结构
struct Edge//边结构
{
	double X;
	double DX;
	double YMAX;
	Edge(double x,double dx,double ymax):X(x),DX(dx),YMAX(ymax)
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
void FillPolygon(Graphics& gp,Point2* ps,int count)
{
	if (count < 3)//顶点数量低于3个无法围成一个多边形
	{
		return;
	}
	std::list<Edge>* NET= new std::list<Edge>[gp.HEIGHT];//创建NET
	int YMIN = (int)ps[0].Y;
	int YMAX = (int)ps[0].Y;
	//往NET中填充数据
	for (int i=0;i<count;i++)//对每条边都会被判断两次，选择Y值小的点作为起点，Y值大的点作为终点，平行于扫描线的边被放弃,并且因为放弃了与扫描线平行的边，所以也不会出现dx等于无穷大的情况
	{
		double x, dx, ymax;
		if (ps[i].Y > ps[(i + 1) % count].Y)//以pi+1作为起点,pi作为终点
		{
			x = ps[(i + 1) % count].X;
			dx = (ps[(i + 1) % count].X- ps[i].X)/(ps[(i + 1) % count].Y- ps[i].Y);
			ymax = ps[i].Y;
			NET[(int)ps[(i + 1) % count].Y].push_back(Edge(x,dx,ymax));
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
	for (int y=YMIN;y<YMAX;y++)//针对多边形覆盖区域的每条扫描线处理
	{
		if (!NET[y].empty())//如果当前扫描线对应的NET不为空
		{
			AEL.splice(AEL.end(), NET[y]);//将其添加到AEL中
		}
		AEL.sort(SortEdge);//将交点排序
		std::list<Edge>::iterator edgeStar, edgeEnd;//成对取出的交点
		int counterOfedge=0;//取出交点的计数器，方便成对取出交点
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
					for (int x = (int)edgeStar->X;x<edgeEnd->X;x++)//绘制这对交点组成的线段
					{
						gp.setPixel(x,y,WHITE);
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

int main()
{
	Graphics gp(640,480);
	Point2 ps[] = { {10,10},{150,10},{60,100},{100,100} };
	FillPolygon(gp, ps, 4);
	getchar();
}