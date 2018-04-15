#include"feature.h"

void copy_data(Mat dst,Data input,int nr,int nc,int n)
{
	for(int i=0;i<input.H;i++)
	{
		for(int j=0;j<input.W;j++)
		{
			int r=nr+i;
			int c=nc+j;
			dst.data[r*dst.cols+c]=input.data[n][i][j];
		}
	}
}

void copy_data_color(Mat dst,Data input,int nr,int nc,int n)
{
	for(int i=0;i<input.H;i++)
	{
		for(int j=0;j<input.W;j++)
		{
			int r=nr+i;
			int c=nc+j;
			dst.data[(r*dst.cols+c)*3]=input.rgb[n][(i*input.W+j)*3];
			dst.data[(r*dst.cols+c)*3+1]=input.rgb[n][(i*input.W+j)*3+1];
			dst.data[(r*dst.cols+c)*3+2]=input.rgb[n][(i*input.W+j)*3+2];
		}
	}
}

void expand_image(Mat src,Mat dst,int expand)
{
	for(int r=0;r<dst.rows;r++)
	{
		for(int c=0;c<dst.cols;c++)
		{
			int x=c/expand;
			int y=r/expand;
			if(x<src.cols&&y<src.rows)
				dst.data[r*dst.step+c]=src.data[y*src.step+x];
		}
	}
}

void write_image(Data input,int numW,int numH,const char prefix[],int image,int expand)
{
	int step=numW*numH;
	int W=numW*input.W;
	int H=numH*input.H;
	Mat dst(H,W,CV_8UC1,Scalar(255));

	int start=image*step;
	int end=std::min(start+step,input.num);
	for(int i=start;i<end;i++)
	{
		int nr=((i-start)/numW)*input.H;
		int nc=((i-start)%numW)*input.W;
		copy_data(dst,input,nr,nc,i);
	}

	char filename[100];
	sprintf(filename,"%s_image%d_to_image%d.jpg",prefix,start,end);
	resize(dst,dst,Size(W*expand,H*expand),0,0,CV_INTER_NN);
	imwrite(filename,dst);
//	imshow(filename,dst);
}

void write_image_color(Data input,int numW,int numH,const char prefix[],int image,int expand)
{
	int step=numW*numH;
	int W=numW*input.W;
	int H=numH*input.H;
	Mat dst(H,W,CV_8UC3,Scalar(255,255,255));

	int start=image*step;
	int end=std::min(start+step,input.num);
	for(int i=start;i<end;i++)
	{
		int nr=((i-start)/numW)*input.H;
		int nc=((i-start)%numW)*input.W;
		copy_data_color(dst,input,nr,nc,i);
	}

	char filename[100];
	sprintf(filename,"%s_color_image%d_to_image%d.jpg",prefix,start,end);
	resize(dst,dst,Size(W*expand,H*expand),0,0,CV_INTER_NN);
	imwrite(filename,dst);
//	imshow(filename,dst);
}

void save_data(Data input,int maxW,int maxH,const char prefix[],int expand)
{
	int numW=0,numH=0;
	numW=maxW/input.W;
	numH=maxH/input.H;
	
	if(numW==0)
		numW=1;
	if(numH==0)
		numH=1;

	int step=numW*numH;
	int image_num=input.num/step;
	if(input.num%step!=0)
		image_num++;

	for(int i=0;i<image_num;i++)
	{
		write_image(input,numW,numH,prefix,i,expand);
		write_image_color(input,numW,numH,prefix,i,expand);
	}
}