#include"nn_minst.h"

/*
activation fuction
选择 tanh作为激活函数
结果的范围为(-1,1)
函数在(-2,2)之间变化较为剧烈
*/
void fuction(double z,double &a)
{
//	a=z;
	a=(exp(z)-exp(-1*z))/(exp(z)+exp(-1*z));
}

double d_fuction(double a)
{
	double t=0;
	fuction(a,t);
	return 1-t*t;
}

//计算向量的乘法,即计算一个Z[i]
void cal_vec_mul(double *input,double *w,double &z,int len)
{
	z=0;
	int i=0;
	for(i=0;i<len;i++)
	{
		z+=(input[i]*w[i]);
	}
}

/*
层之间的向前传递
input:上一层的神经元输出
Z:当前层的神经元输入
A:当前层的神经元输出
w:当前层的网络参数
row:当前层的神经元个数
col:前一层的神经元个数
*/
void fore_spread_once(double *input,double *Z,double *A,double **w,int row, int col)
{
	for(int i=0;i<row;i++)
	{
		cal_vec_mul(input,w[i],Z[i],col);
	}

	for(int i=0;i<row;i++)
	{
		fuction(Z[i],A[i]);
	}
}

/*
计算一个组的一次向前传递
分成组内的一个image的向前传递
参数基本与上面一致，只是变成了多个

使用循环的形式和函数分离的形式实现的考虑
为了方面之后改进为多线程的实现
*/
void fore_spread_group_once(double **input,double **Z,double **A,double **w,int row, int col)
{
	for(int i=0;i<GROUP;i++)
	{
		fore_spread_once(input[i],Z[i],A[i],w,row,col);
	}
}


//一层前向传递
void fore_spread()
{
	//the first layer calculate z and a
	fore_spread_group_once(in,Z1,A1,W1,HIDDEN1,LEN);

	//the second layer calculate z and a
	fore_spread_group_once(A1,Z2,A2,W2,HIDDEN2,HIDDEN1);

	//the second layer calculate z and a
	fore_spread_group_once(A2,Z3,A3,W3,OUT,HIDDEN2);
}
