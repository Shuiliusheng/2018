#include"encode.h"

double **in;//[GROUP][LEN];

double **W1;//[HIDDEN][LEN];// the first argu
double **W2;//[LEN][HIDDEN];// the second argu, setting like the formula

double **Z1;//[GROUP][HIDDEN];//the first layer input and output
double **A1;//[GROUP][HIDDEN];
double **delta1;//[GROUP][HIDDEN];


double **Z2;//[GROUP][LEN];//the second layer input and output
double **A2;//[GROUP][LEN];
double **delta2;//[GROUP][LEN];

void init()
{
	in=(double **)malloc(GROUP*sizeof(double *));
	W1=(double **)malloc(HIDDEN*sizeof(double *));
	W=(double **)malloc(HIDDEN*sizeof(double *));

	Z1=(double **)malloc(GROUP*sizeof(double *));
	A1=(double **)malloc(GROUP*sizeof(double *));

	A=(double **)malloc(GROUP*sizeof(double *));
	Z=(double **)malloc(GROUP*sizeof(double *));

	for(int i=0;i<GROUP;i++)
	{
		in[i]=(double *)malloc(LEN*sizeof(double));
		Z1[i]=(double *)malloc(HIDDEN*sizeof(double));
		A1[i]=(double *)malloc(HIDDEN*sizeof(double));

		A[i]=(double *)malloc(HIDDEN*sizeof(double));
		Z[i]=(double *)malloc(HIDDEN*sizeof(double));
	}
	srand( (unsigned)time( NULL ) );
	for(int i=0;i<HIDDEN;i++)
	{
		W[i]=(double *)malloc(HIDDEN*sizeof(double));
		W1[i]=(double *)malloc(LEN*sizeof(double));
		for(int j=0;j<HIDDEN;j++)
		{
			W[i][j]=(rand()%1000)/2000.0;
		}
		for(int j=0;j<LEN;j++)
		{
			W1[i][j]=(rand()%1000)/2000.0;
		}
	}
	
	cout <<"initial common memory successfully"<<endl;
}