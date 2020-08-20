#include "Graphics.h"
#include "Vector3.h"
#include <wingdi.h>
#include <stdio.h>
#include <io.h>
#include <algorithm>
#include <intrin.h> 
#pragma warning(disable:4996)

GraphicsLibrary::GraphicsLibrary(unsigned int w, unsigned int h) :ScreenWidth(w), ScreenHeight(h)
{
	initgraph(w, h);
	BeginBatchDraw();
	g_pBuf = GetImageBuffer(NULL);
	FragmentShader = NULL;
	VertexShader = NULL;
	TextureHeight = 0;
	TextureWidth = 0;
	vboBuffer = NULL;
	Z_Buffer = new double[(size_t)w * h];
	NET = new std::list<Edge>[ScreenHeight];
}
// 快速画点函数,复制于官网教程https://codeabc.cn/yangw/post/the-principle-of-quick-drawing-points
void GraphicsLibrary::fast_putpixel(int x, int y, COLORREF c)
{
	g_pBuf[y * ScreenWidth + x] = BGR(c);
}

// 快速读点函数,复制于官网教程https://codeabc.cn/yangw/post/the-principle-of-quick-drawing-points
COLORREF GraphicsLibrary::fast_getpixel(int x, int y)
{
	COLORREF c = g_pBuf[y * ScreenWidth + x];
	return BGR(c);
}

bool GraphicsLibrary::loadBMP(const char* filename)
{
	errmsg[0] = '\0';//清空错误信息
	char tmp[1024];
	bool isError = false;
	FILE* file = fopen(filename, "rb");
	if (file)
	{
		long size = filelength(fileno(file));
		textureBuffer = new unsigned char[size];
		fread(textureBuffer, size, 1, file);//读取文件到内存
		fclose(file);

		tagBITMAPFILEHEADER* fhead = (tagBITMAPFILEHEADER*)textureBuffer;
		tagBITMAPINFOHEADER* ihead = (tagBITMAPINFOHEADER*)(textureBuffer + sizeof(tagBITMAPFILEHEADER));
		if (ihead->biHeight < 0)
		{
			isError = true;
			sprintf_s(tmp, sizeof(tmp), "纵轴反向\n");
			strcat_s(errmsg, sizeof(errmsg), tmp);
		}
		if (ihead->biBitCount != 24)
		{
			isError = true;
			sprintf_s(tmp, sizeof(tmp), "不是24位位图\n");
			strcat_s(errmsg, sizeof(errmsg), tmp);
		}
		if (ihead->biCompression != 0)
		{
			isError = true;
			sprintf_s(tmp, sizeof(tmp), "位图有压缩\n");
			strcat_s(errmsg, sizeof(errmsg), tmp);
		}
		bmpHeight = ihead->biHeight;
		bmpwidth = ihead->biWidth;//保存位图宽高
		bmpData = textureBuffer + fhead->bfOffBits;
	}
	else
	{
		isError = true;
		sprintf_s(tmp, sizeof(tmp), "纹理文件打开失败\n");
		strcat_s(errmsg, sizeof(errmsg), tmp);
	}
	return !isError;
}

void GraphicsLibrary::flush()
{
	FlushBatchDraw();
}

