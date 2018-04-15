#include"feature.h"

void read_data(char filename[],Data &output)
{
	Mat src=imread(filename);
	printf("Raw Image Size (%d*%d) convert to (500*400)\n",src.cols,src.rows);
	resize(src,src,Size(500,400),0,0,CV_INTER_NN);
	output.num=1;
	output.H=src.rows;
	output.W=src.cols;
	malloc_data(output);

	for(int i=0;i<output.H*output.W*3;i++)
		output.rgb[0][i]=src.data[i];

	for(int r=0;r<output.H;r++)
	{
		for(int c=0;c<output.W;c++)
		{
			unsigned int b=output.rgb[0][(r*output.W+c)*3];
			unsigned int g=output.rgb[0][(r*output.W+c)*3+1];
			unsigned int r1=output.rgb[0][(r*output.W+c)*3+2];
			output.data[0][r][c]=(0.2989*r1+0.5870*g+0.1140*b);
		}
	}
	//imshow("input",src);
}