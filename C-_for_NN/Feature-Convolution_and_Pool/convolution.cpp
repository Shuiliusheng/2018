#include"feature.h"
int flag=0;

void malloc_data(Data &output)
{
	output.data=(byte ***)malloc(sizeof(byte **)*output.num);
	for(int i=0;i<output.num;i++)
	{
		output.data[i]=(byte **)malloc(sizeof(byte *)*output.H);
		for(int j=0;j<output.H;j++)
			output.data[i][j]=(byte*)malloc(sizeof(byte)*output.W);
	}

	output.rgb=(byte **)malloc(sizeof(byte *)*output.num);
	for(int i=0;i<output.num;i++)
	{
		output.rgb[i]=(byte *)malloc(sizeof(byte)*output.H*output.W*3);
	}
}

bool init_output(Data &output,Data input,int out_num,int use_filter,bool border)
{
	cout <<"Conv: ";
	if(input.num*use_filter<out_num)
	{
		cout <<"Inputs :"<<input.num<<" | ";
		cout <<"Filters:"<<use_filter<<" | ";
		cout <<"Max outputs:"<<input.num*use_filter<<endl;
		cout <<"Require outputs:"<<out_num<<endl;
		return false;
	}

	if(!border)
	{
		if(input.W<5||input.H<5)
		{
			cout <<"Use border mode, Output Image too small:"<<input.W-2<<" "<<input.H-2<<endl;
			return false;
		}
		output.W=input.W-2;
		output.H=input.H-2;
	}
	else
	{
		output.W=input.W;
		output.H=input.H;
	}
	output.num=out_num;
	printf("Input Image:%d*%d*%d  |  Output Image:%d*%d*%d\n",input.num,input.W,input.H,output.num,output.W,output.H);
	
	malloc_data(output);
	return true;
}

byte ReLU(double t)
{
	if(t<=0)
		return 0;
	else
		return (byte)t;
}

double mult_metrix(double m1[3][3],double m2[3][3])
{
	double t=0;
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
			t+=(m1[i][j]*m2[i][j]);
	}
	return ReLU(t);
}

void copy_metrix(byte **input,int r,int c,int W,int H,double m[3][3])
{
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			int t1=r+i;
			int t2=c+j;

			if(t1<0)t1=0;
			if(t2<0)t2=0;
			if(t1>=H)t1=H-1;
			if(t2>=W)t2=W-1;

			m[i][j]=input[t1][t2];
		}
	}
}


void convolution_once_border(byte **input,byte **output,int W,int H,int filter)
{
	double m[3][3]={0};
	for(int r=0;r<H;r++)
	{
		for(int c=0;c<W;c++)
		{
			copy_metrix(input,r-1,c-1,W,H,m);
			byte t=(byte)mult_metrix(m,Filters[filter]);
			output[r][c]=t;
		}
	}

	for(int r=0;r<H;r++)
	{
		output[r][0]=output[r][1];
		output[r][W-1]=output[r][W-2];
	}
	for(int c=0;c<W;c++)
	{
		output[0][c]=output[1][c];
		output[H-1][c]=output[H-2][c];
	}
}

void convolution_once_noborder(byte **input,byte **output,int W,int H,int filter)
{
	double m[3][3];
	for(int r=1;r<H-1;r++)
	{
		for(int c=1;c<W-1;c++)
		{
			copy_metrix(input,r-1,c-1,W,H,m);
			output[r-1][c-1]=(byte)mult_metrix(m,Filters[filter]);
		}
	}
}

bool convolution(Data input,Data &output,int out_num,int start_filter,bool border)
{
	if(!init_output(output,input,out_num,MAX_FILTERS,border))
		return false;
	int num=0;
	for(int j=0;j<MAX_FILTERS;j++)
	{
		for(int i=0;i<input.num;i++)
		{
			if(num>=out_num)
				break;
			int filter=(j+start_filter)%MAX_FILTERS;
			if(border)
				convolution_once_border(input.data[i],output.data[num],input.W,input.H,filter);
			else
				convolution_once_noborder(input.data[i],output.data[num],input.W,input.H,filter);

			//copy color
			if(border)
				std::memcpy(output.rgb[num],input.rgb[i],input.H*input.W*3);
			else
			{
				int dH=input.H-1;
				int dW=input.W-1;
				for(int r=1;r<input.H-1;r++)
					for(int c=1;c<input.W-1;c++)
					{
						output.rgb[num][((r-1)*dW+(c-1))*3]=input.rgb[i][(r*input.H+c)*3];
						output.rgb[num][((r-1)*dW+(c-1))*3+1]=input.rgb[i][(r*input.H+c)*3+1];
						output.rgb[num][((r-1)*dW+(c-1))*3+2]=input.rgb[i][(r*input.H+c)*3+2];
					}
			}
			num++;
		}
	}

	//copy color
	for(int i=0;i<out_num;i++)
	{
		
	}

	return true;
}