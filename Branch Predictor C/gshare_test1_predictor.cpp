#include"branch_predictor.h"

/*
	gshare predictor test1:基本与gshare一致，
	区别在于gshare test1增加了output和predictout，从而动态的调节自动机状态的输出
	可选参数：PC和BHR使用的位宽和PHT表项中的初始状态
*/

//预测器参数
Predictor_Param gshare_test1_param;
//全局BHR寄存器
int64 gshare_test1_bhr;
//用于统计每个状态上发生的分支的信息
int gshare_test1_output[4][2];
//状态输出对应表
char gshare_test1_predict_out[4];

//初始化
void init_gshare_test1_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	gshare_test1_param.index_len=index_length;
	gshare_test1_param.inst_num=0;
	gshare_test1_param.miss_num=0;
	gshare_test1_param.mid_miss=0;
	gshare_test1_param.play_step=b_result.step;
	gshare_test1_param.table=(unsigned char*)malloc(entry_num);
	//初始化BHR为0
	gshare_test1_bhr=0;
	//初始化每个状态对应的输出，初始为两位饱和计数器，高位进行预测
	gshare_test1_predict_out[0]=0;
	gshare_test1_predict_out[1]=0;
	gshare_test1_predict_out[2]=1;
	gshare_test1_predict_out[3]=1;
	//初始化output为0
	for(int i=0;i<8;i++)
		gshare_test1_output[i/2][i%2]=0;

	for(int i=0;i<entry_num;i++)
		gshare_test1_param.table[i]=init_state;
}

//每次在得到实际分支跳转结果之后，将其记录在output中
void gshare_test1_record_output(int64 pc,int taken)
{
	//根据BHR，获取索引1
	int index=get_bit(gshare_test1_bhr,0,gshare_test1_param.index_len);
	//根据pc，获取索引2
	int index1=get_bit(pc,0,gshare_test1_param.index_len);
	//两个索引进行异或操作，得到最终的索引
	index=index1^index;
	//根据索引，找到PHT中的表项，得到其中的状态值
	index=gshare_test1_param.table[index];
	//根据状态值更新output
	if(taken==1)
		gshare_test1_output[index][1]++;
	else
		gshare_test1_output[index][0]++;
}

//间隔step之后，根据output中记录的信息，更新每个状态对应的输出
void gshare_test1_change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		if(gshare_test1_output[i][1]>gshare_test1_output[i][0])
			gshare_test1_predict_out[i]=1;
		else if(gshare_test1_output[i][1]<gshare_test1_output[i][0])
			gshare_test1_predict_out[i]=0;
		gshare_test1_output[i][0]=0;
		gshare_test1_output[i][1]=0;
	}
}

//预测
int gshare_test1_predict(int64 pc)
{
	//使用BHR获取索引1
	int index=get_bit(gshare_test1_bhr,0,gshare_test1_param.index_len);
	//使用pc获取索引2，均为低index_len位
	int index1=get_bit(pc,0,gshare_test1_param.index_len);
	//两个索引进行异或操作，得到实际的索引
	index=index1^index;
	//根据PHT索引，PHT和predictout进行预测
	return gshare_test1_predict_out[gshare_test1_param.table[index]];
}

//更新PHT的表项
void gshare_test1_update_saturation(int64 pc,int taken)
{
	int index=get_bit(gshare_test1_bhr,0,gshare_test1_param.index_len);
	int index1=get_bit(pc,0,gshare_test1_param.index_len);
	index=index1^index;
	if(taken==1)
	{
		if(gshare_test1_param.table[index]<3)
			gshare_test1_param.table[index]++;
	}
	else
	{
		if((int)gshare_test1_param.table[index]>0)
			gshare_test1_param.table[index]--;
	}
}
//更新BHR，移位寄存器
void gshare_test1_update_gshare_test1_bhr(int taken)
{
	gshare_test1_bhr=((gshare_test1_bhr<<1)+taken);
}
//gshare test1预测器主函数
void gshare_test1_predictor(int index_len,int init_state)
{
	init_gshare_test1_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		gshare_test1_param.inst_num++;
		if(gshare_test1_param.inst_num%gshare_test1_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",gshare_test1_param.play_step,1-1.0*gshare_test1_param.mid_miss/gshare_test1_param.play_step);
			//fprintf(log_file,"%d,%lf\n",gshare_test1_param.inst_num,1-1.0*gshare_test1_param.mid_miss/gshare_test1_param.play_step);
			gshare_test1_param.mid_miss=0;
			
			//更新state out table
			gshare_test1_change_predict_out();
		}
		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//根据分支的实际结果，更新output表
		gshare_test1_record_output(pc,taken);

		//predict
		int p=gshare_test1_predict(pc);
		//judgemet
		if(p!=taken)
		{
			gshare_test1_param.mid_miss++;
			gshare_test1_param.miss_num++;
		}
		//update
		gshare_test1_update_saturation(pc,taken);

		//更新BHR
		gshare_test1_update_gshare_test1_bhr(taken);
	}

	b_result.miss_inst=gshare_test1_param.miss_num;
	b_result.total_inst=gshare_test1_param.inst_num;
	b_result.acc=1.0-1.0*gshare_test1_param.miss_num/gshare_test1_param.inst_num;

	printf("gshare_test1 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(gshare_test1_param.table);
}