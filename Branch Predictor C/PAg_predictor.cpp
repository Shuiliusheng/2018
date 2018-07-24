#include"branch_predictor.h"
/*

PAg: pre-address bhr(BHT) and global PHT
ÿ��������ָ֧����Լ���BHR��ʵ�ʲ��ǣ�����ʱBHR�ڲ��洢����Ϣ��Ϊ��ǰ������ָ֧�����ʷ��Ϣ
�ڽ���Ԥ��Ĺ����У����ȸ���pc���ҵ��Լ���bhr��Ȼ�����bhr����PHT���õ�Ԥ����
*/

//PAg predictor�Ĳ���
Predictor_Param PAg_param;

//�ֲ���BHT
int64 *BHT;
int bht_index_len;

//��ʼ��PAgԤ������index_lenָ��BHR��λ��
void init_PAg_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	PAg_param.index_len=index_length;
	PAg_param.inst_num=0;
	PAg_param.miss_num=0;
	PAg_param.mid_miss=0;
	PAg_param.play_step=b_result.step;
	PAg_param.table=(unsigned char*)malloc(entry_num);

	for(int i=0;i<entry_num;i++)
		PAg_param.table[i]=init_state;

	//��ʼ��BHT����������Ϊ0
	entry_num=pow_int2(bht_index_len);
	BHT=(int64*)malloc(sizeof(int64)*entry_num);
	for(int i=0;i<entry_num;i++)
		BHT[i]=0;

}

//PAg��Ԥ��
int PAg_predict(int64 pc)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	//����BHT�еı���õ�PHT������
	index=get_bit(BHT[index],0,PAg_param.index_len);

	if(PAg_param.table[index]>=2)
		return 1;
	else
		return 0;
}

void PAg_update_saturation(int64 pc,int taken)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	//����BHT�еı���õ�PHT������
	index=get_bit(BHT[index],0,PAg_param.index_len);

	//����PHT����
	if(taken==1)
	{
		if(PAg_param.table[index]<3)
			PAg_param.table[index]++;
	}
	else
	{
		if((int)PAg_param.table[index]>0)
			PAg_param.table[index]--;
	}
}

//����BHR����λ�Ĵ���
void PAg_update_bhr(int64 pc, int taken)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	BHT[index]=((BHT[index]<<1)+taken);
}

//PAgԤ����������
void PAg_predictor(int pht_index_len,int init_state,int bht_index_length)
{
	bht_index_len=bht_index_length;
	init_PAg_predictor(pht_index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		PAg_param.inst_num++;
		if(PAg_param.inst_num%PAg_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",PAg_param.play_step,1-1.0*PAg_param.mid_miss/PAg_param.play_step);
			fprintf(log_file,"%d,%lf\n",PAg_param.inst_num,1-1.0*PAg_param.mid_miss/PAg_param.play_step);
			PAg_param.mid_miss=0;
		}
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//PAg��Ԥ��
		int p=PAg_predict(pc);

		//Ԥ�������ж�
		if(p!=taken)
		{
			PAg_param.mid_miss++;
			PAg_param.miss_num++;
		}
		//����PHT����
		PAg_update_saturation(pc,taken);
		//����BHR
		PAg_update_bhr(pc,taken);
	}

	b_result.miss_inst=PAg_param.miss_num;
	b_result.total_inst=PAg_param.inst_num;
	b_result.acc=1.0-1.0*PAg_param.miss_num/PAg_param.inst_num;

	printf("PAg predictor (BHT entry:%d, PHT entry:%d)\n",pow_int2(bht_index_length),pow_int2(pht_index_len));
	free(PAg_param.table);
	free(BHT);
}