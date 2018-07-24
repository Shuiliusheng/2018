#include"nn_minst.h"

/*
��������ʧ������������������
���㷽����MSE
in:��������
out:ʵ�����
length:����ĳ���
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

//����W��ƽ����
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
��������������lost fuction
in:ѵ���������
out:ʵ���������
w1-w3:ÿһ������Ĳ���
l0-l3:������㵽�����ÿһ�����Ԫ����
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
����û���������lost fuction
in:ѵ���������
out:ʵ���������
length:����ĳ���
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

//����ѵ��֮����������
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

//ѵ������
void training()
{
	//����ѵ����������Ҫ�Ĳ���
	train_init_memory();

	char str[100];
	//log�ļ������ڼ�¼��ʧ�ı仯
	sprintf(str,"Traintimes_%d_Group_%d_eta_%lf_lambda_%lf.log",train_times,GROUP,eta,lambda);

	FILE *log=fopen(str,"w");
	fprintf(log,"Times Raw_lost Total_lost\n");

	double lost=0;
	int times=0;
	//��ʼ��imagefile��labelfile
	if(!(init_image()&&init_label()))
		return ;
	
	//ѵ��traintimes
	while(times<train_times)
	{
		times++;
		//��ȡ��������
		read_data(in);
		read_label(out);
		//ǰ�򴫲�һ��
		fore_spread();
		//������ʧ
		double lost1=lost_fuction(A3,out,W1,W2,W3,LEN,HIDDEN1,HIDDEN2,OUT);
		lost=lost_fuction_raw(A3,out,OUT);
		fprintf(log,"times %d : %lf %lf\n",times,lost,lost1);
		printf("times %d : %lf %lf\n",times,lost,lost1);

		//���򴫲�һ��
		back_spread();
	}
	//�������
	save_w(lost,W1,W2,W3,LEN,HIDDEN1,HIDDEN2,OUT);
	fclose(log);

	//�������Ŀռ�
	train_clear_memory();
}
