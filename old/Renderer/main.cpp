#include <graphics.h>      // 引用图形库头文件
#include <conio.h>
#include <stdio.h>
#include <time.h>
#include "Graphics.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <chrono> 
#pragma warning(disable:4996)
//从obj文件读取顶点数据和纹理坐标，暂时不考虑法线
bool loadOBJ(const char* filename, GraphicsLibrary* gl)
{
	std::vector<double> vbo;
	int vboCount = 0;
	struct Postion//只想在函数内部使用这个Position，所以定义在函数内部
	{
		double x;
		double y;
		double z;
	};
	struct Coordinate//纹理坐标
	{
		double x;
		double y;
	};
	struct normal//法向量
	{
		double x;
		double y;
		double z;
	};
	std::vector<Postion> ps;//顶点集合
	std::vector<Coordinate> TextureCoordinate;//纹理坐标集合
	std::vector<normal> normals;//纹理坐标集合
	Postion p;
	Coordinate t;
	normal n;
	char line[1024];
	std::ifstream fin(filename, std::ios::in);
	if (fin.is_open())
	{
		while (fin.getline(line, sizeof(line)))
		{
			char s1[512];
			char ss[3][512];
			std::stringstream sin(line);
			sin >> s1;
			if (strcmp("v", s1) == 0)
			{
				sin >> p.x >> p.y >> p.z;
				ps.push_back(p);
			}
			else if (strcmp("vt", s1) == 0)
			{
				sin >> t.x >> t.y;
				TextureCoordinate.push_back(t);
			}
			else if (strcmp("vn", s1) == 0)
			{
				sin >> n.x >> n.y >> n.z;
				normals.push_back(n);
			}
			else if (strcmp("f", s1) == 0)
			{
				sin >> ss[0] >> ss[1] >> ss[2];
				int index[3];

				for (int i = 0; i < 3; i++)
				{
					if (sscanf(ss[i], "%d/%d/%d", index, index + 1, index + 2) != 3)//obj文件face有三个元素
					{
						fin.close();
						return false;
					}
					vbo.push_back(ps[index[0] - 1].x);
					vbo.push_back(ps[index[0] - 1].y);
					vbo.push_back(ps[index[0] - 1].z);//添加了当前三角形第i个顶点的坐标

					vbo.push_back(TextureCoordinate[index[1] - 1].x);
					vbo.push_back(TextureCoordinate[index[1] - 1].y);//添加了当前三角形第i个顶点的纹理坐标

					vbo.push_back(normals[index[2] - 1].x);
					vbo.push_back(normals[index[2] - 1].y);
					vbo.push_back(normals[index[2] - 1].z);//添加了当前三角形第i个顶点的法向量
					vboCount++;
				}
			}
		}
		fin.close();
		gl->setVBO(&vbo[0], 8, vboCount);//每个顶点有8个属性，分别是x,y,z(顶点坐标), Tx,Ty,Nx,Ny,Nz;T表示纹理，N表示法向量
		gl->setVaryingCount(5);//需要从顶点着色器传递5个参数到片元着色器(即Tx,Ty,Nx,Ny,Nz)
		return true;
	}
	else
	{
		return false;
	}
}



