#include"branch_predictor.h"
/*
	GAg test1：基本与GAg预测器一致
	           区别在于使用了output表和状态输出对应表，动态的更改PHT表项的状态机
	可选参数：BHR的位宽和PHT表项中的初始状态
*/

//GAg预测器参数
Predictor_Param GAg_test1_param;

//BHR
int64 test1_bhr;

//用于统计每个状态上发生的分支的信息
int GAg_test1_output[4][2];
//状态输出对应表
char GAg_test1_predict_out[4];

//初始化
void init_GAg_test1_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	GAg_test1_param.index_len=index_length;
	GAg_test1_param.inst_num=0;
	GAg_test1_param.miss_num=0;
	GAg_test1_param.mid_miss=0;
	GAg_test1_param.play_step=b_result.step;
	GAg_test1_param.table=(unsigned char*)malloc(entry_num);
	//初始化BHR为0
	test1_bhr=0;

	//初始化每个状态对应的输出，初始为两位饱和计数器，高位进行预测
	GAg_test1_predict_out[0]=0;
	GAg_test1_predict_out[1]=0;
	GAg_test1_predict_out[2]=1;
	GAg_test1_predict_out[3]=1;

	//初始化output为0
	for(int i=0;i<8;i++)
		GAg_test1_output[i/2][i%2]=0;

	for(int i=0;i<entry_num;i++)
		GAg_test1_param.table[i]=init_state;
}

//每次在得到实际分支跳转结果之后，将其记录在output中
void GAg_test1_record_output(int taken)
{
	//根据BHR获取PHT的索引
	int index=get_bit(test1_bhr,0,GAg_test1_param.index_len);
	//获取PHT中的状态值
	index=GAg_test1_param.table[index];

	if(taken==1)
		GAg_test1_output[index][1]++;
	else
		GAg_test1_output[index][0]++;
}

//间隔step之后，根据output中记录的信息，更新每个状态对应的输出
void GAg_test1_change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		if(GAg_test1_output[i][1]>GAg_test1_output[i][0])
			GAg_test1_predict_out[i]=1;
		else if(GAg_test1_output[i][1]<GAg_test1_output[i][0])
			GAg_test1_predict_out[i]=0;
		//重置统计的信息
		GAg_test1_output[i][0]=0;
		GAg_test1_output[i][1]=0;
	}
}

//预测
int GAg_test1_predict()
{
	//根据BHR获取PHT的索引
	int index=get_bit(test1_bhr,0,GAg_test1_param.index_len);
	//根据PHT表项的内容和状态输出对应表进行预测
	return GAg_test1_predict_out[GAg_test1_param.table[index]];
}

//更新PHT的表项
void GAg_test1_update_saturation(int taken)
{
	int index=get_bit(test1_bhr,0,GAg_test1_param.index_len);
	if(taken==1)
	{
		if(GAg_test1_param.table[index]<3)
			GAg_test1_param.table[index]++;
	}
	else
	{
		if((int)GAg_test1_param.table[index]>0)
			GAg_test1_param.table[index]--;
	}
}

//更新BHR
void GAg_test1_update_test1_bhr(int taken)
{
	test1_bhr=((test1_bhr<<1)+taken);
}

//GAg test1 主函数
void GAg_test1_predictor(int index_len,int init_state)
{
	init_GAg_test1_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		GAg_test1_param.inst_num++;
		if(GAg_test1_param.inst_num%GAg_test1_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",GAg_test1_param.play_step,1-1.0*GAg_test1_param.mid_miss/GAg_test1_param.play_step);
			//fprintf(log_file,"%d,%lf\n",GAg_test1_param.inst_num,1-1.0*GAg_test1_param.mid_miss/GAg_test1_param.play_step);
			GAg_test1_param.mid_miss=0;
			
			//更新state out table
			GAg_test1_change_predict_out();
		}
		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//根据分支的实际结果，更新output表
		GAg_test1_record_output(taken);

		//predict
		int p=GAg_test1_predict();
		//judgemet
		if(p!=taken)
		{
			GAg_test1_param.mid_miss++;
			GAg_test1_param.miss_num++;
		}
		//update
		GAg_test1_update_saturation(taken);
		//更新BHR
		GAg_test1_update_test1_bhr(taken);
	}

	b_result.miss_inst=GAg_test1_param.miss_num;
	b_result.total_inst=GAg_test1_param.inst_num;
	b_result.acc=1.0-1.0*GAg_test1_param.miss_num/GAg_test1_param.inst_num;

	printf("GAg_test1 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(GAg_test1_param.table);
}