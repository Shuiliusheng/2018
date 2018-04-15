#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include <iostream>
using namespace std;

#include "cv.h"
#include "highgui.h" 
using namespace cv;

#define MAX_FILTERS 26
typedef unsigned char byte;

typedef struct{
	int W;
	int H;
	int num;
	byte ***data;
	byte **rgb;
}Data;



void init_filters();
void malloc_data(Data &output);
byte ReLU(double t);
void save_data(Data input,int maxW,int maxH,const char prefix[],int expand);
void read_data(char filename[],Data &output);
bool convolution(Data input,Data &output,int out_num,int start_filter,bool border);
bool max_pooling(Data input,Data &output,int step);



extern double Filters[MAX_FILTERS][3][3];