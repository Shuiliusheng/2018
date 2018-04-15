#include"nn_minst.h"

/*
损失函数关于A的导数
损失函数选择的是MSE，均方误差，但是没有取平均，而是1/2
取1/2是为了求导之后约去2
*/
double d_Jwb(double a,int i,int g)
{
	//return -1*(out[g][i]/a+(out[g][i]-1)/(1-a));
	return (a-out[g][i]);
}

/*
计算最后一层的一个delta
a:当前神经元的输出
z:当前神经元的输入
i:该层的第几个神经元
g:batch中的第几组
delta:目的输出
*/
void cal_delta(double a,double z,double &delta,int i,int g)
{
	delta=d_fuction(z)*d_Jwb(a,i,g);
}

/*
计算最后一层的delta
delta:结果
A:神经元的输出向量
Z:神经元的输入向量
col:最后一层神经元个数
g:当前计算的第几组
*/
void cal_lastlayer_delta(double *delta,double *A,double *Z,int col,int g)
{
	for(int i=0;i<col;i++)
	{
		cal_delta(A[i],Z[i],delta[i],i,g);
	}
}

//计算最后一层的多组delta
void cal_lastlayer_delta_group(double **delta,double **A,double **Z,int col)
{
	for(int i=0;i<GROUP;i++)
	{
		cal_lastlayer_delta(delta[i],A[i],Z[i],col,i);
	}
}

/*
计算某一层的某一组的一个神经元的delta
D:下一层神经元的当前组的Delta向量
W:下一层的网络参数W
z:当前组的要计算的这个神经元的输入
delta:当前组的要计算的这个神经元的delta
j:当前组的要计算的是第几个神经元
col:下一层神经元的个数
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
计算某一组的某一层的神经元的delta向量
D:下一层神经元的delta向量
Z:当前层的神经元的输入向量
W:下一层的参数
col1:当前层神经元个数
col2:下一层神经元个数
*/
void cal_layer_delta_once(double *delta,double **W,double *Z,double *D,int col1,int col2)
{
	for(int i=0;i<col1;i++)
	{
		cal_delta_vec(D,W,Z[i],delta[i],i,col2);
	}
}

/*
计算一次某一层神经元的所有组对应的delta矩阵
col1:当前层神经元个数
col2:下一层神经元个数
*/
void cal_layer_delta_group_once(double **delta,double **W,double **Z,double **D,int col1,int col2)
{
	for(int i=0;i<GROUP;i++)
	{
		cal_layer_delta_once(delta[i],W,Z[i],D[i],col1,col2);
	}
}

/*
更新一个w
D:当前层的delta矩阵
A:上一层的输出矩阵
i:该w所处当前层的W矩阵中的行数
j:该w所处当前层的W矩阵中的列数
*/
void update_w(double &v,double &w,double **D,double **A,int i,int j)
{
	double sum=0;
	for(int c=0;c<GROUP;c++)
	{
		sum+=(D[c][j]*A[c][i]);
	}
	sum/=GROUP;
	//更新w
	//w=(1-lambda)*w-eta*sum;
	double raw=lambda*w+eta*sum;
	v=R*v+raw;
	w=w-v;
}

/*
更新某一层的W
D:当前层的delta矩阵
A:上一层的输出矩阵
row:W的行数
col:W的列数
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