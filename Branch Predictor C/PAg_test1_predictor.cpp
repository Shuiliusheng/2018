#include"branch_predictor.h"
/*

PAg_test1: ������PAgһ�£���������������output��predictout
*/

//PAg_test1 predictor�Ĳ���
Predictor_Param PAg_test1_param;

//�ֲ���BHT
extern int64 *BHT;
extern int bht_index_len;

//PAg_test1_output ���ڼ�¼ÿ��״̬�£�������֧����ת�������Ŀ
int PAg_test1_output[4][2];
//���ڸ���ÿ��״̬Ӧ�е���������״̬�����Ӧ��
char PAg_test1_predict_out[4];


//��ʼ��PAg_test1Ԥ������index_lenָ��BHR��λ��
void init_PAg_test1_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	PAg_test1_param.index_len=index_length;
	PAg_test1_param.inst_num=0;
	PAg_test1_param.miss_num=0;
	PAg_test1_param.mid_miss=0;
	PAg_test1_param.play_step=b_result.step;
	PAg_test1_param.table=(unsigned char*)malloc(entry_num);

	//��ʼ��ÿ��״̬��Ӧ���������ʼΪ��λ���ͼ���������λ����Ԥ��
	PAg_test1_predict_out[0]=0;
	PAg_test1_predict_out[1]=0;
	PAg_test1_predict_out[2]=1;
	PAg_test1_predict_out[3]=1;

	//��ʼ��PAg_test1_outputΪ0
	for(int i=0;i<8;i++)
		PAg_test1_output[i/2][i%2]=0;

	for(int i=0;i<entry_num;i++)
		PAg_test1_param.table[i]=init_state;

	//��ʼ��BHT����������Ϊ0
	entry_num=pow_int2(bht_index_len);
	BHT=(int64*)malloc(sizeof(int64)*entry_num);
	for(int i=0;i<entry_num;i++)
		BHT[i]=0;

}

//ÿ���ڵõ�ʵ�ʷ�֧��ת���֮�󣬽����¼��output��
void PAg_test1_record_output(int64 pc,int taken)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	//����BHT�еı���õ�PHT������
	index=get_bit(BHT[index],0,PAg_test1_param.index_len);

	index=PAg_test1_param.table[index];

	//��������ֵ������PAg_test1_output
	if(taken==1)
		PAg_test1_output[index][1]++;
	else
		PAg_test1_output[index][0]++;
}

//���step֮�󣬸���output�м�¼����Ϣ������ÿ��״̬��Ӧ�����
void PAg_test1_change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ��ת
		if(PAg_test1_output[i][1]>PAg_test1_output[i][0])
			PAg_test1_predict_out[i]=1;
		else if(PAg_test1_output[i][1]<PAg_test1_output[i][0])//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ����ת
			PAg_test1_predict_out[i]=0;
		PAg_test1_output[i][0]=0;
		PAg_test1_output[i][1]=0;
	}
}

//PAg_test1��Ԥ��
int PAg_test1_predict(int64 pc)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	//����BHT�еı���õ�PHT������
	index=get_bit(BHT[index],0,PAg_test1_param.index_len);

	return PAg_test1_predict_out[PAg_test1_param.table[index]];
}


void PAg_test1_update_saturation(int64 pc,int taken)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	//����BHT�еı���õ�PHT������
	index=get_bit(BHT[index],0,PAg_test1_param.index_len);

	//����PHT����
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

//����BHR����λ�Ĵ���
void PAg_test1_update_bhr(int64 pc, int taken)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	BHT[index]=((BHT[index]<<1)+taken);
}

//PAg_test1Ԥ����������
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

			//����state out table
			PAg_test1_change_predict_out();
		}
		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//���ݷ�֧��ʵ�ʽ��������output��
		PAg_test1_record_output(pc,taken);

		//PAg_test1��Ԥ��
		int p=PAg_test1_predict(pc);

		//Ԥ�������ж�
		if(p!=taken)
		{
			PAg_test1_param.mid_miss++;
			PAg_test1_param.miss_num++;
		}
		//����PHT����
		PAg_test1_update_saturation(pc,taken);
		//����BHR
		PAg_test1_update_bhr(pc,taken);
	}

	b_result.miss_inst=PAg_test1_param.miss_num;
	b_result.total_inst=PAg_test1_param.inst_num;
	b_result.acc=1.0-1.0*PAg_test1_param.miss_num/PAg_test1_param.inst_num;

	printf("PAg_test1 predictor (BHT entry:%d, PHT entry:%d)\n",pow_int2(bht_index_length),pow_int2(pht_index_len));
	free(PAg_test1_param.table);
	free(BHT);
}