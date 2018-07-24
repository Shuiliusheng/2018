#include"nn_minst.h"

/*
基本的损失函数，不包括正则化项
计算方法：MSE
in:计算的输出
out:实际输出
length:输出的长度
*/
void raw_lost(double *in,double *out,int length,double &lost)
{
	lost=0;
	for(int i=0;i<length;i++)
	{
		lost+=pow((in[i]-out[i]),2);
	}
	lost=lost/(2);
}

//计算W的平方和
void sum_w(double **w,int r,int c,double &sum)
{
	sum=0;
	for(int i=0;i<r;i++)
	{
		for(int j=0;j<c;j++)
			sum+=pow(w[i][j],2);
	}
}

/*
计算带有正则化项的lost fuction
in:训练输出矩阵
out:实际输出矩阵
w1-w3:每一层网络的参数
l0-l3:从输入层到输出层每一层的神经元个数
*/
double lost_fuction(double **in,double **out,double **w1,double **w2,double **w3,int l0,int l1,int l2,int l3)
{
	double *lost=(double *)malloc(GROUP*sizeof(double));
	for(int i=0;i<GROUP;i++)
	{
		raw_lost(in[i],out[i],l3,lost[i]);
	}

	double sum1=0,sum2=0,sum3=0;
	sum_w(w1,l1,l0,sum1);
	sum_w(w2,l2,l1,sum2);
	sum_w(w3,l3,l2,sum3);

	double sum=0;
	for(int i=0;i<GROUP;i++)
		sum+=lost[i];
	return sum/GROUP+(sum1+sum2+sum3)*lambda*0.5;
}

/*
计算没有正则化项的lost fuction
in:训练输出矩阵
out:实际输出矩阵
length:输出的长度
*/
double lost_fuction_raw(double **in,double **out,int length)
{
	double *lost=(double *)malloc(GROUP*sizeof(double));
	for(int i=0;i<GROUP;i++)
	{
		raw_lost(in[i],out[i],length,lost[i]);
	}
	double sum=0;
	for(int i=0;i<GROUP;i++)
		sum+=lost[i];
	return sum/GROUP;
}

int train_times=0;

//保存训练之后的网络参数
void save_w(double lost,double **W1,double **W2,double **W3,int l0,int l1,int l2,int l3)
{
	char str[100];
	sprintf(str,"Traintimes_%d_Group_%d_eta_%lf_lambda_%lf.param",train_times,GROUP,eta,lambda);
	FILE *p=fopen(str,"w");
	fprintf(p,"%lf\n",lost);
	for(int i=0;i<l1;i++)
		for(int j=0;j<l0;j++)
			fprintf(p,"%lf\n",W1[i][j]);

	for(int i=0;i<l2;i++)
		for(int j=0;j<l1;j++)
			fprintf(p,"%lf\n",W2[i][j]);

	for(int i=0;i<l3;i++)
		for(int j=0;j<l2;j++)
			fprintf(p,"%lf\n",W3[i][j]);
	fclose(p);
	cout <<"encode network parameters are saved in: "<<str<<endl;
}

//训练过程
void training()
{
	//申请训练过程中需要的参数
	train_init_memory();

	char str[100];
	//log文件，用于记录损失的变化
	sprintf(str,"Traintimes_%d_Group_%d_eta_%lf_lambda_%lf.log",train_times,GROUP,eta,lambda);

	FILE *log=fopen(str,"w");
	fprintf(log,"Times Raw_lost Total_lost\n");

	double lost=0;
	int times=0;
	//初始化imagefile和labelfile
	if(!(init_image()&&init_label()))
		return ;
	
	//训练traintimes
	while(times<train_times)
	{
		times++;
		//读取输入和输出
		read_data(in);
		read_label(out);
		//前向传播一次
		fore_spread();
		//计算损失
		double lost1=lost_fuction(A3,out,W1,W2,W3,LEN,HIDDEN1,HIDDEN2,OUT);
		lost=lost_fuction_raw(A3,out,OUT);
		fprintf(log,"times %d : %lf %lf\n",times,lost,lost1);
		printf("times %d : %lf %lf\n",times,lost,lost1);

		//反向传播一层
		back_spread();
	}
	//保存参数
	save_w(lost,W1,W2,W3,LEN,HIDDEN1,HIDDEN2,OUT);
	fclose(log);

	//清除申请的空间
	train_clear_memory();
}
