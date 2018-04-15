#include"encode.h"

void fuction(double z,double &a)
{
	a=1/(1+exp(-1*z));
}

void cal_vec_mul(double *input,double *w,double &z,int len)
{
	z=0;
	for(int i=0;i<len;i++)
	{
		z+=(input[i]*w[i]);
	}
}

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

void fore_spread_group_once(double **input,double **Z,double **A,double **w,int row, int col)
{
	for(int i=0;i<GROUP;i++)
	{
		fore_spread_once(input[i],Z[i],A[i],w,row,col);
	}
}


void fore_spread()
{
	//the first layer calculate z and a
	fore_spread_group_once(in,Z1,A1,W1,HIDDEN,LEN);

	//the second layer calculate z and a
	fore_spread_group_once(A1,Z2,A2,W2,LEN,HIDDEN);
}
