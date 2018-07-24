#include"branch_predictor.h"
/*
基本两位分支预测器
BHT的表项为两位饱和计数器，预测时使用高位进行预测
可选参数：BHT表项个数，BHT的初始状态
*/


//基本两位预测器的内部参数
Predictor_Param two_bit_param;

//初始化两位预测器的基本信息
void init_two_bit_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	two_bit_param.index_len=index_length;
	two_bit_param.inst_num=0;
	two_bit_param.miss_num=0;
	two_bit_param.mid_miss=0;
	two_bit_param.play_step=b_result.step;
	two_bit_param.table=(unsigned char*)malloc(entry_num);
	for(int i=0;i<entry_num;i++)
		two_bit_param.table[i]=init_state;
}

//根据PC进行预测器
int two_bit_predict(int64 pc)
{
	//获取BHT的索引
	int index=get_bit(pc,0,two_bit_param.index_len);
	//根据BHT表项的高位进行预测
	if(two_bit_param.table[index]>=2)
		return 1;
	else
		return 0;
}

//根据BHT表项中的饱和计数器的状态
void two_bit_update_saturation(int64 pc, int taken)
{
	//获取BHT的索引
	int index=get_bit(pc,0,two_bit_param.index_len);
	//更新累加器状态
	if(taken==1)
	{
		if(two_bit_param.table[index]<3)
			two_bit_param.table[index]++;
	}
	else
	{
		if((int)two_bit_param.table[index]>0)
			two_bit_param.table[index]--;
	}
}


//两位分支预测器主函数
void two_bit_predictor(int index_len,int init_state)
{
	init_two_bit_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		two_bit_param.inst_num++;
		if(two_bit_param.inst_num%two_bit_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",two_bit_param.play_step,1-1.0*two_bit_param.mid_miss/two_bit_param.play_step);
			fprintf(log_file,"%d,%lf\n",two_bit_param.inst_num,1-1.0*two_bit_param.mid_miss/two_bit_param.play_step);
			two_bit_param.mid_miss=0;
		}

		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//predict
		int p=two_bit_predict(pc);
		//judgemet
		if(p!=taken)
		{
			two_bit_param.mid_miss++;
			two_bit_param.miss_num++;
		}
		//update
		two_bit_update_saturation(pc,taken);
	}

	b_result.miss_inst=two_bit_param.miss_num;
	b_result.total_inst=two_bit_param.inst_num;
	b_result.acc=1.0-1.0*two_bit_param.miss_num/two_bit_param.inst_num;

	printf("two bit predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(two_bit_param.table);
}