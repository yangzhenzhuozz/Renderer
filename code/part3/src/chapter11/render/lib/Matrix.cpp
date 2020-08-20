#include "include/Matrix.h"

Matrix::Matrix(int r, int c) :rows(r), columns(c)
{
	buffer = new double[(size_t)rows * (size_t)columns];
	std::fill(buffer, buffer + (size_t)rows * (size_t)columns, 0.0);
}

Matrix::Matrix(const Matrix& src) :rows(src.rows), columns(src.columns)
{
	buffer = new double[(size_t)rows * (size_t)columns];
	std::copy(src.buffer, src.buffer + ((size_t)rows * (size_t)columns), buffer);
}

Matrix::~Matrix()
{
	delete buffer;
}

Matrix Matrix::operator*(const Matrix& b) const
{
	if (this->columns != b.rows)
	{
		throw "矩阵行数和列数无法满足相乘条件";
	}
	Matrix result(this->rows, b.columns);
	for (int r = 0; r < this->rows; r++)
	{
		for (int c = 0; c < b.columns; c++)
		{
			result[r][c] = 0;
			for (int i = 0; i < this->columns; i++)
			{
				result[r][c] = result[r][c] + (*this)[r][i] * b[i][c];
			}
		}
	}
	return result;
}

double* Matrix::operator[](int index) const
{
	return &buffer[index * columns];
}

Matrix& Matrix::operator=(const Matrix& b)
{
	// TODO: 在此处插入 return 语句
	if (this->rows != b.rows || this->columns != b.columns)
	{
		throw "两矩阵行列数不一致，无法赋值";
	}
	std::copy(b.buffer, b.buffer + (size_t)b.rows * (size_t)b.columns, buffer);
	return *this;
}

Matrix Matrix::Perspective(double l, double r, double b, double t, double n, double f)
{
	Matrix result(4, 4);
	result[0][0] = (2 * n) / (r - l);
	result[0][2] = -(r + l) / (r - l);
	result[1][1] = (2 * n) / (t - b);
	result[1][2] = -(t + b) / (t - b);
	result[2][2] = (f + n) / (f - n);
	result[2][3] = (2 * n * f) / (n - f);
	result[3][2] = 1;
	return result;
}

Matrix Matrix::RotateX(double angle)
{
	const double PI = 3.141592653589793;
	double radian = angle * PI / 180;
	Matrix result(4, 4);
	result[0][0] = 1;
	result[1][1] = std::cos(radian);
	result[1][2] = std::sin(radian);
	result[2][1] = -std::sin(radian);
	result[2][2] = std::cos(radian);
	result[3][3] = 1;
	return result;
}

Matrix Matrix::RotateY(double angle)
{
	const double PI = 3.141592653589793;
	double radian = angle * PI / 180;
	Matrix result(4, 4);
	result[0][0] = std::cos(radian);
	result[0][2] = -std::sin(radian);
	result[1][1] = 1;
	result[2][0] = std::sin(radian);
	result[2][2] = std::cos(radian);
	result[3][3] = 1;
	return result;
}

Matrix Matrix::RotateZ(double angle)
{
	const double PI = 3.141592653589793;
	double radian = angle * PI / 180;
	Matrix result(4, 4);
	result[0][0] = std::cos(radian);
	result[0][1] = -std::sin(radian);
	result[1][0] = std::sin(radian);
	result[1][1] = std::cos(radian);
	result[2][2] = 1;
	result[3][3] = 1;
	return result;
}

Matrix Matrix::Translate(double x, double y, double z)
{
	Matrix result(4, 4);
	result[0][0] = 1;
	result[1][1] = 1;
	result[2][2] = 1;
	result[3][3] = 1;
	result[0][3] = x;
	result[1][3] = y;
	result[2][3] = z;
	return result;
}

Matrix Matrix::Scale(double x, double y, double z)
{
	Matrix result(4, 4);
	result[0][0] = x;
	result[1][1] = y;
	result[2][2] = z;
	result[3][3] = 1;
	return result;
}