//对边进行裁剪，这里只裁剪near平面，因为w分量>0时在栅格化的时候就处理了，和opengl还是有点不同，主要是我懒得修改栅格化的代码了
//如果z/w<-1则需要对其进行裁剪,即z+w<0则需要裁剪
//返回0表示平凡接受，返回-1表示本条边平凡拒绝,返回1表示有裁剪并且将A点挪到新点，2表示将B挪到新点,3表示AB都有移动
int GraphicsLibrary::clipEdge(Point4& A, Point4& B, Point4& tmpA, Point4& tmpB, double& proportionA, double& proportionB)
{
	int a = 0, b = 0;//对应位等于0表示接受，1表示拒绝
	double aBC[6], bBC[6];//六个边界
	aBC[0] = A.value[3] + A.value[0];//w+x left
	aBC[1] = A.value[3] - A.value[0];//w-x right
	aBC[2] = A.value[3] + A.value[1];//w+y bottom
	aBC[3] = A.value[3] - A.value[1];//w-y top
	aBC[4] = A.value[3] + A.value[2];//w+z near
	aBC[5] = A.value[3] - A.value[2];//w-z far

	bBC[0] = B.value[3] + B.value[0];//w+x left
	bBC[1] = B.value[3] - B.value[0];//w-x right
	bBC[2] = B.value[3] + B.value[1];//w+y bottom
	bBC[3] = B.value[3] - B.value[1];//w-y top
	bBC[4] = B.value[3] + B.value[2];//w+z near
	bBC[5] = B.value[3] - B.value[2];//w-z far
	for (int i = 0; i < 6; i++)
	{
		if (aBC[i] < 0)
		{
			a = a | (1 << i);
		}
		if (bBC[i] < 0)
		{
			b = b | (1 << i);
		}
	}
	if ((a | b) == 0)//A被B平凡接受：A全部包含于B(A为B的子集)
	{
		return 0;
	}
	if ((a & b) != 0)//A被B平凡拒绝：AB无交集
	{
		return -1;
	}
	double ta = 0.0, tb = 1.0, t;//t表示B点的占比，初始情况下ta中B的权值为0，tb中B的权值为1
	for (int i = 0; i < 6; i++)
	{
		if (aBC[i] < 0)//移动A点到CVV内部
		{
			t = aBC[i] / (aBC[i] - bBC[i]);
			ta = max(ta, t);
		}
		else if (bBC[i] < 0)
		{
			t = aBC[i] / (aBC[i] - bBC[i]);
			tb = min(tb, t);
		}
		if (ta > tb)//经过多个边界裁剪之后，裁剪剩余的a点比b点更加靠近原始的B点，也就是这个线段已经裁没了
		{
			return -1;//平凡拒绝
		}
	}

	proportionA = ta;
	proportionB = tb;
	int result = 0;
	if (a != 0)//A点有修改
	{
		tmpA.value[0] = A.value[0] + (B.value[0] - A.value[0]) * ta;//计算新交点
		tmpA.value[1] = A.value[1] + (B.value[1] - A.value[1]) * ta;
		tmpA.value[2] = A.value[2] + (B.value[2] - A.value[2]) * ta;
		tmpA.value[3] = A.value[3] + (B.value[3] - A.value[3]) * ta;
		result = 1;
	}
	if (b != 0)//B点有修改
	{
		tmpB.value[0] = A.value[0] + (B.value[0] - A.value[0]) * tb;//计算新交点
		tmpB.value[1] = A.value[1] + (B.value[1] - A.value[1]) * tb;
		tmpB.value[2] = A.value[2] + (B.value[2] - A.value[2]) * tb;
		tmpB.value[3] = A.value[3] + (B.value[3] - A.value[3]) * tb;
		if (result == 0)
		{
			result = 2;
		}
		else
		{
			result = 3;
		}
	}
	return result;
}
//对边进行裁剪，这里只裁剪near平面，因为w分量>0时在栅格化的时候就处理了，和opengl还是有点不同，主要是我懒得修改栅格化的代码了
//如果z/w<-1则需要对其进行裁剪,即z+w<0则需要裁剪
//返回-1表示本条边平凡拒绝,返回0表示平凡接受,返回1表示有裁剪并且将A点挪到新点，2表示将B挪到新点,3表示AB都有移动
int GraphicsLibrary::clipEdgeByParallelFace(Point4& A, Point4& B, Point4& tmpA, Point4& tmpB, double& proportionA, double& proportionB, int flag)
{
	int a = 0, b = 0;//对应位等于0表示接受，1表示拒绝
	double aBC[2], bBC[2];//六个边界
	switch (flag)
	{
	case 1:
		aBC[0] = A.value[3] + A.value[0];//w+x left
		aBC[1] = A.value[3] - A.value[0];//w-x right

		bBC[0] = B.value[3] + B.value[0];//w+x left
		bBC[1] = B.value[3] - B.value[0];//w-x right
		break;
	case 2:
		aBC[0] = A.value[3] + A.value[1];//w+y bottom
		aBC[1] = A.value[3] - A.value[1];//w-y top

		bBC[0] = B.value[3] + B.value[1];//w+y bottom
		bBC[1] = B.value[3] - B.value[1];//w-y top
		break;
	case 3:
		aBC[0] = A.value[3] + A.value[2];//w+z near
		aBC[1] = A.value[3] - A.value[2];//w-z far

		bBC[0] = B.value[3] + B.value[2];//w+z near
		bBC[1] = B.value[3] - B.value[2];//w-z far
		break;
	default:
		return 0;//不对边裁剪
	}

	for (int i = 0; i < 2; i++)
	{
		if (aBC[i] < 0)
		{
			a = a | (1 << i);
		}
		if (bBC[i] < 0)
		{
			b = b | (1 << i);
		}
	}
	if ((a | b) == 0)//A被B平凡接受：A全部包含于B(A为B的子集)
	{
		return 0;
	}
	if ((a & b) != 0)//A被B平凡拒绝：AB无交集
	{
		return -1;
	}
	double ta = 0.0, tb = 1.0, t;//t表示B点的占比，初始情况下ta中B的权值为0，tb中B的权值为1
	for (int i = 0; i < 2; i++)
	{
		if (aBC[i] < 0)//移动A点到CVV内部
		{
			t = aBC[i] / (aBC[i] - bBC[i]);
			ta = max(ta, t);
		}
		else if (bBC[i] < 0)
		{
			t = aBC[i] / (aBC[i] - bBC[i]);
			tb = min(tb, t);
		}
		if (ta > tb)//经过多个边界裁剪之后，裁剪剩余的a点比b点更加靠近原始的B点，也就是这个线段已经裁没了
		{
			return -1;//平凡拒绝
		}
	}

	proportionA = ta;
	proportionB = tb;
	int result = 0;
	if (a != 0)//A点有修改
	{
		tmpA.value[0] = A.value[0] + (B.value[0] - A.value[0]) * ta;//计算新交点
		tmpA.value[1] = A.value[1] + (B.value[1] - A.value[1]) * ta;
		tmpA.value[2] = A.value[2] + (B.value[2] - A.value[2]) * ta;
		tmpA.value[3] = A.value[3] + (B.value[3] - A.value[3]) * ta;
		result = 1;
	}
	if (b != 0)//B点有修改
	{
		tmpB.value[0] = A.value[0] + (B.value[0] - A.value[0]) * tb;//计算新交点
		tmpB.value[1] = A.value[1] + (B.value[1] - A.value[1]) * tb;
		tmpB.value[2] = A.value[2] + (B.value[2] - A.value[2]) * tb;
		tmpB.value[3] = A.value[3] + (B.value[3] - A.value[3]) * tb;
		if (result == 0)
		{
			result = 2;
		}
		else
		{
			result = 3;
		}
	}
	return result;
}
void GraphicsLibrary::clipFaceByParallelFace(Point4* ps, int pCount, Point4* resultPoint, int& resultCount, double* varying, int countofvarying, double* resultvarying, int flag)
{
	int effectivePointCount = 0;//记录裁剪之后的顶点数量,最大只能到4,类似于单条直线裁剪三角形，最多只能裁剪出一个四边形
	for (int j = 0; j < pCount; j++)//裁剪三角形的三条边，记录每条边的入点、出点和终点
	{
		double aWeightB;//新a点在B点的权值
		double bWeightB;//新b点在B点的权值
		Point4 tmpa, tmpb;
		int ret = clipEdgeByParallelFace(ps[j], ps[(j + 1) % pCount], tmpa, tmpb, aWeightB, bWeightB, flag);//裁剪边(只裁剪near平面)
		if (ret == -1)//平凡拒绝
		{
			continue;//不处理本条边
		}
		if (ret == 1)//计算新的varying值
		{
			resultPoint[effectivePointCount] = tmpa;//入点
			resultPoint[effectivePointCount + 1] = ps[(j + 1) % pCount];//终点
			for (int k = 0; k < countofvarying; k++)//设置新的varying
			{
				*(resultvarying + (size_t)effectivePointCount * countofvarying + k) = *(varying + (size_t)j * countofvarying + k) * (1 - aWeightB) + *(varying + ((size_t)j + 1) % pCount * countofvarying + k) * aWeightB;
			}
			memcpy(resultvarying + ((size_t)effectivePointCount + 1) * countofvarying, varying + ((size_t)j + 1) % pCount * countofvarying, sizeof(double) * countofvarying);
			effectivePointCount += 2;
		}
		else if (ret == 2)//计算新的varying值
		{
			resultPoint[effectivePointCount] = tmpb;//出点
			for (int k = 0; k < countofvarying; k++)//设置新的varying
			{
				*(resultvarying + (size_t)effectivePointCount * countofvarying + k) = *(varying + (size_t)j * countofvarying + k) * (1 - bWeightB) + *(varying + ((size_t)j + (size_t)1) % pCount * countofvarying + k) * bWeightB;
			}
			effectivePointCount += 1;
		}
		else if (ret == 3)
		{
			resultPoint[effectivePointCount] = tmpa;//入点
			resultPoint[effectivePointCount + 1] = tmpb;//终点
			for (int k = 0; k < countofvarying; k++)//设置新的varying
			{
				*(resultvarying + (size_t)effectivePointCount * countofvarying + k) = *(varying + (size_t)j * countofvarying + k) * (1 - aWeightB) + *(varying + ((size_t)j + 1) % pCount * countofvarying + k) * aWeightB;
				*(resultvarying + ((size_t)effectivePointCount + 1) * countofvarying + k) = *(varying + (size_t)j * countofvarying + k) * (1 - bWeightB) + *(varying + ((size_t)j + 1) % pCount * countofvarying + k) * bWeightB;
			}
			effectivePointCount += 2;
		}
		else //平凡接受
		{
			resultPoint[effectivePointCount] = ps[(j + 1) % pCount];//记录终点
			memcpy(resultvarying + (size_t)(effectivePointCount)* countofvarying, varying + ((size_t)j + 1) % pCount * countofvarying, sizeof(double) * countofvarying);
			effectivePointCount += 1;
		}
	}
	resultCount = effectivePointCount;
}
bool GraphicsLibrary::Draw()
{
	errmsg[0] = '\0';//清空错误信息
	Point4 parray[3];//position Array
	Point4 resultPoint1[9];//经过裁剪之后的边，共三条
	Point4 resultPoint2[9];//经过裁剪之后的边，共三条
	Point4 resultPoint3[9];//经过裁剪之后的边，共三条
	double* Varying = (double*)alloca(sizeof(double) * (size_t)CountOfVarying * 3);
	double* resultVarying1 = (double*)alloca(sizeof(double) * (size_t)CountOfVarying * 9);
	double* resultVarying2 = (double*)alloca(sizeof(double) * (size_t)CountOfVarying * 9);
	double* resultVarying3 = (double*)alloca(sizeof(double) * (size_t)CountOfVarying * 9);//这里是用于模拟动态数组的，如果用malloc还得使用free，会带来严重的效率降低
	for (int i = 0; i < vboCount / 3; i++)//i表示三角形数量
	{
		for (int j = 0; j < 3; j++)//对三个点进行透视乘法
		{
			VertexShader(vboBuffer + ((size_t)i * 3 * NumOfVertexVBO + (size_t)j * NumOfVertexVBO), Varying + ((size_t)j * CountOfVarying), parray[j]);//对每个顶点调用顶点着色器
		}

		//下面使用逐面裁剪，类似于Sutherland-Hodgman，被裁剪的多边形可以是任意多边形，裁剪窗口必须是凸多边形，对应到3D就是必须为凸多面体，CVV是凸多面体
		int effectivePointCount = 3;//记录裁剪之后的顶点数量,最大只能到4,类似于单条直线裁剪三角形，最多只能裁剪出一个四边形
		clipFaceByParallelFace(parray, effectivePointCount, resultPoint1, effectivePointCount, Varying, CountOfVarying, resultVarying1, 1);//使用left right平面裁剪多边形
		if (effectivePointCount == 0)
		{
			continue;//三条边都被平凡拒绝，不用绘制了
		}
		clipFaceByParallelFace(resultPoint1, effectivePointCount, resultPoint2, effectivePointCount, resultVarying1, CountOfVarying, resultVarying2, 2);//使用left right裁剪
		if (effectivePointCount == 0)
		{
			continue;//三条边都被平凡拒绝，不用绘制了
		}
		clipFaceByParallelFace(resultPoint2, effectivePointCount, resultPoint3, effectivePointCount, resultVarying2, CountOfVarying, resultVarying3, 3);//使用near far裁剪
		if (effectivePointCount == 0)
		{
			continue;//三条边都被平凡拒绝，不用绘制了
		}



		for (int k = 0; k < effectivePointCount - 2; k++)//绘制被裁剪出来的n-2个三角形，n个顶点会围城一个n边形，可以分解成n-2个三角形
		{
			if (k != 0)//将RP[0]挪到RP[1]，然后绘制RP[1],RP[2],RP[3]这个三角形
			{
				resultPoint3[k] = resultPoint3[k - 1];
				memcpy(resultVarying3 + (size_t)k * CountOfVarying, resultVarying3 + ((size_t)k - 1) * CountOfVarying, sizeof(double) * CountOfVarying);
			}
			DrawTriangle(resultPoint3 + k, resultVarying3 + (size_t)k * CountOfVarying);
		}
	}
	return true;
}

