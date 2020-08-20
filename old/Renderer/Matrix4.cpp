#include "Matrix4.h"
#include<cstring>
#include <math.h>
#ifndef PI
#define PI 3.1415926
#endif // !PI


Matrix4::Matrix4(double value[4][4])
{
	memcpy(Value, value, sizeof(double) * 4 * 4);
}

Matrix4::Matrix4()
{

}

Matrix4 Matrix4::QuickInverse(Matrix4& src)
{
	Matrix4 dest;
	double a = src.Value[0][0];
	double b = src.Value[1][0];
	double c = src.Value[2][0];
	double d = src.Value[3][0];
	double e = src.Value[0][1];
	double f = src.Value[1][1];
	double g = src.Value[2][1];
	double h = src.Value[3][1];
	double i = src.Value[0][2];
	double j = src.Value[1][2];
	double k = src.Value[2][2];
	double l = src.Value[3][2];
	double m = src.Value[0][3];
	double n = src.Value[1][3];
	double o = src.Value[2][3];
	double p = src.Value[3][3];
	double q = a * f - b * e;
	double r = a * g - c * e;
	double s = a * h - d * e;
	double t = b * g - c * f;
	double u = b * h - d * f;
	double v = c * h - d * g;
	double w = i * n - j * m;
	double x = i * o - k * m;
	double y = i * p - l * m;
	double z = j * o - k * n;
	double A = j * p - l * n;
	double B = k * p - l * o;
	double ivd = 1 / (q * B - r * A + s * z + t * y - u * x + v * w);
	dest.Value[0][0] = (f * B - g * A + h * z) * ivd;
	dest.Value[1][0] = (-b * B + c * A - d * z) * ivd;
	dest.Value[2][0] = (n * v - o * u + p * t) * ivd;
	dest.Value[3][0] = (-j * v + k * u - l * t) * ivd;
	dest.Value[0][1] = (-e * B + g * y - h * x) * ivd;
	dest.Value[1][1] = (a * B - c * y + d * x) * ivd;
	dest.Value[2][1] = (-m * v + o * s - p * r) * ivd;
	dest.Value[3][1] = (i * v - k * s + l * r) * ivd;
	dest.Value[0][2] = (e * A - f * y + h * w) * ivd;
	dest.Value[1][2] = (-a * A + b * y - d * w) * ivd;
	dest.Value[2][2] = (m * u - n * s + p * q) * ivd;
	dest.Value[3][2] = (-i * u + j * s - l * q) * ivd;
	dest.Value[0][3] = (-e * z + f * x - g * w) * ivd;
	dest.Value[1][3] = (a * z - b * x + c * w) * ivd;
	dest.Value[2][3] = (-m * t + n * r - o * q) * ivd;
	dest.Value[3][3] = (i * t - j * r + k * q) * ivd;
	return dest;
}


//透视投影矩阵
/**
 * x'=n*x/z
 * y'=n*y/z
 * z'=z
 * 得到理论上的投影，还需要将其转换到CCV空间(正交投影)
 * x''=2*n/(r-l)x/z-(r+l)/(r-l)
 * y''=2*n/(t-b)y/z-(t+b)/(t-b)
 * z''=2/(f-n)z-(f+n)/(f-n)
 * 这时候把[x'',y'',z'']变为齐次坐标[x''z,y''z,z''z,z](显卡支持齐次坐标计算，自己用CPU计算太慢了)
 * x''z=2*n/(r-l)x-(r+l)/(r-l)*z
 * y''z=2*n/(t-b)y-(t+b)/(t-b)*z
 * z''z=2/(f-n)z*z-(f+n)/(f-n)*z(这是理论值，实际上矩阵不支持乘方，只能线性变换)
 * 线性变换有如下规则：
 *      1:z''属于[-1,1]区间(CCV投影限制区域，这样可以不渲染在屏幕之外的像素，以及超出范围的顶点)
 *      2:z''随着z增大而增大(为了消隐)
 *      3:z''不可能随着xy的变化而变化
 * 所以设:z''z=Az+B，z''=A+B/z(z属于区间[n,f])属于区间[-1,1],解得A=-(n+f)/(n-f) B=2fn/(n-f)
 * z''z=-(n+f)/(n-f)z+2fn/(n-f)
 */
 //在将世界坐标转换为相机坐标时，即：若原先 a=f(z),a为相机空间的信息,z为世界坐标z值，统一变成a=f(-z)
 //这就是下面两种透视矩阵第三列取反的原因
