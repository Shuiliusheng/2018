#include"branch_predictor.h"
/*
��λ������֧Ԥ����������3
�������ݣ����������1һ�£����stepҲ������output��ͳ����Ϣ
          ���𣺲���1�н���һ��output�����еķ�ָ֧�����ͬһ��output
		        ����3�а������output������BHT��������ͬ�����²�ͬ��output
				��ʱoutput������������BHT�ĵ�����λ��ɣ�output�ĸ���������BHT����ĸ���
*/

//Ԥ��������
Predictor_Param two_bit_test2_param;

//output ���ڼ�¼ÿ��״̬�£�������֧����ת�������Ŀ
int test2_output[8192][4][2];

//���ڸ���ÿ��״̬Ӧ�е�������
char test2_predict_out[8192][4];

//��������test2_output��test2_predict_out������
int out_index_bits;

void init_two_bit_test2_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	two_bit_test2_param.index_len=index_length;
	two_bit_test2_param.inst_num=0;
	two_bit_test2_param.miss_num=0;
	two_bit_test2_param.mid_miss=0;
	two_bit_test2_param.play_step=b_result.step;
	two_bit_test2_param.table=(unsigned char*)malloc(entry_num);

	
	int out_entry_num=pow_int2(out_index_bits);
	//�������б���
	for(int n=0;n<out_entry_num;n++)
	{
		//��ʼ��predictoutΪ����״̬�����λԤ���״̬
		test2_predict_out[n][0]=0;
		test2_predict_out[n][1]=0;
		test2_predict_out[n][2]=1;
		test2_predict_out[n][3]=1;

		//��ʼ��outputΪ0
		for(int i=0;i<8;i++)
			test2_output[n][i/2][i%2]=0;
	}
	for(int i=0;i<entry_num;i++)
		two_bit_test2_param.table[i]=init_state;
}

//ÿ���ڵõ�ʵ�ʷ�֧��ת���֮�󣬽����¼��output��
void test2_record_output(int64 pc,int taken)
{
	//����pc��ȡBHT������
	int index=get_bit(pc,0,two_bit_test2_param.index_len);
	//����BHT��������ȡ��out_index_bitsλ��������output��
	int index1=get_bit(index,0,out_index_bits);
	//����BHT�ı����ȡ��ǰ״̬����������output�ĵڶ�ά
	index=two_bit_test2_param.table[index];

	//����output��
	if(taken==1)
		test2_output[index1][index][1]++;
	else
		test2_output[index1][index][0]++;
}

//���step֮�󣬸���output�м�¼����Ϣ������ÿ��״̬��Ӧ�����
void test2_change_predict_out()
{
	//�������е�predictout����
	int out_entry_num=pow_int2(out_index_bits);
	for(int n=0;n<out_entry_num;n++)
		for(int i=0;i<4;i++)
		{
			//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ��ת
			if(test2_output[n][i][1]>test2_output[n][i][0])
				test2_predict_out[n][i]=1;
			else if(test2_output[n][i][1]<test2_output[n][i][0])//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ����ת
				test2_predict_out[n][i]=0;
			//���֮ǰ��¼����Ϣ
			test2_output[n][i][0]=0;
			test2_output[n][i][1]=0;
		}
}

//����pc������BHT��predictout,����Ԥ��
int two_bit_test2_predict(int64 pc)
{
	//����pc���õ�BHT������
	int index=get_bit(pc,0,two_bit_test2_param.index_len);
	//����BHT���������õ�predictout������
	int index1=get_bit(index,0,out_index_bits);
	//���ݱ������Ԥ��
	return test2_predict_out[index1][two_bit_test2_param.table[index]];
}

//���±��ͼ�������״̬
void two_bit_test2_update_saturation(int64 pc, int taken)
{
	int index=get_bit(pc,0,two_bit_test2_param.index_len);
	if(taken==1)
	{
		if(two_bit_test2_param.table[index]<3)
			two_bit_test2_param.table[index]++;
	}
	else
	{
		if((int)two_bit_test2_param.table[index]>0)
			two_bit_test2_param.table[index]--;
	}
}

//��λ��֧Ԥ�����Ĳ���2��������
void two_bit_test2_predictor(int index_len,int init_state,int out_index_len)
{
	out_index_bits=out_index_len;
	init_two_bit_test2_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		two_bit_test2_param.inst_num++;
		//���step֮�󣬸���״̬�����Ӧ��
		if(two_bit_test2_param.inst_num%two_bit_test2_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",two_bit_test2_param.play_step,1-1.0*two_bit_test2_param.mid_miss/two_bit_test2_param.play_step);
			fprintf(log_file,"%d,%lf\n",two_bit_test2_param.inst_num,1-1.0*two_bit_test2_param.mid_miss/two_bit_test2_param.play_step);
			
			//����state out table
			test2_change_predict_out();
			two_bit_test2_param.mid_miss=0;
		}
		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//���ݷ�֧��ʵ�ʽ��������output��
		test2_record_output(pc,taken);

		//predict
		int p=two_bit_test2_predict(pc);
		//judgemet
		if(p!=taken)
		{
			two_bit_test2_param.mid_miss++;
			two_bit_test2_param.miss_num++;
		}
		//update
		two_bit_test2_update_saturation(pc,taken);
	}

	b_result.miss_inst=two_bit_test2_param.miss_num;
	b_result.total_inst=two_bit_test2_param.inst_num;
	b_result.acc=1.0-1.0*two_bit_test2_param.miss_num/two_bit_test2_param.inst_num;

	printf("two bit test2 predictor (BHT entry:%d, Predict_out entry:%d)\n",pow_int2(index_len),pow_int2(out_index_len));
	free(two_bit_test2_param.table);
}