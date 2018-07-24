#include"branch_predictor.h"
/*
	gshare predictor test1:基本与gshare test1一致，
	区别在于使用了具有多个表项output表和状态输出对应表
	可选参数：PC和BHR使用的位宽、PHT表项中的初始状态和output的表项个数
*/

//预测器参数
Predictor_Param gshare_test2_param;

//全局BHR
int64 gshare_test2_bhr;
//具有多个表项的output，最多可设计8192个表项
int gshare_test2_output[8192][4][2];
//具有多个表项的predictout表
char gshare_test2_predict_out[8192][4];
//索引output和predictout的表项位宽
int gshare_test2_out_index_bits;

//初始化
void init_gshare_test2_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	gshare_test2_param.index_len=index_length;
	gshare_test2_param.inst_num=0;
	gshare_test2_param.miss_num=0;
	gshare_test2_param.mid_miss=0;
	gshare_test2_param.play_step=b_result.step;
	gshare_test2_param.table=(unsigned char*)malloc(entry_num);
	
	//更新BHR
	gshare_test2_bhr=0;

	//遍历所有表项
	int out_entry_num=pow_int2(gshare_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
	{
		//初始化predictout为饱和状态器最高位预测的状态
		gshare_test2_predict_out[n][0]=0;
		gshare_test2_predict_out[n][1]=0;
		gshare_test2_predict_out[n][2]=1;
		gshare_test2_predict_out[n][3]=1;
		//初始化output为0
		for(int i=0;i<8;i++)
			gshare_test2_output[n][i/2][i%2]=0;
	}

	for(int i=0;i<entry_num;i++)
		gshare_test2_param.table[i]=init_state;
}

//每次在得到实际分支跳转结果之后，将其记录在output中
void gshare_test2_record_output(int64 pc,int taken)
{
	//根据BHR，获取索引1
	int index=get_bit(gshare_test2_bhr,0,gshare_test2_param.index_len);
	//根据pc，获取索引2
	int index1=get_bit(pc,0,gshare_test2_param.index_len);
	//两个索引进行异或操作，得到最终的索引
	index=index1^index;

	//根据PHT的索引，获取output的第一维索引
	index1=get_bit(index,0,gshare_test2_out_index_bits);

	//根据PHT表项中记录的状态值，获取output的第二维索引
	index=gshare_test2_param.table[index];
	//根据状态值更新output
	if(taken==1)
		gshare_test2_output[index1][index][1]++;
	else
		gshare_test2_output[index1][index][0]++;
}

//间隔step之后，根据output中记录的信息，更新每个状态对应的输出
void gshare_test2_change_predict_out()
{
	//遍历所有的predictout表项
	int out_entry_num=pow_int2(gshare_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
		for(int i=0;i<4;i++)
		{
			//跳转的次数多于不跳转的次数，则预测值应为跳转
			if(gshare_test2_output[n][i][1]>gshare_test2_output[n][i][0])
				gshare_test2_predict_out[n][i]=1;
			else if(gshare_test2_output[n][i][1]<gshare_test2_output[n][i][0])
				gshare_test2_predict_out[n][i]=0;
			//清空之前记录的信息
			gshare_test2_output[n][i][0]=0;
			gshare_test2_output[n][i][1]=0;
		}
}

//根据pc，BHR，索引PHT和predictout,进行预测
int gshare_test2_predict(int64 pc)
{
	//使用BHR获取索引1
	int index=get_bit(gshare_test2_bhr,0,gshare_test2_param.index_len);
	//使用pc获取索引2，均为低index_len位
	int index1=get_bit(pc,0,gshare_test2_param.index_len);
	//两个索引进行异或操作，得到实际的索引
	index=index1^index;
	//根据PHT的索引，获取predictout的第一维索引
	index1=get_bit(index,0,gshare_test2_out_index_bits);
	//根据PHT的索引和predictout的索引，索引PHT和predictout进行预测
	return gshare_test2_predict_out[index1][gshare_test2_param.table[index]];
}

//更新PHT的表项
void gshare_test2_update_saturation(int64 pc,int taken)
{
	int index=get_bit(gshare_test2_bhr,0,gshare_test2_param.index_len);
	int index1=get_bit(pc,0,gshare_test2_param.index_len);
	index=index1^index;

	if(taken==1)
	{
		if(gshare_test2_param.table[index]<3)
			gshare_test2_param.table[index]++;
	}
	else
	{
		if((int)gshare_test2_param.table[index]>0)
			gshare_test2_param.table[index]--;
	}
}

//更新BHR，移位寄存器
void gshare_test2_update_gshare_test2_bhr(int taken)
{
	gshare_test2_bhr=((gshare_test2_bhr<<1)+taken);
}

//gshare test2预测器主函数
void gshare_test2_predictor(int index_len,int init_state,int out_index_len)
{
	gshare_test2_out_index_bits=out_index_len;
	init_gshare_test2_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		gshare_test2_param.inst_num++;
		if(gshare_test2_param.inst_num%gshare_test2_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",gshare_test2_param.play_step,1-1.0*gshare_test2_param.mid_miss/gshare_test2_param.play_step);
			fprintf(log_file,"%d,%lf\n",gshare_test2_param.inst_num,1-1.0*gshare_test2_param.mid_miss/gshare_test2_param.play_step);
			gshare_test2_param.mid_miss=0;
			
			//更新state out table
			gshare_test2_change_predict_out();
		}
		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//根据分支的实际结果，更新output表
		gshare_test2_record_output(pc,taken);

		//predict
		int p=gshare_test2_predict(pc);
		//judgemet
		if(p!=taken)
		{
			gshare_test2_param.mid_miss++;
			gshare_test2_param.miss_num++;
		}
		//update
		gshare_test2_update_saturation(pc,taken);

		//更新BHR
		gshare_test2_update_gshare_test2_bhr(taken);
	}

	b_result.miss_inst=gshare_test2_param.miss_num;
	b_result.total_inst=gshare_test2_param.inst_num;
	b_result.acc=1.0-1.0*gshare_test2_param.miss_num/gshare_test2_param.inst_num;

	printf("gshare_test2 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(gshare_test2_param.table);
}