#include"feature.h"

bool init_output(Data input,Data &output,int step)
{
	cout <<"Pooling:";

	if(input.W/step+1<3||input.H/step+1<3)
	{
		cout <<"The Output Image too small:"<<input.W/step+1<<" "<<input.H/step+1<<endl;
		return false;
	}

	output.num=input.num;
	output.H=input.H/step+1;
	output.W=input.W/step+1;

	printf("Input Image:%d*%d*%d  |  Output Image:%d*%d*%d\n",input.num,input.W,input.H,output.num,output.W,output.H);
	malloc_data(output);
	return true;
}

void copy_array(byte **input,int r,int c,int W,int H,int step,byte *m)
{
	for(int i=0;i<step;i++)
	{
		for(int j=0;j<step;j++)
		{
			int t1=r+i;
			int t2=c+j;
			if(t1>=H)t1=H-1;
			if(t2>=W)t2=W-1;
			m[i*step+j]=input[t1][t2];
		}
	}
}

byte find_max(byte *m,int step,int &w,int &h)
{
	int l=step*step;
	int max=0;
	int n=0;
	for(int i=0;i<l;i++)
		if(m[i]>max)
			max=m[i],n=i;
	h=n/step;
	w=n%step;
	return max;
}

void max_pooling_once(Data input,Data &output,int step,int n)
{
	byte *m=(byte *)malloc(sizeof(byte)*step*step);
	int w=0,h=0;
	for(int r=0;r<output.H;r++)
	{
		for(int c=0;c<output.W;c++)
		{
			copy_array(input.data[n],r*step,c*step,input.W,input.H,step,m);
			output.data[n][r][c]=find_max(m,step,w,h);
			int tr=r*step+h;
			int tc=c*step+w;
			if(tr>=input.H)tr=input.H-1;
			if(tc>=input.W)tc=input.W-1;
			output.rgb[n][(r*output.W+c)*3]=input.rgb[n][(tr*input.W+tc)*3];
			output.rgb[n][(r*output.W+c)*3+1]=input.rgb[n][(tr*input.W+tc)*3+1];
			output.rgb[n][(r*output.W+c)*3+2]=input.rgb[n][(tr*input.W+tc)*3+2];
		}
	}
}

bool max_pooling(Data input,Data &output,int step)
{
	if(!init_output(input,output,step))
		return false;
	for(int i=0;i<input.num;i++)
	{
		max_pooling_once(input,output,step,i);
	}
	return true;
}