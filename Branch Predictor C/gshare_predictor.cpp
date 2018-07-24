#include"branch_predictor.h"

/*
	gshare predictor:������GAgһ�£�
	��������gshareʹ�õ�������Ϊpc��BHR�����֮��Ľ��
	����ʹ�õ�PCλ����BHR��λ��һ��
	��ѡ������PC��BHRʹ�õ�λ���PHT�����еĳ�ʼ״̬
*/

//Ԥ��������
Predictor_Param gshare_param;

//ȫ�ֵ�BHR
int64 gshare_bhr;

//��ʼ��gshareԤ������index_lenָ��BHRλ������ʱʹ�õ�pc��λ��
void init_gshare_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	gshare_param.index_len=index_length;
	gshare_param.inst_num=0;
	gshare_param.miss_num=0;
	gshare_param.mid_miss=0;
	gshare_param.play_step=b_result.step;
	gshare_param.table=(unsigned char*)malloc(entry_num);
	//��ʼ��BHR
	gshare_bhr=0;
	for(int i=0;i<entry_num;i++)
		gshare_param.table[i]=init_state;
}

//ʹ��gshare����Ԥ��
int gshare_predict(int64 pc)
{
	//ʹ��BHR��ȡ����1
	int index=get_bit(gshare_bhr,0,gshare_param.index_len);
	//ʹ��pc��ȡ����2����Ϊ��index_lenλ
	int index1=get_bit(pc,0,gshare_param.index_len);
	//���������������������õ�ʵ�ʵ�����
	index=index1^index;
	//���������ҵ�PHT�������Ԥ��
	if(gshare_param.table[index]>=2)
		return 1;
	else
		return 0;
}

//���±��ͼ�����״̬
void gshare_update_saturation(int64 pc,int taken)
{
	//ʹ��BHR��ȡ����1
	int index=get_bit(gshare_bhr,0,gshare_param.index_len);
	//ʹ��pc��ȡ����2����Ϊ��index_lenλ
	int index1=get_bit(pc,0,gshare_param.index_len);
	//���������������������õ�ʵ�ʵ�����
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

//����BHR����λ�Ĵ���
void gshare_update_gshare_bhr(int taken)
{
	gshare_bhr=((gshare_bhr<<1)+taken);
}

//gshareԤ����������
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
		//��ȡ��һ��ָ��
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

		//����BHR
		gshare_update_gshare_bhr(taken);
	}

	b_result.miss_inst=gshare_param.miss_num;
	b_result.total_inst=gshare_param.inst_num;
	b_result.acc=1.0-1.0*gshare_param.miss_num/gshare_param.inst_num;

	printf("gshare predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(gshare_param.table);
}