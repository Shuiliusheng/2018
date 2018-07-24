#include"branch_predictor.h"

FILE *trace;

bool init_trace(char filename[])
{
	trace=NULL;
	trace=fopen(filename,"r");
	if(trace==NULL)
	{
		cout <<filename<<" is not exist!"<<endl;
		return false;
	}
	cout <<"instruct trace is ready!"<<endl;
	return true;
}

bool read_next_inst(int64 &pc, int &taken, int64 &targetpc)
{
	if(feof(trace))
	{
		fclose(trace);
		return false;
	}
	char str[100];
	fgets(str,100,trace);
	sscanf(str,"branch:%I64u %d %I64u",&pc,&taken,&targetpc);
	return true;
}