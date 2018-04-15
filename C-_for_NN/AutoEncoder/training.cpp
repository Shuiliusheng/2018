#include"encode.h"

void raw_lost(double *in,double *out,double &lost)
{
	lost=0;
	for(int i=0;i<LEN;i++)
	{
		lost+=pow((in[i]-out[i]),2);
	}
	lost=lost/(2);
}

void sum_w(double **w,int r,int c,double &sum)
{
	sum=0;
	for(int i=0;i<r;i++)
	{
		for(int j=0;j<c;j++)
			sum+=pow(w[i][j],2);
	}
}

double lost_fuction(double **in,double **out,double **w1,double **w2,int r1,int r2,int c1,int c2)
{
	double *lost=(double *)malloc(GROUP*sizeof(double));
	for(int i=0;i<GROUP;i++)
	{
		raw_lost(in[i],out[i],lost[i]);
	}

	double sum1=0,sum2=0;
	sum_w(w1,r1,c1,sum1);
	sum_w(w2,r2,c2,sum2);

	double sum=0;
	for(int i=0;i<GROUP;i++)
		sum+=lost[i];
	return sum/GROUP+(sum1+sum2)*lambda*0.5;
}

double lost_fuction_raw(double **in,double **out)
{
	double *lost=(double *)malloc(GROUP*sizeof(double));
	for(int i=0;i<GROUP;i++)
	{
		raw_lost(in[i],out[i],lost[i]);
	}

	double sum=0;
	for(int i=0;i<GROUP;i++)
		sum+=lost[i];
	
	return sum/GROUP;
}

int train_times=0;


void save_w(double lost,double **W1,double **W2,int r1,int c1,int r2,int c2)
{
	char str[100];
	sprintf(str,"Encode_Group_%d_eta_%lf_lambda_%lf.txt",GROUP,eta,lambda);
	FILE *p=fopen(str,"w");
	fprintf(p,"%lf\n",lost);
	for(int i=0;i<r1;i++)
		for(int j=0;j<c1;j++)
			fprintf(p,"%lf\n",W1[i][j]);
	for(int i=0;i<r2;i++)
		for(int j=0;j<c2;j++)
			fprintf(p,"%lf\n",W2[i][j]);
	fclose(p);
	cout <<"encode network parameters are saved in: "<<str<<endl;
}

void encode_init_memory()
{
	W2=(double **)malloc(LEN*sizeof(double *));

	Z2=(double **)malloc(GROUP*sizeof(double *));
	A2=(double **)malloc(GROUP*sizeof(double *));

	delta1=(double **)malloc(GROUP*sizeof(double *));
	delta2=(double **)malloc(GROUP*sizeof(double *));

	for(int i=0;i<GROUP;i++)
	{
		Z2[i]=(double *)malloc(LEN*sizeof(double));
		A2[i]=(double *)malloc(LEN*sizeof(double));

		delta1[i]=(double *)malloc(HIDDEN*sizeof(double));
		delta2[i]=(double *)malloc(LEN*sizeof(double));
	}

	for(int i=0;i<LEN;i++)
	{
		W2[i]=(double *)malloc(HIDDEN*sizeof(double));
		for(int j=0;j<HIDDEN;j++)
			W2[i][j]=(rand()%1000)/2000.0;;
	}
	cout <<"init encode training memory successfully"<<endl;
}

void encode_clear_memory()
{
	for(int i=0;i<LEN;i++)
	{
		free(W2[i]);
	}
	free(W2);
	for(int i=0;i<GROUP;i++)
	{
		free(delta1[i]);
		free(delta2[i]);
		free(A2[i]);
		free(Z2[i]);
	}
	free(delta1);
	free(delta2);
	free(A2);
	free(Z2);
	cout <<"clear encode training memory successfully"<<endl;
}

void encode_training()
{
	double lost=0,lastlost=10000;
	FILE *log=fopen("encode_training_log","w");
	fprintf(log,"training_times raw_lost lost\n");

	encode_init_memory();
	if(!init_image())
		return ;
	
	while(lost-lastlost<0)
	{
		if(train_times!=0)
			lastlost=lost;
		train_times++;
		read_data(in);
		fore_spread();
		double lost1=lost_fuction(in,A2,W1,W2,HIDDEN,LEN,LEN,HIDDEN);
		lost=lost_fuction_raw(in,A2);

		fprintf(log,"times %d : %lf %lf\n",train_times,lost,lost1);
		printf("times %d : %lf %lf\n",train_times,lost,lost1);

		back_spread();
	}
	lost=lost_fuction_raw(in,A2);
	save_w(lost,W1,W2,HIDDEN,LEN,LEN,HIDDEN);

	FILE *t=fopen("g:\\hidden.txt","w");
	for(int i=0;i<GROUP;i++)
	{
		for(int j=0;j<HIDDEN;j++)
		{
			fprintf(t,"%lf ",A1[i][j]);
		}
		fprintf(t,"\n");
	}
	fclose(t);

	encode_clear_memory();

	fclose(log);
}

bool load_param(char name[],double **w,int row,int col)
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

	for(int i=0;i<row;i++)
	{
		for(int j=0;j<col;j++)
		{
			fscanf(p,"%lf",&w[i][j]);
		}
	}
	fclose(p);
	cout <<"encode parameters load successfully!"<<endl;
	return true;
}