#include"branch_predictor.h"
/*
GAp:global BHR and global PHT predictor
BHR记录着前k条条件分支的跳转情况，用于索引PHT表
可选参数：BHR的位宽和PHT表项中的初始状态
*/

//GAg predictor的参数
Predictor_Param GAg_param;

//全局的BHR
int64 bhr;

//初始化GAg预测器，index_len指明BHR的位宽
void init_GAg_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	GAg_param.index_len=index_length;
	GAg_param.inst_num=0;
	GAg_param.miss_num=0;
	GAg_param.mid_miss=0;
	GAg_param.play_step=b_result.step;
	GAg_param.table=(unsigned char*)malloc(entry_num);
	
	//初始化BHR为0
	bhr=0;
	for(int i=0;i<entry_num;i++)
		GAg_param.table[i]=init_state;
}

//GAg的预测
int GAg_predict()
{
	//根据BHR的低index_len位获取索引值
	int index=get_bit(bhr,0,GAg_param.index_len);
	//根据PHT表项中的最高位进行预测
	if(GAg_param.table[index]>=2)
		return 1;
	else
		return 0;
}

//更新PHT
void GAg_update_saturation(int taken)
{
	//根据BHR获取PHT的索引
	int index=get_bit(bhr,0,GAg_param.index_len);
	//更新PHT表项
	if(taken==1)
	{
		if(GAg_param.table[index]<3)
			GAg_param.table[index]++;
	}
	else
	{
		if((int)GAg_param.table[index]>0)
			GAg_param.table[index]--;
	}
}

//更新BHR，移位寄存器
void GAg_update_bhr(int taken)
{
	bhr=((bhr<<1)+taken);
}

//GAg预测器主函数
void GAg_predictor(int index_len,int init_state)
{
	init_GAg_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		GAg_param.inst_num++;
		if(GAg_param.inst_num%GAg_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",GAg_param.play_step,1-1.0*GAg_param.mid_miss/GAg_param.play_step);
			fprintf(log_file,"%d,%lf\n",GAg_param.inst_num,1-1.0*GAg_param.mid_miss/GAg_param.play_step);
			GAg_param.mid_miss=0;
		}
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//GAg的预测
		int p=GAg_predict();

		//预测结果的判断
		if(p!=taken)
		{
			GAg_param.mid_miss++;
			GAg_param.miss_num++;
		}
		//更新PHT表项
		GAg_update_saturation(taken);
		//更新BHR
		GAg_update_bhr(taken);
	}

	b_result.miss_inst=GAg_param.miss_num;
	b_result.total_inst=GAg_param.inst_num;
	b_result.acc=1.0-1.0*GAg_param.miss_num/GAg_param.inst_num;

	printf("GAg predictor (PHT entry:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(GAg_param.table);
}