#include"branch_predictor.h"
/*
两位条件分支预测器：测试3
测试内容：基本与测试1一致，间隔step也会重置output的统计信息
          区别：测试1中仅有一个output，所有的分支指令都更新同一个output
		        测试3中包含多个output，根据BHT的索引不同，更新不同的output
				此时output的索引由索引BHT的低若干位组成，output的个数不超过BHT表项的个数
*/

//预测器参数
Predictor_Param two_bit_test2_param;

//output 用于记录每个状态下，条件分支的跳转情况的数目
int test2_output[8192][4][2];

//用于给出每个状态应有的输出情况
char test2_predict_out[8192][4];

//用于索引test2_output和test2_predict_out两个表
int out_index_bits;

void init_two_bit_test2_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	two_bit_test2_param.index_len=index_length;
	two_bit_test2_param.inst_num=0;
	two_bit_test2_param.miss_num=0;
	two_bit_test2_param.mid_miss=0;
	two_bit_test2_param.play_step=b_result.step;
	two_bit_test2_param.table=(unsigned char*)malloc(entry_num);

	
	int out_entry_num=pow_int2(out_index_bits);
	//遍历所有表项
	for(int n=0;n<out_entry_num;n++)
	{
		//初始化predictout为饱和状态器最高位预测的状态
		test2_predict_out[n][0]=0;
		test2_predict_out[n][1]=0;
		test2_predict_out[n][2]=1;
		test2_predict_out[n][3]=1;

		//初始化output为0
		for(int i=0;i<8;i++)
			test2_output[n][i/2][i%2]=0;
	}
	for(int i=0;i<entry_num;i++)
		two_bit_test2_param.table[i]=init_state;
}

//每次在得到实际分支跳转结果之后，将其记录在output中
void test2_record_output(int64 pc,int taken)
{
	//根据pc获取BHT的索引
	int index=get_bit(pc,0,two_bit_test2_param.index_len);
	//根据BHT的索引获取低out_index_bits位，以索引output表
	int index1=get_bit(index,0,out_index_bits);
	//根据BHT的表项获取当前状态，用于索引output的第二维
	index=two_bit_test2_param.table[index];

	//更新output表
	if(taken==1)
		test2_output[index1][index][1]++;
	else
		test2_output[index1][index][0]++;
}

//间隔step之后，根据output中记录的信息，更新每个状态对应的输出
void test2_change_predict_out()
{
	//遍历所有的predictout表项
	int out_entry_num=pow_int2(out_index_bits);
	for(int n=0;n<out_entry_num;n++)
		for(int i=0;i<4;i++)
		{
			//跳转的次数多于不跳转的次数，则预测值应为跳转
			if(test2_output[n][i][1]>test2_output[n][i][0])
				test2_predict_out[n][i]=1;
			else if(test2_output[n][i][1]<test2_output[n][i][0])//跳转的次数少于不跳转的次数，则预测值应为不跳转
				test2_predict_out[n][i]=0;
			//清空之前记录的信息
			test2_output[n][i][0]=0;
			test2_output[n][i][1]=0;
		}
}

//根据pc，索引BHT和predictout,进行预测
int two_bit_test2_predict(int64 pc)
{
	//根据pc，得到BHT的索引
	int index=get_bit(pc,0,two_bit_test2_param.index_len);
	//根据BHT的索引，得到predictout的索引
	int index1=get_bit(index,0,out_index_bits);
	//根据表项进行预测
	return test2_predict_out[index1][two_bit_test2_param.table[index]];
}

//更新饱和计数器的状态
void two_bit_test2_update_saturation(int64 pc, int taken)
{
	int index=get_bit(pc,0,two_bit_test2_param.index_len);
	if(taken==1)
	{
		if(two_bit_test2_param.table[index]<3)
			two_bit_test2_param.table[index]++;
	}
	else
	{
		if((int)two_bit_test2_param.table[index]>0)
			two_bit_test2_param.table[index]--;
	}
}

//两位分支预测器的测试2的主函数
void two_bit_test2_predictor(int index_len,int init_state,int out_index_len)
{
	out_index_bits=out_index_len;
	init_two_bit_test2_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		two_bit_test2_param.inst_num++;
		//间隔step之后，更新状态输出对应表
		if(two_bit_test2_param.inst_num%two_bit_test2_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",two_bit_test2_param.play_step,1-1.0*two_bit_test2_param.mid_miss/two_bit_test2_param.play_step);
			fprintf(log_file,"%d,%lf\n",two_bit_test2_param.inst_num,1-1.0*two_bit_test2_param.mid_miss/two_bit_test2_param.play_step);
			
			//更新state out table
			test2_change_predict_out();
			two_bit_test2_param.mid_miss=0;
		}
		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//根据分支的实际结果，更新output表
		test2_record_output(pc,taken);

		//predict
		int p=two_bit_test2_predict(pc);
		//judgemet
		if(p!=taken)
		{
			two_bit_test2_param.mid_miss++;
			two_bit_test2_param.miss_num++;
		}
		//update
		two_bit_test2_update_saturation(pc,taken);
	}

	b_result.miss_inst=two_bit_test2_param.miss_num;
	b_result.total_inst=two_bit_test2_param.inst_num;
	b_result.acc=1.0-1.0*two_bit_test2_param.miss_num/two_bit_test2_param.inst_num;

	printf("two bit test2 predictor (BHT entry:%d, Predict_out entry:%d)\n",pow_int2(index_len),pow_int2(out_index_len));
	free(two_bit_test2_param.table);
}