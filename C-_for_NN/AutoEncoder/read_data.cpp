#include"encode.h"

FILE *file=NULL;
FILE *label=NULL;
char image_name[300];
char label_name[300];

bool init_image()
{
	file=NULL;
	file=fopen(image_name,"rb");
	if(file==NULL)
	{
		cout <<image_name<<" is not exist!"<<endl;
		return false;
	}
	unsigned int temp=0;
	fread(&temp,1,sizeof(int),file);
	fread(&temp,1,sizeof(int),file);
	fread(&temp,1,sizeof(int),file);
	fread(&temp,1,sizeof(int),file);
	cout <<"input the train data again!"<<endl;

	return true;
	//cin >>temp;
}


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
	cout <<"input the label data again!"<<endl;
	return true;
}

void read_vec(double *dst)
{
	unsigned char temp=0;
	if(feof(file))
	{
		fclose(file);
		init_image();
	}
	for(int i=0;i<LEN;i++)
	{
		fread(&temp,1,1,file);
		dst[i]=(unsigned int)temp*1.0;
		dst[i]=dst[i]/256;
	}
}

void read_data(double **dst)
{
	for(int i=0;i<GROUP;i++)
	{
		read_vec(dst[i]);
	}
}

void read_label_single(double *dst)
{
	if(feof(label))
	{
		fclose(label);
		init_label();
	}
	unsigned char temp;
	fread(&temp,1,1,label);
	for(int i=0;i<HIDDEN;i++)
		dst[i]=0;
	dst[temp]=1;
}

void read_label(double **dst)
{
	for(int i=0;i<GROUP;i++)
	{
		read_label_single(dst[i]);
	}
}
