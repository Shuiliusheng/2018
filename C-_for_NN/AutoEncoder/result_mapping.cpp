#include"encode.h"


double **IN=A1;

double **A;//[GROUP][HIDDEN];
double **Z;//[GROUP][HIDDEN];
double **W;//[HIDDEN][HIDDEN];

double **OUT;

void classify_init_memory()
{
	OUT=(double **)malloc(GROUP*sizeof(double *));
	for(int i=0;i<GROUP;i++)
	{
		OUT[i]=(double *)malloc(LEN*sizeof(double));
	}
	for(int i=0;i<HIDDEN;i++)
	{
		for(int j=0;j<HIDDEN;j++)
			W[i][j]=(rand()%1000)/2000.0;;
	}
	cout <<"init classify training memory successfully"<<endl;
}

void classify_clear_memory()
{
	for(int i=0;i<GROUP;i++)
	{
		free(OUT[i]);
	}
	free(OUT);
	cout <<"clear classify training memory successfully"<<endl;
}

void classify_fore_group_run(double **in)
{
	for(int g=0;g<GROUP;g++)
	{
		for(int j=0;j<HIDDEN;j++)
		{
			Z[g][j]=0;
			for(int i=0;i<HIDDEN;i++)
				Z[g][j]+=(W[j][i]*in[g][i]);
			fuction(Z[g][j],A[g][j]);
		}
	}
}

double cal_sum(double **out,double **in,int i,int j)
{
	double temp=0;
	for(int g=0;g<GROUP;g++)
	{
		temp+=((A[g][j]-out[g][j])*d_fuction(Z[g][j])*in[g][i]);
	}
	return temp/GROUP;
}

void classify_back_group_run(double **in,double **out)
{
	for(int j=0;j<HIDDEN;j++)
	{
		for(int i=0;i<HIDDEN;i++)
		{
			W[j][i]=(1-eta1*lambda1)*W[j][i]-eta1*cal_sum(out,in,i,j);
		}
	}
}

double classify_lost_raw(double **out)
{
	double temp=0;
	for(int g=0;g<GROUP;g++)
	{
		for(int i=0;i<HIDDEN;i++)
			temp+=(pow((A[g][i]-out[g][i]),2));
	}
	return 0.5*temp/GROUP;
}

double classify_lost(double **out)
{
	double temp=0;
	for(int i=0;i<HIDDEN;i++)
	{
		for(int j=0;j<HIDDEN;j++)
			temp+=(pow(W[i][j],2));
	}
	return temp*lambda1*0.5+classify_lost_raw(out);
}

int train_times1=0;
void classify_training()
{

	FILE *log=fopen("classify_training_log","w");
	fprintf(log,"training_times raw_lost lost\n");

	classify_init_memory();
	init_image();
	init_label();
	
	double lost=1;
	while(fabs(lost)>0.01)
	{
		train_times1++;
		read_data(in);
		read_label(OUT);

		fore_spread_group_once(in,Z1,A1,W1,HIDDEN,LEN);

		classify_fore_group_run(IN);
		classify_back_group_run(IN,OUT);
		lost=classify_lost_raw(OUT);
		cout <<"Classify_lost "<<train_times1<<":"<<lost<<endl;
		fprintf(log,"times %d : %lf\n",train_times1,lost);
	}
	classify_save_param(W,lost,HIDDEN,HIDDEN);
	classify_clear_memory();
	fclose(log);
}

void classify_save_param(double **w,double lost,int row,int col)
{
	char str[100];
	sprintf(str,"Classify_Group_%d_eta_%lf_lambda_%lf.txt",GROUP,eta1,lambda1);
	FILE *p=fopen(str,"w");
	fprintf(p,"%lf\n",lost);

	for(int i=0;i<row;i++)
		for(int j=0;j<col;j++)
			fprintf(p,"%lf\n",W1[i][j]);
	fclose(p);
	cout <<"classify network parameters are saved in: "<<str<<endl;
}