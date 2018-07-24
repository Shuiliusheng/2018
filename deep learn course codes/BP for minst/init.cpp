#include"nn_minst.h"

double **in;//[GROUP][LEN];
double **out;//[GROUP][OUT];

//W : the net parameters
//Z : the input for activation function
//A : the output of activation function
//delta: the mid result for calculating gradient


double **W1;//[HIDDEN1][LEN];// the first argu
double **W2;//[HIDDEN2][HIDDEN1];// the second argu, setting like the formula
double **W3;//[OUT][HIDDEN2]

double **Z1;//[GROUP][HIDDEN1];//the first layer input and output
double **A1;//[GROUP][HIDDEN1];
double **delta1;//[GROUP][HIDDEN1];


double **Z2;//[GROUP][HIDDEN2];//the second layer input and output
double **A2;//[GROUP][HIDDEN2];
double **delta2;//[GROUP][HIDDEN2];

double **Z3;//[GROUP][OUT];//the second layer input and output
double **A3;//[GROUP][OUT];
double **delta3;//[GROUP][OUT];

/*
����ѵ���Ͳ��Ա���Ĳ������ڴ�ռ�
�������� in����� out��������� W����Ԫ������ Z����Ԫ����� A

��ʼ���������
������(rand()%2000-1000)/3000.0 
��Χ����-0.33��0.33��
*/
void init()
{
	in=(double **)malloc(GROUP*sizeof(double *));
	out=(double **)malloc(GROUP*sizeof(double *));

	W1=(double **)malloc(HIDDEN1*sizeof(double *));
	W2=(double **)malloc(HIDDEN2*sizeof(double *));
	W3=(double **)malloc(OUT*sizeof(double *));

	Z1=(double **)malloc(GROUP*sizeof(double *));
	A1=(double **)malloc(GROUP*sizeof(double *));

	Z2=(double **)malloc(GROUP*sizeof(double *));
	A2=(double **)malloc(GROUP*sizeof(double *));

	Z3=(double **)malloc(GROUP*sizeof(double *));
	A3=(double **)malloc(GROUP*sizeof(double *));

	for(int i=0;i<GROUP;i++)
	{
		in[i]=(double *)malloc(LEN*sizeof(double));
		out[i]=(double *)malloc(OUT*sizeof(double));

		Z1[i]=(double *)malloc(HIDDEN1*sizeof(double));
		A1[i]=(double *)malloc(HIDDEN1*sizeof(double));

		Z2[i]=(double *)malloc(HIDDEN2*sizeof(double));
		A2[i]=(double *)malloc(HIDDEN2*sizeof(double));

		Z3[i]=(double *)malloc(OUT*sizeof(double));
		A3[i]=(double *)malloc(OUT*sizeof(double));
	}

	srand((unsigned)time(NULL));
	for(int i=0;i<HIDDEN1;i++)
	{
		W1[i]=(double *)malloc(LEN*sizeof(double));
		for(int j=0;j<LEN;j++)
			W1[i][j]=(rand()%2000-1000)/3000.0;
	}

	for(int i=0;i<HIDDEN2;i++)
	{
		W2[i]=(double *)malloc(HIDDEN1*sizeof(double));
		for(int j=0;j<HIDDEN1;j++)
			W2[i][j]=(rand()%2000-1000)/3000.0;
	}

	for(int i=0;i<OUT;i++)
	{
		W3[i]=(double *)malloc(HIDDEN2*sizeof(double));
		for(int j=0;j<HIDDEN2;j++)
			W3[i][j]=(rand()%2000-1000)/3000.0;
	}
	
	cout <<"initial common memory successfully"<<endl;
}
 
/*
��ʼ��ѵ���׶�ʹ�õĲ���
�ڷ��򴫲��׶ε��м����
delta
*/
void train_init_memory()
{
	delta1=(double **)malloc(GROUP*sizeof(double *));
	delta2=(double **)malloc(GROUP*sizeof(double *));
	delta3=(double **)malloc(GROUP*sizeof(double *));

	for(int i=0;i<GROUP;i++)
	{
		delta1[i]=(double *)malloc(HIDDEN1*sizeof(double));
		delta2[i]=(double *)malloc(HIDDEN2*sizeof(double));
		delta3[i]=(double *)malloc(OUT*sizeof(double));
	}
	cout <<"init encode training memory successfully"<<endl;
}

/*
���ѵ���׶�����Ŀռ�
delta
*/
void train_clear_memory()
{
	for(int i=0;i<GROUP;i++)
	{
		free(delta1[i]);
		free(delta2[i]);
		free(delta3[i]);
	}
	free(delta1);
	free(delta2);
	free(delta3);
	cout <<"clear encode training memory successfully"<<endl;
}


/*
�����ļ�������ѵ���õ����������W
name:�������ڵ��ļ���������ļ������ڣ�����false
W1:��һ������Ĳ���
W2:�ڶ����������
W3:�������������
l0:�������
l1,l2,l3:��һ�㵽������Ŀ��
*/
bool load_param(char name[],double **W1,double **W2,double **W3,int l0,int l1,int l2,int l3)
{
	FILE *p=NULL;
	p=fopen(name,"r");
	if(p==NULL)
	{
		cout <<name<<" is not exist!"<<endl;
		return false;
	}

	double lost=0;
	fscanf(p,"%lf",&lost);

	for(int i=0;i<l1;i++)
		for(int j=0;j<l0;j++)
			fscanf(p,"%lf\n",&W1[i][j]);

	for(int i=0;i<l2;i++)
		for(int j=0;j<l1;j++)
			fscanf(p,"%lf\n",&W2[i][j]);

	for(int i=0;i<l3;i++)
		for(int j=0;j<l2;j++)
			fscanf(p,"%lf\n",&W3[i][j]);

	fclose(p);
	cout <<"encode parameters load successfully!"<<endl;
	return true;
}