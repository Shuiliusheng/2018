#include"branch_predictor.h"
/*
	GAg test1��������GAgԤ����һ��
	           ��������ʹ����output���״̬�����Ӧ����̬�ĸ���PHT�����״̬��
	��ѡ������BHR��λ���PHT�����еĳ�ʼ״̬
*/

//GAgԤ��������
Predictor_Param GAg_test1_param;

//BHR
int64 test1_bhr;

//����ͳ��ÿ��״̬�Ϸ����ķ�֧����Ϣ
int GAg_test1_output[4][2];
//״̬�����Ӧ��
char GAg_test1_predict_out[4];

//��ʼ��
void init_GAg_test1_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	GAg_test1_param.index_len=index_length;
	GAg_test1_param.inst_num=0;
	GAg_test1_param.miss_num=0;
	GAg_test1_param.mid_miss=0;
	GAg_test1_param.play_step=b_result.step;
	GAg_test1_param.table=(unsigned char*)malloc(entry_num);
	//��ʼ��BHRΪ0
	test1_bhr=0;

	//��ʼ��ÿ��״̬��Ӧ���������ʼΪ��λ���ͼ���������λ����Ԥ��
	GAg_test1_predict_out[0]=0;
	GAg_test1_predict_out[1]=0;
	GAg_test1_predict_out[2]=1;
	GAg_test1_predict_out[3]=1;

	//��ʼ��outputΪ0
	for(int i=0;i<8;i++)
		GAg_test1_output[i/2][i%2]=0;

	for(int i=0;i<entry_num;i++)
		GAg_test1_param.table[i]=init_state;
}

//ÿ���ڵõ�ʵ�ʷ�֧��ת���֮�󣬽����¼��output��
void GAg_test1_record_output(int taken)
{
	//����BHR��ȡPHT������
	int index=get_bit(test1_bhr,0,GAg_test1_param.index_len);
	//��ȡPHT�е�״ֵ̬
	index=GAg_test1_param.table[index];

	if(taken==1)
		GAg_test1_output[index][1]++;
	else
		GAg_test1_output[index][0]++;
}

//���step֮�󣬸���output�м�¼����Ϣ������ÿ��״̬��Ӧ�����
void GAg_test1_change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		if(GAg_test1_output[i][1]>GAg_test1_output[i][0])
			GAg_test1_predict_out[i]=1;
		else if(GAg_test1_output[i][1]<GAg_test1_output[i][0])
			GAg_test1_predict_out[i]=0;
		//����ͳ�Ƶ���Ϣ
		GAg_test1_output[i][0]=0;
		GAg_test1_output[i][1]=0;
	}
}

//Ԥ��
int GAg_test1_predict()
{
	//����BHR��ȡPHT������
	int index=get_bit(test1_bhr,0,GAg_test1_param.index_len);
	//����PHT��������ݺ�״̬�����Ӧ�����Ԥ��
	return GAg_test1_predict_out[GAg_test1_param.table[index]];
}

//����PHT�ı���
void GAg_test1_update_saturation(int taken)
{
	int index=get_bit(test1_bhr,0,GAg_test1_param.index_len);
	if(taken==1)
	{
		if(GAg_test1_param.table[index]<3)
			GAg_test1_param.table[index]++;
	}
	else
	{
		if((int)GAg_test1_param.table[index]>0)
			GAg_test1_param.table[index]--;
	}
}

//����BHR
void GAg_test1_update_test1_bhr(int taken)
{
	test1_bhr=((test1_bhr<<1)+taken);
}

//GAg test1 ������
void GAg_test1_predictor(int index_len,int init_state)
{
	init_GAg_test1_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		GAg_test1_param.inst_num++;
		if(GAg_test1_param.inst_num%GAg_test1_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",GAg_test1_param.play_step,1-1.0*GAg_test1_param.mid_miss/GAg_test1_param.play_step);
			//fprintf(log_file,"%d,%lf\n",GAg_test1_param.inst_num,1-1.0*GAg_test1_param.mid_miss/GAg_test1_param.play_step);
			GAg_test1_param.mid_miss=0;
			
			//����state out table
			GAg_test1_change_predict_out();
		}
		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//���ݷ�֧��ʵ�ʽ��������output��
		GAg_test1_record_output(taken);

		//predict
		int p=GAg_test1_predict();
		//judgemet
		if(p!=taken)
		{
			GAg_test1_param.mid_miss++;
			GAg_test1_param.miss_num++;
		}
		//update
		GAg_test1_update_saturation(taken);
		//����BHR
		GAg_test1_update_test1_bhr(taken);
	}

	b_result.miss_inst=GAg_test1_param.miss_num;
	b_result.total_inst=GAg_test1_param.inst_num;
	b_result.acc=1.0-1.0*GAg_test1_param.miss_num/GAg_test1_param.inst_num;

	printf("GAg_test1 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(GAg_test1_param.table);
}