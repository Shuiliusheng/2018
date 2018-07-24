#include"branch_predictor.h"
/*
������λ��֧Ԥ����
BHT�ı���Ϊ��λ���ͼ�������Ԥ��ʱʹ�ø�λ����Ԥ��
��ѡ������BHT���������BHT�ĳ�ʼ״̬
*/


//������λԤ�������ڲ�����
Predictor_Param two_bit_param;

//��ʼ����λԤ�����Ļ�����Ϣ
void init_two_bit_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	two_bit_param.index_len=index_length;
	two_bit_param.inst_num=0;
	two_bit_param.miss_num=0;
	two_bit_param.mid_miss=0;
	two_bit_param.play_step=b_result.step;
	two_bit_param.table=(unsigned char*)malloc(entry_num);
	for(int i=0;i<entry_num;i++)
		two_bit_param.table[i]=init_state;
}

//����PC����Ԥ����
int two_bit_predict(int64 pc)
{
	//��ȡBHT������
	int index=get_bit(pc,0,two_bit_param.index_len);
	//����BHT����ĸ�λ����Ԥ��
	if(two_bit_param.table[index]>=2)
		return 1;
	else
		return 0;
}

//����BHT�����еı��ͼ�������״̬
void two_bit_update_saturation(int64 pc, int taken)
{
	//��ȡBHT������
	int index=get_bit(pc,0,two_bit_param.index_len);
	//�����ۼ���״̬
	if(taken==1)
	{
		if(two_bit_param.table[index]<3)
			two_bit_param.table[index]++;
	}
	else
	{
		if((int)two_bit_param.table[index]>0)
			two_bit_param.table[index]--;
	}
}


//��λ��֧Ԥ����������
void two_bit_predictor(int index_len,int init_state)
{
	init_two_bit_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		two_bit_param.inst_num++;
		if(two_bit_param.inst_num%two_bit_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",two_bit_param.play_step,1-1.0*two_bit_param.mid_miss/two_bit_param.play_step);
			fprintf(log_file,"%d,%lf\n",two_bit_param.inst_num,1-1.0*two_bit_param.mid_miss/two_bit_param.play_step);
			two_bit_param.mid_miss=0;
		}

		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//predict
		int p=two_bit_predict(pc);
		//judgemet
		if(p!=taken)
		{
			two_bit_param.mid_miss++;
			two_bit_param.miss_num++;
		}
		//update
		two_bit_update_saturation(pc,taken);
	}

	b_result.miss_inst=two_bit_param.miss_num;
	b_result.total_inst=two_bit_param.inst_num;
	b_result.acc=1.0-1.0*two_bit_param.miss_num/two_bit_param.inst_num;

	printf("two bit predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(two_bit_param.table);
}