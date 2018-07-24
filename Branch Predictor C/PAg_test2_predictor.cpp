#include"branch_predictor.h"
/*

PAg_test2: ������PAg test1һ�£���������������output��predictout�ı���
*/

//PAg_test2 predictor�Ĳ���
Predictor_Param PAg_test2_param;

//�ֲ���BHT
extern int64 *BHT;
extern int bht_index_len;

//PAg_test2_output ���ڼ�¼ÿ��״̬�£�������֧����ת�������Ŀ
int PAg_test2_output[8192][4][2];
//���ڸ���ÿ��״̬Ӧ�е���������״̬�����Ӧ��
char PAg_test2_predict_out[8192][4];
//����output��predictout�ı���λ��
int PAg_test2_out_index_bits;

//��ʼ��PAg_test2Ԥ������index_lenָ��BHR��λ��
void init_PAg_test2_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	PAg_test2_param.index_len=index_length;
	PAg_test2_param.inst_num=0;
	PAg_test2_param.miss_num=0;
	PAg_test2_param.mid_miss=0;
	PAg_test2_param.play_step=b_result.step;
	PAg_test2_param.table=(unsigned char*)malloc(entry_num);

	//�������б���
	int out_entry_num=pow_int2(PAg_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
	{
		//��ʼ��predictoutΪ����״̬�����λԤ���״̬
		PAg_test2_predict_out[n][0]=0;
		PAg_test2_predict_out[n][1]=0;
		PAg_test2_predict_out[n][2]=1;
		PAg_test2_predict_out[n][3]=1;
		//��ʼ��outputΪ0
		for(int i=0;i<8;i++)
			PAg_test2_output[n][i/2][i%2]=0;
	}

	for(int i=0;i<entry_num;i++)
		PAg_test2_param.table[i]=init_state;

	//��ʼ��BHT����������Ϊ0
	entry_num=pow_int2(bht_index_len);
	BHT=(int64*)malloc(sizeof(int64)*entry_num);
	for(int i=0;i<entry_num;i++)
		BHT[i]=0;

}

//ÿ���ڵõ�ʵ�ʷ�֧��ת���֮�󣬽����¼��output��
void PAg_test2_record_output(int64 pc,int taken)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	//����BHT�еı���õ�PHT������
	index=get_bit(BHT[index],0,PAg_test2_param.index_len);

	//PHT��Ӧoutput�����Ҳ���ΪӦ����BHT��Ӧoutput
	int index1=get_bit(index,0,PAg_test2_out_index_bits);

	int state=PAg_test2_param.table[index];

	//��������ֵ������PAg_test2_output
	if(taken==1)
		PAg_test2_output[index1][state][1]++;
	else
		PAg_test2_output[index1][state][0]++;
}

//���step֮�󣬸���output�м�¼����Ϣ������ÿ��״̬��Ӧ�����
void PAg_test2_change_predict_out()
{
	//�������е�predictout����
	int out_entry_num=pow_int2(PAg_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
		for(int i=0;i<4;i++)
		{
			//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ��ת
			if(PAg_test2_output[n][i][1]>PAg_test2_output[n][i][0])
				PAg_test2_predict_out[n][i]=1;
			else if(PAg_test2_output[n][i][1]<PAg_test2_output[n][i][0])
				PAg_test2_predict_out[n][i]=0;
			//���֮ǰ��¼����Ϣ
			PAg_test2_output[n][i][0]=0;
			PAg_test2_output[n][i][1]=0;
		}
}

//PAg_test2��Ԥ��
int PAg_test2_predict(int64 pc)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	//����BHT�еı���õ�PHT������
	index=get_bit(BHT[index],0,PAg_test2_param.index_len);

	int index1=get_bit(index,0,PAg_test2_out_index_bits);

	return PAg_test2_predict_out[index1][PAg_test2_param.table[index]];
}


void PAg_test2_update_saturation(int64 pc,int taken)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	//����BHT�еı���õ�PHT������
	index=get_bit(BHT[index],0,PAg_test2_param.index_len);

	//����PHT����
	if(taken==1)
	{
		if(PAg_test2_param.table[index]<3)
			PAg_test2_param.table[index]++;
	}
	else
	{
		if((int)PAg_test2_param.table[index]>0)
			PAg_test2_param.table[index]--;
	}
}

//����BHR����λ�Ĵ���
void PAg_test2_update_bhr(int64 pc, int taken)
{
	//����PC��ȡBHT������
	int index=get_bit(pc,0,bht_index_len);
	BHT[index]=((BHT[index]<<1)+taken);
}

//PAg_test2Ԥ����������
void PAg_test2_predictor(int pht_index_len,int init_state,int bht_index_length,int out_index_len)
{

	//����out_index_bits
	PAg_test2_out_index_bits=out_index_len;

	bht_index_len=bht_index_length;
	init_PAg_test2_predictor(pht_index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		PAg_test2_param.inst_num++;
		if(PAg_test2_param.inst_num%PAg_test2_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",PAg_test2_param.play_step,1-1.0*PAg_test2_param.mid_miss/PAg_test2_param.play_step);
			fprintf(log_file,"%d,%lf\n",PAg_test2_param.inst_num,1-1.0*PAg_test2_param.mid_miss/PAg_test2_param.play_step);
			PAg_test2_param.mid_miss=0;

			//����state out table
			PAg_test2_change_predict_out();
		}
		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//���ݷ�֧��ʵ�ʽ��������output��
		PAg_test2_record_output(pc,taken);

		//PAg_test2��Ԥ��
		int p=PAg_test2_predict(pc);

		//Ԥ�������ж�
		if(p!=taken)
		{
			PAg_test2_param.mid_miss++;
			PAg_test2_param.miss_num++;
		}
		//����PHT����
		PAg_test2_update_saturation(pc,taken);
		//����BHR
		PAg_test2_update_bhr(pc,taken);
	}

	b_result.miss_inst=PAg_test2_param.miss_num;
	b_result.total_inst=PAg_test2_param.inst_num;
	b_result.acc=1.0-1.0*PAg_test2_param.miss_num/PAg_test2_param.inst_num;

	printf("PAg_test2 predictor (BHT entry:%d, PHT entry:%d)\n",pow_int2(bht_index_length),pow_int2(pht_index_len));
	free(PAg_test2_param.table);
	free(BHT);
}