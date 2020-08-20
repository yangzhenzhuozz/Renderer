#include <GraphicsDevice.h>
#include <GraphicsLibrary.h>
#include <Matrix.h>
#include "stdio.h"
#include <conio.h>
#include <string.h>

#include <iostream>
#include <ctime>
#include <thread>
#include <fstream>
#include <sstream>
std::clock_t start, end;

//读取无压缩翻转的24位bmp图片作为纹理
class Texture
{
private:
	int bmpHeight;
	int bmpWidth;
	byte* imgBuffer;
	byte* textureBuffer;
public:
	~Texture()
	{
		delete imgBuffer;
	}
	//读取bmp文件
	Texture(const char* filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::in);//打开文件
		if (!file.bad())
		{
			file.seekg(0, std::ios_base::end);
			size_t fileLen = file.tellg();//获取文件大小
			imgBuffer = new byte[fileLen];//创建文件缓冲区
			file.seekg(0, std::ios_base::beg);//移动到文件头部
			file.read((char*)imgBuffer, fileLen);//读取文件到内存
			file.close();//关闭文件

			//下面的代码就是解析bmp文件的代码了，我也是参考网上查阅到的资料编写的，可能有bug
			tagBITMAPFILEHEADER* fhead = (tagBITMAPFILEHEADER*)imgBuffer;
			tagBITMAPINFOHEADER* ihead = (tagBITMAPINFOHEADER*)(imgBuffer + sizeof(tagBITMAPFILEHEADER));
			if (ihead->biHeight < 0)
			{
				throw "纵轴反向";
			}
			if (ihead->biBitCount != 24)
			{
				throw "不是24位位图";
			}
			if (ihead->biCompression != 0)
			{
				throw "位图有压缩";
			}
			bmpHeight = ihead->biHeight;
			bmpWidth = ihead->biWidth;//保存位图宽高
			textureBuffer = imgBuffer + fhead->bfOffBits;
		}
		else
		{
			throw "文件打开失败";
		}
	}
	COLORREF texture2D(double u, double v)
	{
		u = u - floor(u);
		v = v - floor(v);
		int CountOfRowSize = (((bmpWidth * 24) + 31) >> 5) << 2;//每行像素所占用的字节，文档上面是这么写的
		int X = (int)(u * bmpWidth);
		int Y = (int)(v * bmpHeight);
		return RGB(textureBuffer[Y * CountOfRowSize + X * 3 + 2], textureBuffer[Y * CountOfRowSize + X * 3 + 1], textureBuffer[Y * CountOfRowSize + X * 3 + 0]);
	}
};
Texture* texture;
Matrix p_M = Matrix::Perspective(-1.6, 1.6, -1.2, 1.2, 2, 10);//透视投影矩阵
Matrix m_M(4, 4);//模型变换矩阵
Matrix mvp(4, 4);
double light_vec[] = { 0.57735026918962576450914878050196 ,0.57735026918962576450914878050196 ,-0.57735026918962576450914878050196 };//光线向量，已经规范化
void VS(double* vbo1, double* vbo2, Point4& gl_Vertex, std::vector<double>& varying)
{
	Point4 p_(vbo1[0], vbo1[1], vbo1[2], 1);
	Matrix p(4, 1);
	p[0][0] = vbo1[0];
	p[1][0] = vbo1[1];
	p[2][0] = vbo1[2];
	p[3][0] = 1;
	Matrix r = mvp * p;
	gl_Vertex = Point4(r[0][0], r[1][0], r[2][0], r[3][0]);

	//前面两个是纹理坐标，复制到varying中
	varying[0] = vbo2[0];
	varying[1] = vbo2[1];

	Matrix Normal(4, 1);//构造法向量矩阵，向量的ω分量为0
	Normal[0][0] = vbo2[2];
	Normal[1][0] = vbo2[3];
	Normal[2][0] = vbo2[4];
	Normal[3][0] = 0;
	Matrix N = m_M * Normal;//将法向量进行模型变换
	varying[2] = N[0][0];//将模型变换后的法向量复制到varying中
	varying[3] = N[1][0];
	varying[4] = N[2][0];
}
void FS(std::vector<double>& varying, COLORREF& gl_Color)
{
	double ambient = 0.2;//环境光强
	double diffuse = varying[2] * light_vec[0] + varying[3] * light_vec[1] + varying[4] * light_vec[2];//将法向量和规范化后的光向量相乘
	if (diffuse < 0)//相反的时候可以计算出负数，我们让光强最小为0
	{
		diffuse = 0;
	}
	diffuse += ambient;//加上环境光
	if (diffuse > 1)//最大值为1
	{
		diffuse = 1;
	}
	COLORREF c = texture->texture2D(varying[0], varying[1]);//从纹理中取得纹素
	gl_Color = RGB(diffuse * GetRValue(c), diffuse * GetGValue(c), diffuse * GetBValue(c));
};

