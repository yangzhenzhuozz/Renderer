#ifndef _MATRIX
#define _MATRIX
#include <algorithm>
#include <cmath>
class Matrix
{
public:
	int rows, columns;//行数，列数
	double* buffer;//数据缓冲区
	Matrix(int r, int c);
	Matrix(const Matrix& src);
	~Matrix();
	Matrix operator *(const Matrix& b) const;
	double* operator [](int index) const;//返回一个一维数组
	Matrix& operator=(const Matrix& b);
	static Matrix Perspective(double l, double r, double b, double t, double n, double f);//获取透视投影矩阵
	static Matrix RotateX(double angle);//获取围绕X轴旋转的矩阵
	static Matrix RotateY(double angle);//获取围绕Y轴旋转的矩阵
	static Matrix RotateZ(double angle);//获取围绕Z轴旋转的矩阵
	static Matrix Translate(double x, double y, double z);//平移
	static Matrix Scale(double x, double y, double z);//缩放
private:

};
#endif