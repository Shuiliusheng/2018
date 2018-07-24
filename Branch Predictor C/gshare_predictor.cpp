#include"branch_predictor.h"

/*
	gshare predictor:基本与GAg一致，
	区别在于gshare使用的索引变为pc和BHR的异或之后的结果
	其中使用的PC位数和BHR的位数一致
	可选参数：PC和BHR使用的位宽和PHT表项中的初始状态
*/

//预测器参数
Predictor_Param gshare_param;

//全局的BHR
int64 gshare_bhr;

//初始化gshare预测器，index_len指明BHR位宽和异或时使用的pc的位数
void init_gshare_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	gshare_param.index_len=index_length;
	gshare_param.inst_num=0;
	gshare_param.miss_num=0;
	gshare_param.mid_miss=0;
	gshare_param.play_step=b_result.step;
	gshare_param.table=(unsigned char*)malloc(entry_num);
	//初始化BHR
	gshare_bhr=0;
	for(int i=0;i<entry_num;i++)
		gshare_param.table[i]=init_state;
}

//使用gshare进行预测
int gshare_predict(int64 pc)
{
	//使用BHR获取索引1
	int index=get_bit(gshare_bhr,0,gshare_param.index_len);
	//使用pc获取索引2，均为低index_len位
	int index1=get_bit(pc,0,gshare_param.index_len);
	//两个索引进行异或操作，得到实际的索引
	index=index1^index;
	//根据索引找到PHT表项，进行预测
	if(gshare_param.table[index]>=2)
		return 1;
	else
		return 0;
}

//更新饱和计数器状态
void gshare_update_saturation(int64 pc,int taken)
{
	//使用BHR获取索引1
	int index=get_bit(gshare_bhr,0,gshare_param.index_len);
	//使用pc获取索引2，均为低index_len位
	int index1=get_bit(pc,0,gshare_param.index_len);
	//两个索引进行异或操作，得到实际的索引
	index=index1^index;

	if(taken==1)
	{
		if(gshare_param.table[index]<3)
			gshare_param.table[index]++;
	}
	else
	{
		if((int)gshare_param.table[index]>0)
			gshare_param.table[index]--;
	}
}

//更新BHR，移位寄存器
void gshare_update_gshare_bhr(int taken)
{
	gshare_bhr=((gshare_bhr<<1)+taken);
}

//gshare预测器主函数
void gshare_predictor(int index_len,int init_state)
{
	init_gshare_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		gshare_param.inst_num++;
		if(gshare_param.inst_num%gshare_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",gshare_param.play_step,1-1.0*gshare_param.mid_miss/gshare_param.play_step);
			fprintf(log_file,"%d,%lf\n",gshare_param.inst_num,1-1.0*gshare_param.mid_miss/gshare_param.play_step);
			gshare_param.mid_miss=0;
		}
		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//predict
		int p=gshare_predict(pc);
		//judgemet
		if(p!=taken)
		{
			gshare_param.mid_miss++;
			gshare_param.miss_num++;
		}
		//update
		gshare_update_saturation(pc,taken);

		//更新BHR
		gshare_update_gshare_bhr(taken);
	}

	b_result.miss_inst=gshare_param.miss_num;
	b_result.total_inst=gshare_param.inst_num;
	b_result.acc=1.0-1.0*gshare_param.miss_num/gshare_param.inst_num;

	printf("gshare predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(gshare_param.table);
}