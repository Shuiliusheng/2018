#include"branch_predictor.h"
/*
两位条件分支预测器：测试2
测试内容：与test1的测试内容基本一致
		  区别：间隔step步长之后，统计的output信息不会重置
*/

//预测器参数
Predictor_Param two_bit_test1_param;

//test1_output 用于记录每个状态下，条件分支的跳转情况的数目
int test1_output[4][2];
//用于给出每个状态应有的输出情况，状态输出对应表
char test1_predict_out[4];

void init_two_bit_test1_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	two_bit_test1_param.index_len=index_length;
	two_bit_test1_param.inst_num=0;
	two_bit_test1_param.miss_num=0;
	two_bit_test1_param.mid_miss=0;
	two_bit_test1_param.play_step=b_result.step;
	two_bit_test1_param.table=(unsigned char*)malloc(entry_num);
	
	//初始化每个状态对应的输出，初始为两位饱和计数器，高位进行预测
	test1_predict_out[0]=0;
	test1_predict_out[1]=0;
	test1_predict_out[2]=1;
	test1_predict_out[3]=1;

	//初始化test1_output为0
	for(int i=0;i<8;i++)
		test1_output[i/2][i%2]=0;
	for(int i=0;i<entry_num;i++)
		two_bit_test1_param.table[i]=init_state;
}

//每次在得到实际分支跳转结果之后，将其记录在output中
void test1_record_output(int64 pc,int taken)
{
	//根据pc获取BHT表项中的状态值
	int index=get_bit(pc,0,two_bit_test1_param.index_len);
	//根据状态值得到test1_output中对应的索引值
	index=two_bit_test1_param.table[index];

	//利用索引值，更新test1_output
	if(taken==1)
		test1_output[index][1]++;
	else
		test1_output[index][0]++;
}

//间隔step之后，根据output中记录的信息，更新每个状态对应的输出
void test1_change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		//跳转的次数多于不跳转的次数，则预测值应为跳转
		if(test1_output[i][1]>test1_output[i][0])
			test1_predict_out[i]=1;
		else if(test1_output[i][1]<test1_output[i][0])//跳转的次数少于不跳转的次数，则预测值应为不跳转
			test1_predict_out[i]=0;
		//test1_output[i][0]=0;
		//test1_output[i][1]=0;
	}
}

//预测，根据pc得到索引，利用BHT和状态输出对应表进行预测
int two_bit_test1_predict(int64 pc)
{
	//根据pc获取BHT的索引
	int index=get_bit(pc,0,two_bit_test1_param.index_len);
	//索引BHT，根据BHT表项中的状态，找到状态输出表中的预测输出
	return test1_predict_out[two_bit_test1_param.table[index]];
}

//更新BHT中的饱和计数器
void two_bit_test1_update_saturation(int64 pc, int taken)
{
	int index=get_bit(pc,0,two_bit_test1_param.index_len);
	if(taken==1)
	{
		if(two_bit_test1_param.table[index]<3)
			two_bit_test1_param.table[index]++;
	}
	else
	{
		if((int)two_bit_test1_param.table[index]>0)
			two_bit_test1_param.table[index]--;
	}
}

//两位分支预测器的测试2的主函数
void two_bit_test1_predictor(int index_len,int init_state)
{
	init_two_bit_test1_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		two_bit_test1_param.inst_num++;
		if(two_bit_test1_param.inst_num%two_bit_test1_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",two_bit_test1_param.play_step,1-1.0*two_bit_test1_param.mid_miss/two_bit_test1_param.play_step);
			//fprintf(log_file,"%d,%lf\n",two_bit_test1_param.inst_num,1-1.0*two_bit_test1_param.mid_miss/two_bit_test1_param.play_step);
			
			//更新state out table
			test1_change_predict_out();
			two_bit_test1_param.mid_miss=0;
		}
		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//根据分支的实际结果，更新output表
		test1_record_output(pc,taken);

		//predict
		int p=two_bit_test1_predict(pc);
		//judgemet
		if(p!=taken)
		{
			two_bit_test1_param.mid_miss++;
			two_bit_test1_param.miss_num++;
		}
		//update
		two_bit_test1_update_saturation(pc,taken);
	}

	b_result.miss_inst=two_bit_test1_param.miss_num;
	b_result.total_inst=two_bit_test1_param.inst_num;
	b_result.acc=1.0-1.0*two_bit_test1_param.miss_num/two_bit_test1_param.inst_num;

	printf("two bit test1 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(two_bit_test1_param.table);
}