Matrix4 mvpMatrix;
GraphicsLibrary* gl;
Matrix4 invModMatrix;//mod矩阵的逆矩阵(inverseMatrix)
Vector4 invLight;//光线的逆向量
//vs fs的定义在Graph的头文件有说明
void vs(double const vbo[3], double* varying, Point4& Position)//顶点着色器
{

	double src[4];
	src[0] = vbo[0];
	src[1] = vbo[1];
	src[2] = vbo[2];
	src[3] = 1;
	Matrix::Mult(mvpMatrix.Value[0], src, 4, 1, 4, Position.value);//计算顶点坐标
	varying[0] = vbo[3];
	varying[1] = vbo[4];
	varying[2] = vbo[5];
	varying[3] = vbo[6];
	varying[4] = vbo[7];
}
void fs(double* varying, COLORREF& FragColor)//片元着色器
{
	Vector3 nor(varying[2], varying[3], varying[4]);
	double diffuse = Vector4::dot(nor, invLight);//根据光向量和法线夹角计算光照强度
	if (diffuse < 0.1)
	{
		diffuse = 0.1;
	}
	else if (diffuse > 1.0)
	{
		diffuse = 1.0;
	}
	COLORREF c = gl->texture2D(varying[0], varying[1]);//读取指定纹素,因为ABO被设置成每个顶点两个属性，一个是纹理x坐标，一个是纹理y坐标
	//COLORREF c = WHITE;
	FragColor = RGB(diffuse * GetRValue(c), diffuse * GetGValue(c), diffuse * GetBValue(c));
}
int main()
{
	char msg[256];//往调试器打印信息用的缓冲区
	int width = 640, height = 480;
	gl = new GraphicsLibrary(width, height);//创建一个画布
	int vw = width / 2;
	int vh = height / 2;
	//gl->setViewPort(vw, vh, vw, vh);//设置视口，和opengl概念一致
	gl->enable_CW = true;//启用顺时针逆时针三角形剔除
	gl->CW_CCW = false;//绘制逆时针三角形
	gl->VertexShader = vs;//设置顶点着色器程序
	gl->FragmentShader = fs;//设置片元着色器程序

	Vector3 eyePosition(0, 0, 15); //相机原点
	if (!loadOBJ("mod/cube.obj", gl))//从文件加载模型
	{
		sprintf(msg, "加载obj文件失败\n");
		OutputDebugString(msg);//往调试器输出错误信息
		delete gl;
		return -1;
	}
	if (!gl->loadBMP("mod/texture.bmp"))//加载BMP文件做纹理
	{
		OutputDebugString(gl->errmsg);//往调试器输出错误信息
		delete gl;
		return -1;
	}

	/*坐标系是上面为Y正方向,右面为X正方向,屏幕向外为Z正方向*/

	Vector3 up(0, 1, 0); //相机上方向
	Vector3 destination(0, 0, -10000); //相机看向的点
	Matrix4 vMatrix = Matrix4::LookAt(eyePosition, up, destination);
	double l, r, b, t, n, f;
	double w = 5; //近平面的参照宽高
	double proportion;//宽高比
	if (vw < vh)
	{ //宽度略小,以宽度为标准
		proportion = vh / vw;
		l = -w / 2;
		r = w / 2;
		b = -w * proportion / 2;
		t = w * proportion / 2;
		n = w;
		f = w * 10;
	}
	else
	{
		proportion = vw / vh;
		l = -w * proportion / 2;
		r = w * proportion / 2;
		b = -w / 2;
		t = w / 2;
		n = w;
		f = w * 10;
	}
	Matrix4 pMatrix = Matrix4::PerspectiveProjection(l, r, b, t, n, f);
	Matrix4 vpMatrix;
	Matrix::Mult(pMatrix.Value[0], vMatrix.Value[0], 4, 4, 4, vpMatrix.Value[0]);

	Vector3 axisA(0, 1, 0);//旋转轴A
	Vector3 axisB(1, 0, 0);//旋转轴B
	double angleA = 0;
	double angleB = 0;//绕AB轴旋转的角度
	double translateVec[3] = { 0,0,0 };//平移值

	double totalTime = 0;
	int totalFrame = 1;
	MOUSEMSG m;		// 定义鼠标消息
	int MX_s = 0;
	int MY_s = 0;//鼠标左键按下时的坐标
	int MX_e = 0;
	int MY_e = 0;//鼠标移动时坐标
	bool isClick = false;//鼠标左键是否按下
	bool isDraw = true;//默认绘制第一帧
	for (;; )
	{
		m = GetMouseMsg();//这个函数是阻塞的,可以在不绘制时用于减少CPU使用率
		for (;;)//把缓冲区中的所有鼠标消息取出来
		{
			switch (m.uMsg)
			{
			case WM_MOUSEWHEEL://鼠标滚动移动物体
				if (m.wheel > 0)
				{
					translateVec[2] += 0.1;
				}
				else
				{
					translateVec[2] -= 0.1;
				}
				isDraw = true;//需要绘制
				break;
			case WM_MOUSEMOVE:
				if (isClick)
				{
					MX_e = m.x;
					MY_e = m.y;
					angleA += ((double)MX_e - (double)MX_s) / width * 180;//采用累加方式记录鼠标总偏移量
					angleB += ((double)MY_e - (double)MY_s) / height * 180;
					MX_s = MX_e;
					MY_s = MY_e;
					isDraw = true;//需要绘制
				}
				break;
			case WM_LBUTTONDOWN:
				isClick = true;
				MX_s = m.x;
				MY_s = m.y;
				break;
			case WM_LBUTTONUP:
				isClick = false;
				break;
			}
			if (!MouseHit())
			{
				break;
			}
			m = GetMouseMsg();// 获取一条鼠标消息
		}
		if (!isDraw)
		{
			continue;//跳过下面的绘制代码
		}
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		Matrix4 mMatrixRotateA = Matrix4::Rotate(axisA, angleA);//计算旋转矩阵A
		Matrix4 mMatrixRotateB = Matrix4::Rotate(axisB, angleB);//计算旋转矩阵B
		Matrix4 mMatrixTranslate = Matrix4::Translate(translateVec);
		Matrix4 mMatrixTmp;
		Matrix4 mMatrix;//model matrix
		Matrix::Mult(mMatrixRotateB.Value[0], mMatrixRotateA.Value[0], 4, 4, 4, mMatrixTmp.Value[0]);//计算旋转结果
		Matrix::Mult(mMatrixTranslate.Value[0], mMatrixTmp.Value[0], 4, 4, 4, mMatrix.Value[0]);//得到平移结果,矩阵相乘有结合律 position*A*B*C*D = position*(A*B*C*D)，在这里面，D动作最先执行

		invModMatrix = Matrix4::QuickInverse(mMatrix);//求出模型矩阵的逆矩阵
		Vector4 lightvec(0.5, 0.5, 0.5, 0);//光线向量，向量的w分量为0
		Matrix::Mult(invModMatrix.Value[0], lightvec.value, 4, 1, 4, invLight.value);//计算光线的逆向量(即模型不动，光线动)
		invLight.Normalize();
		Matrix::Mult(vpMatrix.Value[0], mMatrix.Value[0], 4, 4, 4, mvpMatrix.Value[0]);//计算MVP矩阵
		gl->clearDepth(1);//清除深度缓冲区
		gl->clear();//清除屏幕
		gl->Draw();//绘制
		gl->flush();//等待同步
		gl->Swap();//绘制到屏幕
		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
		std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		double useTime = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;//花费时间
		if (16 - useTime * 1000 > 0)//如果绘制小于16毫秒，则休眠一段时间补齐16毫秒
		{
			Sleep(16 - (int)(useTime * 1000));
		}
		totalTime += useTime;
		totalFrame++;//帧计数器加一
		sprintf(msg, "第%d帧:本帧绘制耗时:%lf 毫秒,平均耗时:%lf毫秒\n", totalFrame, useTime * 1000, (double)totalTime / (double)totalFrame * 1000);
		OutputDebugString(msg);//往调试器输出两帧绘制时间间隔
		isDraw = false;//本轮绘制结束
	}
	delete gl;
	return 0;
}
