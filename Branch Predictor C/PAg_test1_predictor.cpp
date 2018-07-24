#include"branch_predictor.h"
/*

PAg_test1: 基本与PAg一致，区别在于增加了output和predictout
*/

//PAg_test1 predictor的参数
Predictor_Param PAg_test1_param;

//局部的BHT
extern int64 *BHT;
extern int bht_index_len;

//PAg_test1_output 用于记录每个状态下，条件分支的跳转情况的数目
int PAg_test1_output[4][2];
//用于给出每个状态应有的输出情况，状态输出对应表
char PAg_test1_predict_out[4];


//初始化PAg_test1预测器，index_len指明BHR的位宽
void init_PAg_test1_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	PAg_test1_param.index_len=index_length;
	PAg_test1_param.inst_num=0;
	PAg_test1_param.miss_num=0;
	PAg_test1_param.mid_miss=0;
	PAg_test1_param.play_step=b_result.step;
	PAg_test1_param.table=(unsigned char*)malloc(entry_num);

	//初始化每个状态对应的输出，初始为两位饱和计数器，高位进行预测
	PAg_test1_predict_out[0]=0;
	PAg_test1_predict_out[1]=0;
	PAg_test1_predict_out[2]=1;
	PAg_test1_predict_out[3]=1;

	//初始化PAg_test1_output为0
	for(int i=0;i<8;i++)
		PAg_test1_output[i/2][i%2]=0;

	for(int i=0;i<entry_num;i++)
		PAg_test1_param.table[i]=init_state;

	//初始化BHT，表项设置为0
	entry_num=pow_int2(bht_index_len);
	BHT=(int64*)malloc(sizeof(int64)*entry_num);
	for(int i=0;i<entry_num;i++)
		BHT[i]=0;

}

//每次在得到实际分支跳转结果之后，将其记录在output中
void PAg_test1_record_output(int64 pc,int taken)
{
	//根据PC获取BHT的索引
	int index=get_bit(pc,0,bht_index_len);
	//根据BHT中的表项，得到PHT的索引
	index=get_bit(BHT[index],0,PAg_test1_param.index_len);

	index=PAg_test1_param.table[index];

	//利用索引值，更新PAg_test1_output
	if(taken==1)
		PAg_test1_output[index][1]++;
	else
		PAg_test1_output[index][0]++;
}

//间隔step之后，根据output中记录的信息，更新每个状态对应的输出
void PAg_test1_change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		//跳转的次数多于不跳转的次数，则预测值应为跳转
		if(PAg_test1_output[i][1]>PAg_test1_output[i][0])
			PAg_test1_predict_out[i]=1;
		else if(PAg_test1_output[i][1]<PAg_test1_output[i][0])//跳转的次数少于不跳转的次数，则预测值应为不跳转
			PAg_test1_predict_out[i]=0;
		PAg_test1_output[i][0]=0;
		PAg_test1_output[i][1]=0;
	}
}

//PAg_test1的预测
int PAg_test1_predict(int64 pc)
{
	//根据PC获取BHT的索引
	int index=get_bit(pc,0,bht_index_len);
	//根据BHT中的表项，得到PHT的索引
	index=get_bit(BHT[index],0,PAg_test1_param.index_len);

	return PAg_test1_predict_out[PAg_test1_param.table[index]];
}


void PAg_test1_update_saturation(int64 pc,int taken)
{
	//根据PC获取BHT的索引
	int index=get_bit(pc,0,bht_index_len);
	//根据BHT中的表项，得到PHT的索引
	index=get_bit(BHT[index],0,PAg_test1_param.index_len);

	//更新PHT表项
	if(taken==1)
	{
		if(PAg_test1_param.table[index]<3)
			PAg_test1_param.table[index]++;
	}
	else
	{
		if((int)PAg_test1_param.table[index]>0)
			PAg_test1_param.table[index]--;
	}
}

//更新BHR，移位寄存器
void PAg_test1_update_bhr(int64 pc, int taken)
{
	//根据PC获取BHT的索引
	int index=get_bit(pc,0,bht_index_len);
	BHT[index]=((BHT[index]<<1)+taken);
}

//PAg_test1预测器主函数
void PAg_test1_predictor(int pht_index_len,int init_state,int bht_index_length)
{
	bht_index_len=bht_index_length;
	init_PAg_test1_predictor(pht_index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		PAg_test1_param.inst_num++;
		if(PAg_test1_param.inst_num%PAg_test1_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",PAg_test1_param.play_step,1-1.0*PAg_test1_param.mid_miss/PAg_test1_param.play_step);
			//fprintf(log_file,"%d,%lf\n",PAg_test1_param.inst_num,1-1.0*PAg_test1_param.mid_miss/PAg_test1_param.play_step);
			PAg_test1_param.mid_miss=0;

			//更新state out table
			PAg_test1_change_predict_out();
		}
		//读取下一条指令
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//根据分支的实际结果，更新output表
		PAg_test1_record_output(pc,taken);

		//PAg_test1的预测
		int p=PAg_test1_predict(pc);

		//预测结果的判断
		if(p!=taken)
		{
			PAg_test1_param.mid_miss++;
			PAg_test1_param.miss_num++;
		}
		//更新PHT表项
		PAg_test1_update_saturation(pc,taken);
		//更新BHR
		PAg_test1_update_bhr(pc,taken);
	}

	b_result.miss_inst=PAg_test1_param.miss_num;
	b_result.total_inst=PAg_test1_param.inst_num;
	b_result.acc=1.0-1.0*PAg_test1_param.miss_num/PAg_test1_param.inst_num;

	printf("PAg_test1 predictor (BHT entry:%d, PHT entry:%d)\n",pow_int2(bht_index_length),pow_int2(pht_index_len));
	free(PAg_test1_param.table);
	free(BHT);
}