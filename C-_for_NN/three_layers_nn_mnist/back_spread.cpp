#include"nn_minst.h"

/*
��ʧ��������A�ĵ���
��ʧ����ѡ�����MSE������������û��ȡƽ��������1/2
ȡ1/2��Ϊ����֮��Լȥ2
*/
double d_Jwb(double a,int i,int g)
{
	//return -1*(out[g][i]/a+(out[g][i]-1)/(1-a));
	return (a-out[g][i]);
}

/*
�������һ���һ��delta
a:��ǰ��Ԫ�����
z:��ǰ��Ԫ������
i:�ò�ĵڼ�����Ԫ
g:batch�еĵڼ���
delta:Ŀ�����
*/
void cal_delta(double a,double z,double &delta,int i,int g)
{
	delta=d_fuction(z)*d_Jwb(a,i,g);
}

/*
�������һ���delta
delta:���
A:��Ԫ���������
Z:��Ԫ����������
col:���һ����Ԫ����
g:��ǰ����ĵڼ���
*/
void cal_lastlayer_delta(double *delta,double *A,double *Z,int col,int g)
{
	for(int i=0;i<col;i++)
	{
		cal_delta(A[i],Z[i],delta[i],i,g);
	}
}

//�������һ��Ķ���delta
void cal_lastlayer_delta_group(double **delta,double **A,double **Z,int col)
{
	for(int i=0;i<GROUP;i++)
	{
		cal_lastlayer_delta(delta[i],A[i],Z[i],col,i);
	}
}

/*
����ĳһ���ĳһ���һ����Ԫ��delta
D:��һ����Ԫ�ĵ�ǰ���Delta����
W:��һ����������W
z:��ǰ���Ҫ����������Ԫ������
delta:��ǰ���Ҫ����������Ԫ��delta
j:��ǰ���Ҫ������ǵڼ�����Ԫ
col:��һ����Ԫ�ĸ���
*/
void cal_delta_vec(double *D,double **W,double z,double &delta,int j,int col)
{
	delta=0;
	for(int i=0;i<col;i++)
	{
		delta+=(D[i]*W[i][j]*d_fuction(z));
	}
}

/*
����ĳһ���ĳһ�����Ԫ��delta����
D:��һ����Ԫ��delta����
Z:��ǰ�����Ԫ����������
W:��һ��Ĳ���
col1:��ǰ����Ԫ����
col2:��һ����Ԫ����
*/
void cal_layer_delta_once(double *delta,double **W,double *Z,double *D,int col1,int col2)
{
	for(int i=0;i<col1;i++)
	{
		cal_delta_vec(D,W,Z[i],delta[i],i,col2);
	}
}

/*
����һ��ĳһ����Ԫ���������Ӧ��delta����
col1:��ǰ����Ԫ����
col2:��һ����Ԫ����
*/
void cal_layer_delta_group_once(double **delta,double **W,double **Z,double **D,int col1,int col2)
{
	for(int i=0;i<GROUP;i++)
	{
		cal_layer_delta_once(delta[i],W,Z[i],D[i],col1,col2);
	}
}

/*
����һ��w
D:��ǰ���delta����
A:��һ����������
i:��w������ǰ���W�����е�����
j:��w������ǰ���W�����е�����
*/
void update_w(double &v,double &w,double **D,double **A,int i,int j)
{
	double sum=0;
	for(int c=0;c<GROUP;c++)
	{
		sum+=(D[c][j]*A[c][i]);
	}
	sum/=GROUP;
	//����w
	//w=(1-lambda)*w-eta*sum;
	double raw=lambda*w+eta*sum;
	v=R*v+raw;
	w=w-v;
}

/*
����ĳһ���W
D:��ǰ���delta����
A:��һ����������
row:W������
col:W������
*/
void update_w_once(double **V,double **W,double **D,double **A,int row,int col)
{
	for(int j=0;j<row;j++)
	{
		for(int i=0;i<col;i++)
		{
			update_w(V[j][i],W[j][i],D,A,i,j);
		}
	}
}

void back_spread()
{
	
	//calculate the last layer delta
	cal_lastlayer_delta_group(delta3,A3,Z3,OUT);

	//calculate the last-1 layer delta
	cal_layer_delta_group_once(delta2,W3,Z2,delta3,HIDDEN2,OUT);

	//calculate the last-2 layer delta
	cal_layer_delta_group_once(delta1,W2,Z1,delta2,HIDDEN1,HIDDEN2);

	//updata W3
	update_w_once(V3,W3,delta3,A2,OUT,HIDDEN2);

	//updata W2
	update_w_once(V2,W2,delta2,A1,HIDDEN2,HIDDEN1);

	//update W1
	update_w_once(V1,W1,delta1,in,HIDDEN1,LEN);
}