void GraphicsLibrary::clear()
{
	cleardevice();
}

void GraphicsLibrary::clearDepth(double v)
{
	std::fill(Z_Buffer, Z_Buffer + ((size_t)ScreenWidth * ScreenHeight), v);
	//memset(Z_Buffer, 0x7f, sizeof(double)*Width*Height);//用0x7f作为memset能搞定的极大值，memset应该有优化，比如调用cpu的特殊指令可以在较短的周期内赋值
}

void GraphicsLibrary::Swap()
{
	EndBatchDraw();
	BeginBatchDraw();
}

void GraphicsLibrary::setVaryingCount(int count)
{
	if (interpolationVarying != NULL)
	{
		delete[] interpolationVarying;
	}
	interpolationVarying = new double[count];
	CountOfVarying = count;
}

COLORREF GraphicsLibrary::texture2D(double x, double y)
{
	x = x - floor(x);
	y = y - floor(y);
	int CountOfRowSize = (((bmpwidth * 24) + 31) >> 5) << 2;//每行像素所占用的字节
	if (textureBuffer != NULL)
	{
		int X = (int)(x * bmpwidth);
		int Y = (int)(y * bmpHeight);
		return RGB(bmpData[Y * CountOfRowSize + X * 3 + 2], bmpData[Y * CountOfRowSize + X * 3 + 1], bmpData[Y * CountOfRowSize + X * 3]);
	}
	else
	{
		return RGB(255, 255, 255);
	}
}
//边表排序程序
bool SortEdge(Edge const& E1, Edge const& E2)//将边表排序，按X增序排序，如果X一样，则按照dx增序排序
{
	if (E1.x != E2.x)
	{
		return E1.x < E2.x;
	}
	else
	{
		return E1.dx < E2.dx;
	}
}
//本函数中插值计算都是采用double
void GraphicsLibrary::DrawTriangle(Point4* parray, double* varying)
{
	Point4 ps[3];
	for (int i = 0; i < 3; i++)
	{
		ps[i] = parray[i];
		if (ps[i].value[3] <= 0)
		{
			printf("error");
		}
		ps[i].value[0] = ps[i].value[0] / ps[i].value[3];//X,Y,Z按照齐次坐标规则正确还原，W暂时不还原，后面插值不用1/Z，改为用1/W插值
		ps[i].value[1] = ps[i].value[1] / ps[i].value[3];
		ps[i].value[2] = ps[i].value[2] / ps[i].value[3];//经过矩阵计算,W变成了原始点的-Z值

		//视口变换
		ps[i].value[0] = (ps[i].value[0] + 1) / 2 * (ScreenWidth - 1);//将ccv空间转换到视口空间
		ps[i].value[1] = (ScreenHeight - 1) - (ps[i].value[1] + 1) / 2 * (ScreenHeight - 1);
	}
	Vector3 ab(ps[1].value[0] - ps[0].value[0], ps[1].value[1] - ps[0].value[1], 0.0);//ps[0]->ps[1]
	Vector3 bc(ps[2].value[0] - ps[1].value[0], ps[2].value[1] - ps[1].value[1], 0.0);//ps[1]->ps[2]
	Vector3 ca(ps[0].value[0] - ps[2].value[0], ps[0].value[1] - ps[2].value[1], 0.0);//ps[2]->ps[0]
	Vector3 ac(ps[2].value[0] - ps[0].value[0], ps[2].value[1] - ps[0].value[1], 0.0);//ps[2]->ps[0]
	double square = ab.value[0] * ac.value[1] - ab.value[1] * ac.value[0];//得到三角形有向面积的2倍
	if (square == 0)
	{
		return;
	}
	if (enable_CW)
	{
		if (!CW_CCW)//使用顺时针绘制
		{
			if (square > 0)//实际确实逆时针
			{
				return;
			}
		}
		else//同上
		{
			if (square < 0)
			{
				return;
			}
		}
	}



	int YMIN = (int)ps[0].value[1];
	int YMAX = (int)ps[0].value[1];
	//往NET中填充数据
	for (int i = 0; i < 3; i++)//对每条边都会被判断两次，选择Y值小的点作为起点，Y值大的点作为终点，平行于扫描线的边被放弃,并且因为放弃了与扫描线平行的边，所以也不会出现dx等于无穷大的情况
	{
		double x, dx, ymax;
		if (ps[i].value[1] > ps[(i + 1) % 3].value[1])//以pi+1作为起点,pi作为终点
		{
			x = ps[(i + 1) % 3].value[0];
			dx = (ps[(i + 1) % 3].value[0] - ps[i].value[0]) / (ps[(i + 1) % 3].value[1] - ps[i].value[1]);
			ymax = ps[i].value[1];
			NET[(int)ps[(i + 1) % 3].value[1]].push_back(Edge(x, dx, ymax));
		}
		else if (ps[i].value[1] < ps[(i + 1) % 3].value[1])//以pi作为起点，pi+1作为终点
		{
			x = ps[i].value[0];
			dx = (ps[(i + 1) % 3].value[0] - ps[i].value[0]) / (ps[(i + 1) % 3].value[1] - ps[i].value[1]);
			ymax = ps[(i + 1) % 3].value[1];
			NET[(int)ps[i].value[1]].push_back(Edge(x, dx, ymax));
		}
		else//平行于扫描线的边
		{
		}
		YMIN = (int)min(ps[i].value[1], YMIN);//记录多边形的最小Y值
		YMAX = (int)max(ps[i].value[1], YMAX);//记录多边形的最大Y值
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
			if ((int)it->Ymax <= y)
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
					for (int x = (int)edgeStar->x; x < edgeEnd->x; x++)//绘制这对交点组成的线段
					{
						double WeightA, WeightB, WeightC;
						Vector3 bp(x - ps[1].value[0], y - ps[1].value[1], 0.0);
						Vector3 ap(x - ps[0].value[0], y - ps[0].value[1], 0.0);
						Vector3 cp(x - ps[2].value[0], y - ps[2].value[1], 0.0);
						WeightA = (bc.value[0] * bp.value[1] - bc.value[1] * bp.value[0]) / square;
						WeightB = (ca.value[0] * cp.value[1] - ca.value[1] * cp.value[0]) / square;
						WeightC = (ab.value[0] * ap.value[1] - ab.value[1] * ap.value[0]) / square;//得到屏幕空间中三个顶点的权值

						double z;//P点的Z值
						double Pomega = 1 / ((1 / ps[0].value[3]) * WeightA + (1 / ps[1].value[3]) * WeightB + (1 / ps[2].value[3]) * WeightC);//求出当前顶点的ω分量

						z = Pomega * (ps[0].value[2] / ps[0].value[3] * WeightA + ps[1].value[2] / ps[1].value[3] * WeightB + ps[2].value[2] / ps[2].value[3] * WeightC);//使用线性插值计算当前绘制像素的Z值
						if (z > Z_Buffer[y * ScreenWidth + x])
						{
							continue;//跳过当前像素的绘制
						}
						else
						{
							Z_Buffer[y * ScreenWidth + x] = z;//更新深度信息
						}
						for (int index = 0; index < CountOfVarying; index++)//对每个Varying插值
						{
							interpolationVarying[index] = Pomega * (varying[index] / ps[0].value[3] * WeightA + varying[index + CountOfVarying] / ps[1].value[3] * WeightB + varying[index + CountOfVarying * 2] / ps[2].value[3] * WeightC);
						}

						COLORREF c;
						FragmentShader(interpolationVarying, c);//调用片元着色器

						//因为(x,scanline)是视口坐标，所以需要加上一个(viewPortX,viewPortY)的偏移
						fast_putpixel(x, y, c);//填充颜色
					}
					//将这对交点的x值增加dx
					edgeStar->x += edgeStar->dx;
					edgeEnd->x += edgeEnd->dx;
				}
				it++;
			}
		}
	}
}
void GraphicsLibrary::setVBO(double* buffer, int numOfvertex, int count)
{
	if (vboBuffer != NULL)
	{
		delete[] vboBuffer;
	}
	vboBuffer = new double[sizeof(double) * numOfvertex * count];
	memcpy(vboBuffer, buffer, sizeof(double) * numOfvertex * count);
	NumOfVertexVBO = numOfvertex;
	vboCount = count;
}


GraphicsLibrary::~GraphicsLibrary()
{
	if (Z_Buffer != NULL)
	{
		delete[] Z_Buffer;
	}
	if (textureBuffer != NULL)
	{
		delete[] textureBuffer;
	}
	if (vboBuffer != NULL)
	{
		delete[] vboBuffer;
	}
	if (NET != NULL)
	{
		delete[] NET;
	}
	if (interpolationVarying != NULL)
	{
		delete[] interpolationVarying;
	}
	if (interpolationVarying != NULL)
	{
		delete[] interpolationVarying;
	}
	closegraph();          // 关闭绘图窗口
}
