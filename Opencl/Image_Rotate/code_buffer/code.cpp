#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include <unistd.h>
#include "opencl_code.h"
using namespace cv;

unsigned char *src_data;
unsigned char *cpu_data;
unsigned char *gpu_data;
Mat src;
Mat cpu_dst;
Mat gpu_dst;
int W,H;
int dW=500,dH=500;
void test(double degree,char dstname[]);

bool read_image(char filename[])
{
    src=imread(filename);
    if(!src.data)
    {
        cout <<filename<<" can't be read!"<<endl;
        return false;
    }
    src_data=src.data;
    W=src.cols;
    H=src.rows;
    cout <<"    SOURCE IMAGE PICTURE ATTRIBUTE:"<<endl;
	cout <<"         type :"<<src.type()<<" (CV_8UC3)"<<endl;
	cout <<"         Width:"<<W<<endl;
	cout <<"         Height:"<<H<<endl;
	cout <<"    DST IMAGE PICTURE ATTRIBUTE:"<<endl;
	cout <<"         type :"<<src.type()<<" (CV_8UC3)"<<endl;
	cout <<"         Width:"<<dW<<endl;
	cout <<"         Height:"<<dH<<endl;
	cout <<endl;
    return true;
}

bool init(char filename[])
{
    if(!read_image(filename))
        return false;
    cpu_dst=Mat(dW,dH,CV_8UC3,Scalar(255,255,255));
    gpu_dst=Mat(dW,dH,CV_8UC3,Scalar(255,255,255));
    cpu_data=cpu_dst.data;
    gpu_data=gpu_dst.data;
    return true;
}

double cpu_rotate(double rad,unsigned char *src,unsigned char *dst)
{
	struct timeval tv,tv1;
	gettimeofday(&tv,NULL);
	
    rad=rad/180*3.14;
    double cosrad=cos(-1*rad);
    double sinrad=sin(-1*rad);
    for(int y=0;y<dH;y++)
    {
        for(int x=0;x<dW;x++)
        {
            int rx=(x-dW/2)*cosrad+(y-dH/2)*sinrad;
            int ry=(y-dH/2)*cosrad-(x-dW/2)*sinrad;
            rx=rx+W/2;
            ry=ry+H/2;
            if(rx>=0&&rx<W&&ry>=0&&ry<H)
            {
                int t1=y*(dW*3)+x*3;
                int t2=ry*(W*3)+rx*3;
                for(int i=0;i<3;i++)
                {
                    dst[t1+i]=src[t2+i];
                }
            }
        }
    }
	gettimeofday(&tv1,NULL);
	long cost_time =(tv1.tv_usec)-(tv.tv_usec);
	printf("CPU Cost time: %lf ms\n", cost_time/1000.0);
	return cost_time/1000.0;
}

void cpu_test(double degree,char dstname[],int times)
{
	double t=0;
	for(int i=0;i<times;i++)
		t+=cpu_rotate(degree,src_data,cpu_data);
	printf("Average CPU Cost time: %lf ms\n\n", t/times);
	
	char name[200]="";
	strcat(name,"cpu_ratate_");
	strcat(name,dstname);
	imwrite(name,cpu_dst);
}

void gpu_test(double degree,char kernel_name[],char dstname[],int times)
{
	double t=0;
	for(int i=0;i<times;i++)
		t+=gpu_rotate(kernel_name,src_data,gpu_data,degree,W,H,dW,dH);
	printf("Average GPU Cost time: %lf ms\n\n", t/times);
	
	char name[200]="";
	strcat(name,"gpu_ratate_");
	strcat(name,dstname);
	imwrite(name,gpu_dst);
}

// g++ code.cpp opencl_code.h -o run `pkg-config --cflags --libs opencv` -lOpenCL

int main(int argc,char **argv)
{
	if(strcmp(argv[1],"-h")==0)
	{
		cout <<"arguments using"<<endl;
		cout <<"./run image_name kernel_name run_times rotate_degree (dW dH local_size)"<<endl;
		cout <<endl;
		return 0;
	}
	
	if(argc==7)
	{
		sscanf(argv[5],"%d",&dW);
		sscanf(argv[6],"%d",&dH);
	}
	else if(argc==8)
	{
		sscanf(argv[5],"%d",&dW);
		sscanf(argv[6],"%d",&dH);
		sscanf(argv[7],"%d",&local_size);
	}
	
	if(!init(argv[1]))
		return 1;
	
	int times=0;
	sscanf(argv[3],"%d",&times);
	double rad=0;
	sscanf(argv[4],"%lf",&rad);
	cpu_test(rad,argv[1],times);
	gpu_test(rad,argv[2],argv[1],times);
//	test(rad,argv[1]);
	return 0;
}


void test_rotate(double rad,unsigned char *src,unsigned char *dst)
{	
    rad=rad/180*3.14;
    double cosrad=cos(rad);
    double sinrad=sin(rad);
    for(int y=0;y<dH;y++)
    {
        for(int x=0;x<dW;x++)
        {
            int rx=x/2;
            int ry=y/2;
            if(rx>=0&&rx<W&&ry>=0&&ry<H)
            {
                int t1=y*(dW*3)+x*3;
                int t2=ry*(W*3)+rx*3;
                for(int i=0;i<3;i++)
                {
                    dst[t1+i]=src[t2+i];
                }
            }
        }
    }
}
	
void test(double degree,char dstname[])
{
	test_rotate(degree,src_data,cpu_data);
	char name[200]="";
	strcat(name,"test_ratate_");
	strcat(name,dstname);
	imwrite(name,cpu_dst);
}