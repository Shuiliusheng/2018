#include"encode.h"

double d_fuction(double a)
{
	return exp(-1*a)*pow((1+exp(-1*a)),-2);
}

double d_Jwb(double a,int col,int g)
{
	//return -1*(in[g][col]/a+(in[g][col]-1)/(1-a));
	return (a-in[g][col]);
}

void cal_delta(double a,double z,double &delta,int col,int g)
{
	delta=d_fuction(z)*d_Jwb(a,col,g);
}

void cal_lastlayer_delta(double *delta,double *A,double *Z,int col,int g)
{
	for(int i=0;i<col;i++)
	{
		cal_delta(A[i],Z[i],delta[i],col,g);
	}
}

void cal_lastlayer_delta_group(double **delta,double **A,double **Z,int col)
{
	for(int i=0;i<GROUP;i++)
	{
		cal_lastlayer_delta(delta[i],A[i],Z[i],col,i);
	}
}

void cal_delta_vec(double *D,double **W,double z,double &delta,int j,int col)
{
	delta=0;
	for(int i=0;i<col;i++)
	{
		delta+=(D[i]*W[i][j]*d_fuction(z));
	}
}

void cal_layer_delta_once(double *delta,double **W,double *Z,double *D,int col1,int col2)
{
	for(int i=0;i<col1;i++)
	{
		cal_delta_vec(D,W,Z[i],delta[i],i,col2);
	}
}

void cal_layer_delta_group_once(double **delta,double **W,double **Z,double **D,int col1,int col2)
{
	for(int i=0;i<GROUP;i++)
	{
		cal_layer_delta_once(delta[i],W,Z[i],D[i],col1,col2);
	}
}

void update_w(double &w,double **D,double **A,int i,int j)
{
	double sum=0;
	for(int c=0;c<GROUP;c++)
	{
		sum+=(D[c][j]*A[c][i]);
	}
	sum/=GROUP;
	//¸üÐÂw
	w=(1-eta*lambda)*w-eta*sum;
}
void update_w_once(double **W,double **D,double **A,int row,int col)
{
	for(int j=0;j<row;j++)
	{
		for(int i=0;i<col;i++)
		{
			update_w(W[j][i],D,A,i,j);
		}
	}
}

void back_spread()
{
	//calculate the last layer delta
	cal_lastlayer_delta_group(delta2,A2,Z2,LEN);

	//calculate the last-1 layer delta
	cal_layer_delta_group_once(delta1,W2,Z1,delta2,HIDDEN,LEN);

	//updata W2
	update_w_once(W2,delta2,A1,LEN,HIDDEN);

	//update W1
	update_w_once(W1,delta1,in,HIDDEN,LEN);
}