#ifndef _Matrix
#define _Matrix
class Matrix
{
public:
	Matrix();
	static void Mult(double* a, double* b, int m, int n, int p, double* result);//矩阵相乘,m a的行数,n b的列数 即a:m*p b:p*n
	static void Transpose(double* a, int m, int n, double* result);//转置矩阵
	~Matrix();
};
#endif // !_Matrix

