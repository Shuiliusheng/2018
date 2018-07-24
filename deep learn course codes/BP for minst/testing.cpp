#include"nn_minst.h"

/*
�ӽ����10ά�����У��ҵ����ֵ��
Ȼ��������ֵ��Ӧ��λ�ü�ΪԤ��Ľ��
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
���Խ׶�
һ�ζ�ȡ���еĲ���ͼƬ��batchsize��Ϊ���еĲ���ͼƬ�ĸ���
����һ��ǰ�򴫲�
�ȶԽ����ʵ�ʽ����������ȷ��
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