Matrix4 Matrix4::PerspectiveProjection(double l, double r, double b, double t, double n, double f)
{
	double tmp[4][4] = {
			{2 * n / (r - l), 0, (r + l) / (r - l), 0},
			{0, 2 * n / (t - b), (t + b) / (t - b), 0},
			{0, 0, (n + f) / (n - f), 2 * f * n / (n - f)},
			{0, 0, -1, 0},
	};
	return Matrix4(tmp);
}

Matrix4 Matrix4::LookAt(Vector3& eye, Vector3& up, Vector3& dest)
{
	up.Normalize();

	//相机的三个轴，z正方向为相机->视点
	Vector3 z(eye, dest);
	z.Normalize();
	Vector3 x(Vector3::CrossProduct(z, up)); //因为Vector.CrossProduct计算结果为右手，现在我们想要左手结果，把参数互换,下面同理
	x.Normalize();
	Vector3 y(Vector3::CrossProduct(x, z)); //得到x,y,z三个向量
	y.Normalize();

	/*
	let TS =
		[[1, 0, 0, -eye[0]],
		[0, 1, 0, -eye[1]],
		[0, 0, 1, -eye[2]],
		[0, 0, 0, 1]];//平移矩阵
	*/
	double T[4][4] = {
		{1, 0, 0, -eye.value[0] },
	{0, 1, 0, -eye.value[1]},
	{0, 0, 1, -eye.value[2]},
	{0, 0, 0, 1}
	}; //平移矩阵的逆矩阵
	/*
	let A = [//这个表示了相机的旋转,并且为一个正交矩阵，其逆矩阵等于转置矩阵
		//相机在自己的坐标系中旋转，第三列取反
		[x[0], y[0], -z[0], 0],
		[x[1], y[1], -z[1], 0],
		[x[2], y[2], -z[2], 0],
		[0, 0, 0, 1]
	];//旋转
	*/
	double R[4][4] = //A逆矩阵
	{
			{x.value[0], x.value[1], x.value[2], 0},
			{y.value[0], y.value[1], y.value[2], 0},
			{-z.value[0], -z.value[1], -z.value[2], 0},
			{0, 0, 0, 1}
	};
	Matrix4 a(R);
	Matrix4 b(T);
	double reuslt[4][4];
	Matrix::Mult(R[0], T[0], 4, 4, 4, reuslt[0]);//得到的矩阵为相机在世界的移动,m*v表示把世界的物体也按照相机的方式移动(把世界坐标转换为相机坐标
	return Matrix4(reuslt);
}

Matrix4 Matrix4::Rotate(Vector3& vec, double angle)
{
	double radian = angle * PI / 180; //转为弧度制
	double u = vec.value[0];
	double v = vec.value[1];
	double w = vec.value[2];
	double s = u * u + v * v + w * w;
	double sq = sqrt(s);
	double cosine = cos(radian);
	double sine = sin(radian);
	double result[4][4] = {
			{(u * u + (v * v + w * w) * cosine) / s, (u * v * (1 - cosine) - w * sq * sine) / s, (u * w * (1 - cosine) + v * sq * sine) / s, 0},
			{(u * v * (1 - cosine) + w * sq * sine) / s, (v * v + (u * u + w * w) * cosine) / s, (v * w * (1 - cosine) - u * sq * sine) / s, 0},
			{(u * w * (1 - cosine) - v * sq * sine) / s, (v * w * (1 - cosine) + u * sq * sine) / s, (w * w + (u * u + v * v) * cosine) / s, 0},
			{0, 0, 0, 1}
	};
	return Matrix4(result);
}

Matrix4 Matrix4::Translate(double vec[3])
{
	double u = vec[0];
	double v = vec[1];
	double w = vec[2];
	double result[4][4] = {
			{1, 0, 0, u},
			{0, 1, 0, v},
			{0, 0, 1, w},
			{0, 0, 0, 1}
	};
	return Matrix4(result);
}

Matrix4::~Matrix4()
{
}
