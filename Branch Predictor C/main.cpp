#include"branch_predictor.h"

char trace_name[100]="G:\\×ÀÃæ\\2018_summer\\sniper_branch\\branch_log\\branch_fmm_log";
char log_name[200]="G:\\×ÀÃæ\\2018_summer\\sniper_branch\\branch_log\\branch_fmm_log_PAg_step_500.csv";

Branch_result b_result;
FILE *log_file;
int step=500;
bool init()
{
	b_result.step=step;
	if(!init_trace(trace_name))
		return false;
	return true;
}

void show_result()
{
	printf("trace:%s\n",trace_name);
	printf("instruction number: %d, miss predict number:%d\n",b_result.total_inst,b_result.miss_inst);
	printf("predict accuracy:%lf\n",b_result.acc);
}

int main(int argv,char **args)
{
	int out_index_len=0;
	int BHT_index_len=0;
	if(argv==4)
	{
		sscanf(args[1],"%d",&step);
		sscanf(args[2],"%d",&out_index_len);
		sscanf(args[2],"%d",&BHT_index_len);
		strcpy(trace_name,args[3]);
		sprintf(log_name,"%s_%s_step_%d_bht_index_%d.csv",args[3],"two_bit_test3_predictor",step,out_index_len);
	}
	else if(argv==5)
	{
		sscanf(args[1],"%d",&step);
		sscanf(args[2],"%d",&out_index_len);
		sscanf(args[3],"%d",&BHT_index_len);
		strcpy(trace_name,args[4]);
		sprintf(log_name,"%s_%s_step_%d_bht_index_%d_out_index_%d.csv",args[4],"two_bit_test3_predictor",step,BHT_index_len,out_index_len);
	}
	else if(argv==3)
	{
		sscanf(args[1],"%d",&step);
		strcpy(trace_name,args[2]);
		sprintf(log_name,"%s_%s_step_%d.csv",args[2],"two_bit_test_predictor",step);
	}
	//log_file=fopen(log_name,"w");

	char name[200];
	for(int i=0;i<16;i++)
	{
		init();
		sprintf(name,"G:\\×ÀÃæ\\2018_summer\\sniper_branch\\branch_log\\branch_%s_log_two_bit_step_500_output_%d.csv","fmm",i);
		log_file=fopen(name,"w");
		two_bit_test4_predictor(10,0,i);
		fclose(log_file);
	}
	
	//for(int i=0;i<=20;i++)
	//{
	//	if(init())
	//	{
	//		int temp=0,temp1=0;
	//		if(i<out_index_len)
	//			temp=i;
	//		else
	//			temp=out_index_len;

	//		if(i<BHT_index_len)
	//			temp1=i;
	//		else
	//			temp1=BHT_index_len;

	//		//PAg_test2_predictor(i,0,temp1,temp);
	//		two_bit_test_predictor(i,0);
	//		//show_result();
	//		fprintf(log_file,"%d,%lf\n",i,b_result.acc);
	//	}
	//}

	fclose(log_file);
	return 0;
}

//one bit
/*
if(init())
{
	one_bit_predictor(i,0);
	show_result();
}
*/