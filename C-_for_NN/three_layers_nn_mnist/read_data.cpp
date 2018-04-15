/*
主要功能：读取image和label，并且形成一定的输出和输入格式
*/
#include"nn_minst.h"

//image和label的读入参数
FILE *image=NULL;
FILE *label=NULL;
//文件名称
char image_name[300];
char label_name[300];

//初始化 image 的读入
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

//初始化 label 的读入
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

//读取 一张图片 变成向量
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
//读取 一组图片 组数为 batchsize
void read_data(double **dst)
{
	for(int i=0;i<GROUP;i++)
	{
		read_vec(dst[i]);
	}
}

//读取 一个label 变成10维向量
//例如 label为1，则向量的第二个数为1，其余均为-1
//原因：因为tanh的输出为（-1，1），因此需要将输出改变为类似的范围，提高精度
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
//读取 一组label 组数为 batchsize
void read_label(double **dst)
{
	for(int i=0;i<GROUP;i++)
	{
		read_label_single(dst[i]);
	}
}
