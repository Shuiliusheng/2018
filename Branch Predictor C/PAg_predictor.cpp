#include"branch_predictor.h"
/*

PAg: pre-address bhr(BHT) and global PHT
每条条件分支指令都有自己的BHR（实际不是），此时BHR内部存储的信息变为当前条件分支指令的历史信息
在进行预测的过程中，首先根据pc，找到自己的bhr，然后根据bhr索引PHT，得到预测结果
*/

//PAg predictor的参数
Predictor_Param PAg_param;

//局部的BHT
int64 *BHT;
int bht_index_len;

//初始化PAg预测器，index_len指明BHR的位宽
void init_PAg_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	PAg_param.index_len=index_length;
	PAg_param.inst_num=0;
	PAg_param.miss_num=0;
	PAg_param.mid_miss=0;
	PAg_param.play_step=b_result.step;
	PAg_param.table=(unsigned char*)malloc(entry_num);

	for(int i=0;i<entry_num;i++)
		PAg_param.table[i]=init_state;

	//初始化BHT，表项设置为0
	entry_num=pow_int2(bht_index_len);
	BHT=(int64*)malloc(sizeof(int64)*entry_num);
	for(int i=0;i<entry_num;i++)
		BHT[i]=0;

}

//PAg的预测
int PAg_predict(int64 pc)
{
	//根据PC获取BHT的索引
	int index=get_bit(pc,0,bht_index_len);
	//根据BHT中的表项，得到PHT的索引
	index=get_bit(BHT[index],0,PAg_param.index_len);

	if(PAg_param.table[index]>=2)
		return 1;
	else
		return 0;
}

void PAg_update_saturation(int64 pc,int taken)
{
	//根据PC获取BHT的索引
	int index=get_bit(pc,0,bht_index_len);
	//根据BHT中的表项，得到PHT的索引
	index=get_bit(BHT[index],0,PAg_param.index_len);

	//更新PHT表项
	if(taken==1)
	{
		if(PAg_param.table[index]<3)
			PAg_param.table[index]++;
	}
	else
	{
		if((int)PAg_param.table[index]>0)
			PAg_param.table[index]--;
	}
}

//更新BHR，移位寄存器
void PAg_update_bhr(int64 pc, int taken)
{
	//根据PC获取BHT的索引
	int index=get_bit(pc,0,bht_index_len);
	BHT[index]=((BHT[index]<<1)+taken);
}

//PAg预测器主函数
void PAg_predictor(int pht_index_len,int init_state,int bht_index_length)
{
	bht_index_len=bht_index_length;
	init_PAg_predictor(pht_index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		PAg_param.inst_num++;
		if(PAg_param.inst_num%PAg_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",PAg_param.play_step,1-1.0*PAg_param.mid_miss/PAg_param.play_step);
			fprintf(log_file,"%d,%lf\n",PAg_param.inst_num,1-1.0*PAg_param.mid_miss/PAg_param.play_step);
			PAg_param.mid_miss=0;
		}
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//PAg的预测
		int p=PAg_predict(pc);

		//预测结果的判断
		if(p!=taken)
		{
			PAg_param.mid_miss++;
			PAg_param.miss_num++;
		}
		//更新PHT表项
		PAg_update_saturation(pc,taken);
		//更新BHR
		PAg_update_bhr(pc,taken);
	}

	b_result.miss_inst=PAg_param.miss_num;
	b_result.total_inst=PAg_param.inst_num;
	b_result.acc=1.0-1.0*PAg_param.miss_num/PAg_param.inst_num;

	printf("PAg predictor (BHT entry:%d, PHT entry:%d)\n",pow_int2(bht_index_length),pow_int2(pht_index_len));
	free(PAg_param.table);
	free(BHT);
}