//读取obj文件，返回顶点数量
int loadOBJ(const char* filename, double*& vbo1, double*& vbo2)
{
	std::vector<double> vbo_1;//顶点xyz值
	std::vector<double> vbo_2;//纹理uv，法向量xyz
	int vboCount = 0;//顶点数量
	struct Position//顶点坐标
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
	std::vector<Position> Positions;//顶点集合
	std::vector<Coordinate> TextureCoordinate;//纹理坐标集合
	std::vector<normal> Normals;//法线集合

	Position p;
	Coordinate t;
	normal n;

	std::ifstream file(filename, std::ios::in);//打开文件
	char line[1024];//obj文件的一行不能超过1204字节
	if (!file.bad())
	{
		while (file.getline(line, sizeof(line)))
		{
			char line_head[512];//行头部
			char face_string[3][512];//面信息
			std::stringstream sin(line);
			sin >> line_head;
			if (strcmp("v", line_head) == 0)
			{
				sin >> p.x >> p.y >> p.z;
				Positions.push_back(p);
			}
			else if (strcmp("vt", line_head) == 0)
			{
				sin >> t.x >> t.y;
				TextureCoordinate.push_back(t);
			}
			else if (strcmp("vn", line_head) == 0)
			{
				sin >> n.x >> n.y >> n.z;
				Normals.push_back(n);
			}
			else if (strcmp("f", line_head) == 0)
			{
				sin >> face_string[0] >> face_string[1] >> face_string[2];
				int index[3];
				for (int i = 0; i < 3; i++)
				{
					sscanf(face_string[i], "%d/%d/%d", index, index + 1, index + 2);//obj文件face有三个元素

					//添加当前三角形第i个顶点的坐标
					vbo_1.push_back(Positions[(size_t)index[0] - 1].x);
					vbo_1.push_back(Positions[(size_t)index[0] - 1].y);
					vbo_1.push_back(-Positions[(size_t)index[0] - 1].z);//因为3ds _max导出的坐标是右手坐标系，而我们使用的是左手坐标系，z值是相反的

					//添加当前三角形第i个顶点的纹理坐标
					vbo_2.push_back(TextureCoordinate[(size_t)index[1] - 1].x);
					vbo_2.push_back(TextureCoordinate[(size_t)index[1] - 1].y);

					//添加当前三角形第i个顶点的法向量
					vbo_2.push_back(Normals[(size_t)index[2] - 1].x);
					vbo_2.push_back(Normals[(size_t)index[2] - 1].y);
					vbo_2.push_back(-Normals[(size_t)index[2] - 1].z);//因为3ds _max导出的坐标是右手坐标系，而我们使用的是左手坐标系，z值是相反的
					vboCount++;
				}
			}
		}
		vbo1 = new double[vbo_1.size()];
		vbo2 = new double[vbo_2.size()];
		std::copy(vbo_1.begin(), vbo_1.end(), vbo1);//复制数据到vbo
		std::copy(vbo_2.begin(), vbo_2.end(), vbo2);
		file.close();
	}
	else
	{
		throw "文件打开失败";
	}
	return vboCount;
}
int main()
{
	double* vbo1, * vbo2;
	int vboCount = loadOBJ("./model/a.obj", vbo1, vbo2);
	texture = new Texture(("./model/a.bmp"));
	GraphicsDevice gd(640, 480);
	GraphicsLibrary gl(gd);
	gl.setVBO(vbo1, 3, vbo2, 5, vboCount);
	gl.setVaryingSize(5);
	gl.VertexShader = VS;
	gl.FragmentShader = FS;
	gl.FrontFace = _FrontFace_CCW;
	int i = 45;
	for (;;)
	{
		if (kbhit())//如果按下键盘，则判断是不是q
		{
			if (getch() == 'q')
			{
				break;
			}
		}
		start = clock();
		auto move_1 = Matrix::Translate(0, 0, -4);
		auto rotate_1 = Matrix::RotateX(i);
		auto rotate_2 = Matrix::RotateZ(i);
		auto move_2 = Matrix::Translate(0, 0, 4);
		m_M = move_2 * rotate_2 * rotate_1 * move_1;
		mvp = p_M * m_M;
		gl.clear_depth(1.0);//将深度缓冲区中的所有值设置为1.0，因为我们在cvv裁剪中限定了far平面的深度值为1.0，再远的像素不再绘制
		gl.clear_color(RGB(0, 0, 0));//将所有像素值设置为黑色
		gl.draw();//渲染画面
		gl.flush();
		end = clock();
		double totalTime = ((double)((double)end - (double)start) / CLOCKS_PER_SEC) * 1000;
		std::cout << "Total time:" << totalTime << "ms" << std::endl;	//ms为单位
		i++;
		if (totalTime < 16)//如果小于16ms，则等待一段时间后再次渲染
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(16 - (int)totalTime));//休眠50ms
		}
	}
	delete texture;
	delete vbo1;
	delete vbo2;
	return 0;
}