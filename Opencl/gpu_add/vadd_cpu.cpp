#include<stdio.h>
#include<iostream>
#include <sys/time.h>
#include <unistd.h>
#include<stdlib.h>
using namespace std;

int main()
{
	int n=1;
	while(n!=0)
	{
		cout <<"input the length:";
		cin >>n;
		int *A=(int*)malloc(sizeof(int)*n);
		int *B=(int*)malloc(sizeof(int)*n);
		int *C=(int*)malloc(sizeof(int)*n);
		for(int i=0;i<n;i++)
		{
			A[i]=i;
			B[i]=i;
		}
		struct timeval tv,tv1;
		gettimeofday(&tv,NULL);
		for(int i=0;i<n;i++)
			C[i]=A[i]+B[i];
		gettimeofday(&tv1,NULL);
		long cost_time =(tv1.tv_usec)-(tv.tv_usec);
		printf("Cost time: %ld us\n", cost_time);
		free(A);
		free(B);
		free(C);
	}
	return 0;
}