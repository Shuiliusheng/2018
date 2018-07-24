#include"nn_minst.h"

/*
activation fuction
ѡ�� tanh��Ϊ�����
����ķ�ΧΪ(-1,1)
������(-2,2)֮��仯��Ϊ����
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

//���������ĳ˷�,������һ��Z[i]
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
��֮�����ǰ����
input:��һ�����Ԫ���
Z:��ǰ�����Ԫ����
A:��ǰ�����Ԫ���
w:��ǰ����������
row:��ǰ�����Ԫ����
col:ǰһ�����Ԫ����
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
����һ�����һ����ǰ����
�ֳ����ڵ�һ��image����ǰ����
��������������һ�£�ֻ�Ǳ���˶��

ʹ��ѭ������ʽ�ͺ����������ʽʵ�ֵĿ���
Ϊ�˷���֮��Ľ�Ϊ���̵߳�ʵ��
*/
void fore_spread_group_once(double **input,double **Z,double **A,double **w,int row, int col)
{
	for(int i=0;i<GROUP;i++)
	{
		fore_spread_once(input[i],Z[i],A[i],w,row,col);
	}
}


//һ��ǰ�򴫵�
void fore_spread()
{
	//the first layer calculate z and a
	fore_spread_group_once(in,Z1,A1,W1,HIDDEN1,LEN);

	//the second layer calculate z and a
	fore_spread_group_once(A1,Z2,A2,W2,HIDDEN2,HIDDEN1);

	//the second layer calculate z and a
	fore_spread_group_once(A2,Z3,A3,W3,OUT,HIDDEN2);
}
