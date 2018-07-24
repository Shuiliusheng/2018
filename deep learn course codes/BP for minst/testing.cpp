#include"nn_minst.h"

/*
从结果的10维向量中，找到最大值，
然后这个最大值对应的位置即为预测的结果
*/
int find_max(double *src,int len)
{
	int l=0;
	double max=src[0];
	for(int i=0;i<len;i++)
	{
		if(max<src[i])
		{
			max=src[i];
			l=i;
		}
	}
	return l;
}

/*
测试阶段
一次读取所有的测试图片，batchsize即为所有的测试图片的个数
进行一次前向传播
比对结果和实际结果，给出正确率
*/
void testing(char str[])
{

	FILE *log=fopen("testing_log","a");

	if(!(init_image()&&init_label()))
		return ;
	read_data(in);
	read_label(out);
	fore_spread();

	double correct=0;
	for(int i=0;i<GROUP;i++)
	{
		int l1=find_max(A3[i],OUT);
		int l2=find_max(out[i],OUT);
		if(l1==l2)
		{
			correct=correct+1;
			printf("%d %d correct\n",l2,l1);
		}
		else
		{
			printf("%d %d wrong\n",l2,l1);
		}
	}
	printf("correct rate:%lf\n",correct/GROUP);
	fprintf(log,"%s\n",str);
	fprintf(log,"correct rate:%lf\n",correct/GROUP);
	fclose(log);
}