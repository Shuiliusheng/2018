#include"branch_predictor.h"
/*
	GAg test2：基本与GAg test1预测器一致
	           区别在于使用了具有多个表项output表和状态输出对应表
	可选参数：BHR的位宽、PHT表项中的初始状态和output的表项个数
*/

//预测器参数
Predictor_Param GAg_test2_param;

//BHR
int64 test2_bhr;

//具有多个表项的output，最多可设计8192个表项
int GAg_test2_output[8192][4][2];
//具有多个表项的predictout表
char GAg_test2_predict_out[8192][4];
//索引output和predictout的表项位宽
int GAg_test2_out_index_bits;

//初始化
void init_GAg_test2_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	GAg_test2_param.index_len=index_length;
	GAg_test2_param.inst_num=0;
	GAg_test2_param.miss_num=0;
	GAg_test2_param.mid_miss=0;
	GAg_test2_param.play_step=b_result.step;
	GAg_test2_param.table=(unsigned char*)malloc(entry_num);

	//更新BHR
	test2_bhr=0;

	//遍历所有表项
	int out_entry_num=pow_int2(GAg_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
	{
		//初始化predictout为饱和状态器最高位预测的状态
		GAg_test2_predict_out[n][0]=0;
		GAg_test2_predict_out[n][1]=0;
		GAg_test2_predict_out[n][2]=1;
		GAg_test2_predict_out[n][3]=1;
		//初始化output为0
		for(int i=0;i<8;i++)
			GAg_test2_output[n][i/2][i%2]=0;
	}

	for(int i=0;i<entry_num;i++)
		GAg_test2_param.table[i]=init_state;
}

//每次在得到实际分支跳转结果之后，将其记录在output中
void GAg_test2_record_output(int taken)
{
	//根据BHR，获取PHT的索引
	int index=get_bit(test2_bhr,0,GAg_test2_param.index_len);
	//根据PHT的索引，获取output的索引
	int index1=get_bit(index,0,GAg_test2_out_index_bits);

	//根据PHT的索引，得到PHT表项记录的状态值
	int state=GAg_test2_param.table[index];

	//根据状态值和output的索引，记录分支信息
	if(taken==1)
		GAg_test2_output[index1][state][1]++;
	else
		GAg_test2_output[index1][state][0]++;
}

//间隔step之后，根据output中记录的信息，更新每个状态对应的输出
void GAg_test2_change_predict_out()
{
	//遍历所有的predictout表项
	int out_entry_num=pow_int2(GAg_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
		for(int i=0;i<4;i++)
		{
			//跳转的次数多于不跳转的次数，则预测值应为跳转
			if(GAg_test2_output[n][i][1]>GAg_test2_output[n][i][0])
				GAg_test2_predict_out[n][i]=1;
			else if(GAg_test2_output[n][i][1]<GAg_test2_output[n][i][0])
				GAg_test2_predict_out[n][i]=0;
			//清空之前记录的信息
			GAg_test2_output[n][i][0]=0;
			GAg_test2_output[n][i][1]=0;
		}
}

//根据pc，索引PHT和predictout,进行预测
int GAg_test2_predict()
{
	//根据BHR，获取PHT的索引
	int index=get_bit(test2_bhr,0,GAg_test2_param.index_len);
	//根据PHT的索引，获取predictout的索引
	int index1=get_bit(index,0,GAg_test2_out_index_bits);
	//根据PHT表项中的内容和predictout的索引，查找predictout表进行预测
	return GAg_test2_predict_out[index1][GAg_test2_param.table[index]];
}

//更新饱和计数器的状态
void GAg_test2_update_saturation(int taken)
{
	int index=get_bit(test2_bhr,0,GAg_test2_param.index_len);
	if(taken==1)
	{
		if(GAg_test2_param.table[index]<3)
			GAg_test2_param.table[index]++;
	}
	else
	{
		if((int)GAg_test2_param.table[index]>0)
			GAg_test2_param.table[index]--;
	}
}

//更新BHR
void GAg_test2_update_test2_bhr(int taken)
{
	test2_bhr=((test2_bhr<<1)+taken);
}


//GAg test2 主函数
void GAg_test2_predictor(int index_len,int init_state,int out_index_len)
{
	//设置out_index_bits
	GAg_test2_out_index_bits=out_index_len;
	
	init_GAg_test2_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		GAg_test2_param.inst_num++;
		if(GAg_test2_param.inst_num%GAg_test2_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",GAg_test2_param.play_step,1-1.0*GAg_test2_param.mid_miss/GAg_test2_param.play_step);
			fprintf(log_file,"%d,%lf\n",GAg_test2_param.inst_num,1-1.0*GAg_test2_param.mid_miss/GAg_test2_param.play_step);
			GAg_test2_param.mid_miss=0;
			//更新state out table
			GAg_test2_change_predict_out();
		}
		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//根据分支的实际结果，更新output表
		GAg_test2_record_output(taken);

		//predict
		int p=GAg_test2_predict();
		//judgemet
		if(p!=taken)
		{
			GAg_test2_param.mid_miss++;
			GAg_test2_param.miss_num++;
		}
		//update
		GAg_test2_update_saturation(taken);
		//更新BHR
		GAg_test2_update_test2_bhr(taken);
	}

	b_result.miss_inst=GAg_test2_param.miss_num;
	b_result.total_inst=GAg_test2_param.inst_num;
	b_result.acc=1.0-1.0*GAg_test2_param.miss_num/GAg_test2_param.inst_num;

	printf("GAg_test2 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(GAg_test2_param.table);
}