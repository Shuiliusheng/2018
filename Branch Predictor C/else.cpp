#include"branch_predictor.h"
int64 get_bit(int64 src, int start, int num)
{
	int64 temp = pow_int2(num);
	src=src>>start;
	return src%temp;
}

int pow_int2(int len)
{
	int r=1;
	for(int i=0;i<len;i++)
		r=r*2;
	return r;
}