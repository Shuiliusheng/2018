/*
��Ҫ���ܣ���ȡimage��label�������γ�һ��������������ʽ
*/
#include"nn_minst.h"

//image��label�Ķ������
FILE *image=NULL;
FILE *label=NULL;
//�ļ�����
char image_name[300];
char label_name[300];

//��ʼ�� image �Ķ���
bool init_image()
{
	image=NULL;
	image=fopen(image_name,"rb");
	if(image==NULL)
	{
		cout <<image_name<<" is not exist!"<<endl;
		return false;
	}
	unsigned int temp=0;
	fread(&temp,1,sizeof(int),image);
	fread(&temp,1,sizeof(int),image);
	fread(&temp,1,sizeof(int),image);
	fread(&temp,1,sizeof(int),image);
	cout <<"open image file..."<<endl;

	return true;
}

//��ʼ�� label �Ķ���
bool init_label()
{
	label=NULL;
	label=fopen(label_name,"rb");
	if(label==NULL)
	{
		cout <<label_name<<" is not exist!"<<endl;
		return false;
	}
	unsigned int temp=0;
	fread(&temp,1,sizeof(int),label);
	fread(&temp,1,sizeof(int),label);
	cout <<"open label file..."<<endl;
	return true;
}

//��ȡ һ��ͼƬ �������
void read_vec(double *dst)
{
	unsigned char temp=0;
	if(feof(image))
	{
		fclose(image);
		init_image();
	}
	for(int i=0;i<LEN;i++)
	{
		fread(&temp,1,1,image);
		dst[i]=(unsigned int)temp*1.0;
		dst[i]=dst[i]/256;
	}
}
//��ȡ һ��ͼƬ ����Ϊ batchsize
void read_data(double **dst)
{
	for(int i=0;i<GROUP;i++)
	{
		read_vec(dst[i]);
	}
}

//��ȡ һ��label ���10ά����
//���� labelΪ1���������ĵڶ�����Ϊ1�������Ϊ-1
//ԭ����Ϊtanh�����Ϊ��-1��1���������Ҫ������ı�Ϊ���Ƶķ�Χ����߾���
void read_label_single(double *dst)
{
	if(feof(label))
	{
		fclose(label);
		init_label();
	}
	unsigned char temp;
	fread(&temp,1,1,label);
	for(int i=0;i<OUT;i++)
		dst[i]=-1;
	dst[temp]=1;
}
//��ȡ һ��label ����Ϊ batchsize
void read_label(double **dst)
{
	for(int i=0;i<GROUP;i++)
	{
		read_label_single(dst[i]);
	}
}
