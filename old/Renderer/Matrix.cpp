#include "Matrix.h"



Matrix::Matrix()
{
}


Matrix::~Matrix()
{
}

void Matrix::Mult(double* a, double* b, int m, int n, int p, double* result)
{
	for (int vm = 0; vm < m; vm++)
	{
		for (int vn = 0; vn < n; vn++)
		{
			result[vm * n + vn] = 0;//计算vm行vn列数据
			for (int vp = 0; vp < p; vp++)
			{
				result[vm * n + vn] += a[vm * p + vp] * b[vp * n + vn];
			}

		}
	}
}

void Matrix::Transpose(double* a, int m, int n, double* result)
{
	for (int vm = 0; vm < m; vm++)
	{
		for (int vn = 0; vn < n; vn++)
		{
			result[vm * n + vn] = a[vn * n + vm];
		}
	}
}
