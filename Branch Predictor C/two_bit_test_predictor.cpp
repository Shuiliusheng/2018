#include"branch_predictor.h"
/*
两位条件分支预测器：测试1
测试内容：间隔step步长，根据统计的信息，更新每个状态对应的输出信息
          间隔step之后，统计信息会清零，重新进行统计
		  step可变，但是所有的表项只有一个统计使用的output
*/

//预测器参数
Predictor_Param two_bit_test_param;

//output 用于记录每个状态下，条件分支的跳转情况的数目
int output[4][2];
//用于给出每个状态应有的输出情况
char predict_out[4];
void init_two_bit_test_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	two_bit_test_param.index_len=index_length;
	two_bit_test_param.inst_num=0;
	two_bit_test_param.miss_num=0;
	two_bit_test_param.mid_miss=0;
	two_bit_test_param.play_step=b_result.step;
	two_bit_test_param.table=(unsigned char*)malloc(entry_num);

	//初始化每个状态对应的输出，初始为两位饱和计数器，高位进行预测
	predict_out[0]=0;
	predict_out[1]=0;
	predict_out[2]=1;
	predict_out[3]=1;

	//初始化output为0
	for(int i=0;i<8;i++)
		output[i/2][i%2]=0;
	for(int i=0;i<entry_num;i++)
		two_bit_test_param.table[i]=init_state;
}

//每次在得到实际分支跳转结果之后，将其记录在output中
void record_output(int64 pc,int taken)
{
	//根据pc获取BHT表项中的状态值
	int index=get_bit(pc,0,two_bit_test_param.index_len);
	//根据状态值得到output中对应的索引值
	index=two_bit_test_param.table[index];

	//利用索引值，更新output
	if(taken==1)
		output[index][1]++;
	else
		output[index][0]++;
}

//间隔step之后，根据output中记录的信息，更新每个状态对应的输出
void change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		//跳转的次数多于不跳转的次数，则预测值应为跳转
		if(output[i][1]>output[i][0])
			predict_out[i]=1;
		else if(output[i][1]<output[i][0])//跳转的次数少于不跳转的次数，则预测值应为不跳转
			predict_out[i]=0;
		//清空之前记录的信息
		output[i][0]=0;
		output[i][1]=0;
	}
}

//预测，根据pc得到索引，利用BHT和状态输出对应表进行预测
int two_bit_test_predict(int64 pc)
{
	//根据pc获取BHT的索引
	int index=get_bit(pc,0,two_bit_test_param.index_len);
	//索引BHT，根据BHT表项中的状态，找到状态输出表中的预测输出
	return predict_out[two_bit_test_param.table[index]];
}

//更新BHT中的饱和计数器
void two_bit_test_update_saturation(int64 pc, int taken)
{
	int index=get_bit(pc,0,two_bit_test_param.index_len);
	if(taken==1)
	{
		if(two_bit_test_param.table[index]<3)
			two_bit_test_param.table[index]++;
	}
	else
	{
		if((int)two_bit_test_param.table[index]>0)
			two_bit_test_param.table[index]--;
	}
}

//两位分支预测器的测试1的主函数
void two_bit_test_predictor(int index_len,int init_state)
{
	init_two_bit_test_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		two_bit_test_param.inst_num++;

		//间隔step之后，更新状态输出对应表
		if(two_bit_test_param.inst_num%two_bit_test_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",two_bit_test_param.play_step,1-1.0*two_bit_test_param.mid_miss/two_bit_test_param.play_step);
			//fprintf(log_file,"%d,%lf\n",two_bit_test_param.inst_num,1-1.0*two_bit_test_param.mid_miss/two_bit_test_param.play_step);
			
			//更新state out table
			change_predict_out();
			two_bit_test_param.mid_miss=0;
		}
		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//根据分支的实际结果，更新output表
		record_output(pc,taken);

		//predict
		int p=two_bit_test_predict(pc);
		//judgemet
		if(p!=taken)
		{
			two_bit_test_param.mid_miss++;
			two_bit_test_param.miss_num++;
		}
		//update
		two_bit_test_update_saturation(pc,taken);
	}

	b_result.miss_inst=two_bit_test_param.miss_num;
	b_result.total_inst=two_bit_test_param.inst_num;
	b_result.acc=1.0-1.0*two_bit_test_param.miss_num/two_bit_test_param.inst_num;

	printf("two bit test0 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(two_bit_test_param.table);
}