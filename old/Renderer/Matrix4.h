#ifndef _Matrix4
#define _Matrix4
#include "Matrix.h"
#include "Vector4.h"
#include "Vector3.h"
class Matrix4
{
public:
	double Value[4][4] = {
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		{0,0,0,1}
	};//矩阵值
	Matrix4(double value[4][4]);
	Matrix4();
	static Matrix4 QuickInverse(Matrix4& src);//4阶方阵快速逆矩阵算法
	static Matrix4 PerspectiveProjection(double l, double  r, double b, double t, double n, double f);//给出透视投影矩阵 Perspective Matrix
	static Matrix4 LookAt(Vector3& eye, Vector3& up, Vector3& dest);//View Matrix 参数分别为相机位置，相机上方向朝向，目标视点
	static Matrix4 Rotate(Vector3& vec, double angle);//旋转轴和旋转角度(角度制)
	static Matrix4 Translate(double vec[3]);//移动
	~Matrix4();
};
#endif // !_Matrix4

