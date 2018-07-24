#include"branch_predictor.h"
/*
	GAg test2��������GAg test1Ԥ����һ��
	           ��������ʹ���˾��ж������output���״̬�����Ӧ��
	��ѡ������BHR��λ��PHT�����еĳ�ʼ״̬��output�ı������
*/

//Ԥ��������
Predictor_Param GAg_test2_param;

//BHR
int64 test2_bhr;

//���ж�������output���������8192������
int GAg_test2_output[8192][4][2];
//���ж�������predictout��
char GAg_test2_predict_out[8192][4];
//����output��predictout�ı���λ��
int GAg_test2_out_index_bits;

//��ʼ��
void init_GAg_test2_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	GAg_test2_param.index_len=index_length;
	GAg_test2_param.inst_num=0;
	GAg_test2_param.miss_num=0;
	GAg_test2_param.mid_miss=0;
	GAg_test2_param.play_step=b_result.step;
	GAg_test2_param.table=(unsigned char*)malloc(entry_num);

	//����BHR
	test2_bhr=0;

	//�������б���
	int out_entry_num=pow_int2(GAg_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
	{
		//��ʼ��predictoutΪ����״̬�����λԤ���״̬
		GAg_test2_predict_out[n][0]=0;
		GAg_test2_predict_out[n][1]=0;
		GAg_test2_predict_out[n][2]=1;
		GAg_test2_predict_out[n][3]=1;
		//��ʼ��outputΪ0
		for(int i=0;i<8;i++)
			GAg_test2_output[n][i/2][i%2]=0;
	}

	for(int i=0;i<entry_num;i++)
		GAg_test2_param.table[i]=init_state;
}

//ÿ���ڵõ�ʵ�ʷ�֧��ת���֮�󣬽����¼��output��
void GAg_test2_record_output(int taken)
{
	//����BHR����ȡPHT������
	int index=get_bit(test2_bhr,0,GAg_test2_param.index_len);
	//����PHT����������ȡoutput������
	int index1=get_bit(index,0,GAg_test2_out_index_bits);

	//����PHT���������õ�PHT�����¼��״ֵ̬
	int state=GAg_test2_param.table[index];

	//����״ֵ̬��output����������¼��֧��Ϣ
	if(taken==1)
		GAg_test2_output[index1][state][1]++;
	else
		GAg_test2_output[index1][state][0]++;
}

//���step֮�󣬸���output�м�¼����Ϣ������ÿ��״̬��Ӧ�����
void GAg_test2_change_predict_out()
{
	//�������е�predictout����
	int out_entry_num=pow_int2(GAg_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
		for(int i=0;i<4;i++)
		{
			//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ��ת
			if(GAg_test2_output[n][i][1]>GAg_test2_output[n][i][0])
				GAg_test2_predict_out[n][i]=1;
			else if(GAg_test2_output[n][i][1]<GAg_test2_output[n][i][0])
				GAg_test2_predict_out[n][i]=0;
			//���֮ǰ��¼����Ϣ
			GAg_test2_output[n][i][0]=0;
			GAg_test2_output[n][i][1]=0;
		}
}

//����pc������PHT��predictout,����Ԥ��
int GAg_test2_predict()
{
	//����BHR����ȡPHT������
	int index=get_bit(test2_bhr,0,GAg_test2_param.index_len);
	//����PHT����������ȡpredictout������
	int index1=get_bit(index,0,GAg_test2_out_index_bits);
	//����PHT�����е����ݺ�predictout������������predictout�����Ԥ��
	return GAg_test2_predict_out[index1][GAg_test2_param.table[index]];
}

//���±��ͼ�������״̬
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

//����BHR
void GAg_test2_update_test2_bhr(int taken)
{
	test2_bhr=((test2_bhr<<1)+taken);
}


//GAg test2 ������
void GAg_test2_predictor(int index_len,int init_state,int out_index_len)
{
	//����out_index_bits
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
			//����state out table
			GAg_test2_change_predict_out();
		}
		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//���ݷ�֧��ʵ�ʽ��������output��
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
		//����BHR
		GAg_test2_update_test2_bhr(taken);
	}

	b_result.miss_inst=GAg_test2_param.miss_num;
	b_result.total_inst=GAg_test2_param.inst_num;
	b_result.acc=1.0-1.0*GAg_test2_param.miss_num/GAg_test2_param.inst_num;

	printf("GAg_test2 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(GAg_test2_param.table);
}