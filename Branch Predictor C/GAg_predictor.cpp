#include"branch_predictor.h"
/*
GAp:global BHR and global PHT predictor
BHR��¼��ǰk��������֧����ת�������������PHT��
��ѡ������BHR��λ���PHT�����еĳ�ʼ״̬
*/

//GAg predictor�Ĳ���
Predictor_Param GAg_param;

//ȫ�ֵ�BHR
int64 bhr;

//��ʼ��GAgԤ������index_lenָ��BHR��λ��
void init_GAg_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	GAg_param.index_len=index_length;
	GAg_param.inst_num=0;
	GAg_param.miss_num=0;
	GAg_param.mid_miss=0;
	GAg_param.play_step=b_result.step;
	GAg_param.table=(unsigned char*)malloc(entry_num);
	
	//��ʼ��BHRΪ0
	bhr=0;
	for(int i=0;i<entry_num;i++)
		GAg_param.table[i]=init_state;
}

//GAg��Ԥ��
int GAg_predict()
{
	//����BHR�ĵ�index_lenλ��ȡ����ֵ
	int index=get_bit(bhr,0,GAg_param.index_len);
	//����PHT�����е����λ����Ԥ��
	if(GAg_param.table[index]>=2)
		return 1;
	else
		return 0;
}

//����PHT
void GAg_update_saturation(int taken)
{
	//����BHR��ȡPHT������
	int index=get_bit(bhr,0,GAg_param.index_len);
	//����PHT����
	if(taken==1)
	{
		if(GAg_param.table[index]<3)
			GAg_param.table[index]++;
	}
	else
	{
		if((int)GAg_param.table[index]>0)
			GAg_param.table[index]--;
	}
}

//����BHR����λ�Ĵ���
void GAg_update_bhr(int taken)
{
	bhr=((bhr<<1)+taken);
}

//GAgԤ����������
void GAg_predictor(int index_len,int init_state)
{
	init_GAg_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		GAg_param.inst_num++;
		if(GAg_param.inst_num%GAg_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",GAg_param.play_step,1-1.0*GAg_param.mid_miss/GAg_param.play_step);
			fprintf(log_file,"%d,%lf\n",GAg_param.inst_num,1-1.0*GAg_param.mid_miss/GAg_param.play_step);
			GAg_param.mid_miss=0;
		}
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//GAg��Ԥ��
		int p=GAg_predict();

		//Ԥ�������ж�
		if(p!=taken)
		{
			GAg_param.mid_miss++;
			GAg_param.miss_num++;
		}
		//����PHT����
		GAg_update_saturation(taken);
		//����BHR
		GAg_update_bhr(taken);
	}

	b_result.miss_inst=GAg_param.miss_num;
	b_result.total_inst=GAg_param.inst_num;
	b_result.acc=1.0-1.0*GAg_param.miss_num/GAg_param.inst_num;

	printf("GAg predictor (PHT entry:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(GAg_param.table);
}