#include"branch_predictor.h"
/*
基本一位预测器，可选参数：BHT的表项数，BHT的初始状态
*/

//存储预测器的参数：BHT、索引BHT的位宽、指令数、预测错误数、步长等
Predictor_Param one_bit_param;

//一位预测器的初始化，初始BHT的索引位宽，BHT的初始状态等其它信息
void init_one_bit_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	one_bit_param.index_len=index_length;
	one_bit_param.inst_num=0;
	one_bit_param.miss_num=0;
	one_bit_param.mid_miss=0;
	one_bit_param.play_step=b_result.step;
	one_bit_param.table=(unsigned char*)malloc(entry_num);
	for(int i=0;i<entry_num;i++)
		one_bit_param.table[i]=init_state;
}

//使用一位预测器根据pc低位索引BHT，进行预测
int one_bit_predict(int64 pc)
{
	//获取索引
	int index=get_bit(pc,0,one_bit_param.index_len);
	//根据BHT返回预测值
	return (int)one_bit_param.table[index];
}

//根据pc和实际的跳转情况，更新BHT
void one_bit_update(int64 pc, int taken)
{
	//获取索引
	int index=get_bit(pc,0,one_bit_param.index_len);
	//根据taken更新
	one_bit_param.table[index]=(unsigned char)taken;
}

//一位预测器的主函数
//参数：index_len  索引的位宽
//      init_state BHT的初始状态
void one_bit_predictor(int index_len,int init_state)
{
	init_one_bit_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		//指令数加一
		one_bit_param.inst_num++;
		//间隔step显示一次step期间的结果
		if(one_bit_param.inst_num%one_bit_param.play_step==0)
		{
			printf("step len:%d, hit rate: %lf\n",one_bit_param.play_step,1-1.0*one_bit_param.mid_miss/one_bit_param.play_step);
			one_bit_param.mid_miss=0;
		}
		//读取下一条条件转移指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//predict
		int p=one_bit_predict(pc);
		//judgemet
		if(p!=taken)
		{
			one_bit_param.mid_miss++;
			one_bit_param.miss_num++;
		}
		//update
		one_bit_update(pc,taken);
	}

	//将结果传递给全局的信息记录
	b_result.miss_inst=one_bit_param.miss_num;
	b_result.total_inst=one_bit_param.inst_num;
	b_result.acc=1.0-1.0*one_bit_param.miss_num/one_bit_param.inst_num;

	//释放申请的内存
	printf("one bit predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(one_bit_param.table);
}