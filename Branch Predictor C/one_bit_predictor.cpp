#include"branch_predictor.h"
/*
����һλԤ��������ѡ������BHT�ı�������BHT�ĳ�ʼ״̬
*/

//�洢Ԥ�����Ĳ�����BHT������BHT��λ��ָ������Ԥ���������������
Predictor_Param one_bit_param;

//һλԤ�����ĳ�ʼ������ʼBHT������λ��BHT�ĳ�ʼ״̬��������Ϣ
void init_one_bit_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	one_bit_param.index_len=index_length;
	one_bit_param.inst_num=0;
	one_bit_param.miss_num=0;
	one_bit_param.mid_miss=0;
	one_bit_param.play_step=b_result.step;
	one_bit_param.table=(unsigned char*)malloc(entry_num);
	for(int i=0;i<entry_num;i++)
		one_bit_param.table[i]=init_state;
}

//ʹ��һλԤ��������pc��λ����BHT������Ԥ��
int one_bit_predict(int64 pc)
{
	//��ȡ����
	int index=get_bit(pc,0,one_bit_param.index_len);
	//����BHT����Ԥ��ֵ
	return (int)one_bit_param.table[index];
}

//����pc��ʵ�ʵ���ת���������BHT
void one_bit_update(int64 pc, int taken)
{
	//��ȡ����
	int index=get_bit(pc,0,one_bit_param.index_len);
	//����taken����
	one_bit_param.table[index]=(unsigned char)taken;
}

//һλԤ������������
//������index_len  ������λ��
//      init_state BHT�ĳ�ʼ״̬
void one_bit_predictor(int index_len,int init_state)
{
	init_one_bit_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		//ָ������һ
		one_bit_param.inst_num++;
		//���step��ʾһ��step�ڼ�Ľ��
		if(one_bit_param.inst_num%one_bit_param.play_step==0)
		{
			printf("step len:%d, hit rate: %lf\n",one_bit_param.play_step,1-1.0*one_bit_param.mid_miss/one_bit_param.play_step);
			one_bit_param.mid_miss=0;
		}
		//��ȡ��һ������ת��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//predict
		int p=one_bit_predict(pc);
		//judgemet
		if(p!=taken)
		{
			one_bit_param.mid_miss++;
			one_bit_param.miss_num++;
		}
		//update
		one_bit_update(pc,taken);
	}

	//��������ݸ�ȫ�ֵ���Ϣ��¼
	b_result.miss_inst=one_bit_param.miss_num;
	b_result.total_inst=one_bit_param.inst_num;
	b_result.acc=1.0-1.0*one_bit_param.miss_num/one_bit_param.inst_num;

	//�ͷ�������ڴ�
	printf("one bit predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(one_bit_param.table);
}