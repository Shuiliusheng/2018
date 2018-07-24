#include"branch_predictor.h"
/*
��λ������֧Ԥ����������2
�������ݣ���test1�Ĳ������ݻ���һ��
		  ���𣺼��step����֮��ͳ�Ƶ�output��Ϣ��������
*/

//Ԥ��������
Predictor_Param two_bit_test1_param;

//test1_output ���ڼ�¼ÿ��״̬�£�������֧����ת�������Ŀ
int test1_output[4][2];
//���ڸ���ÿ��״̬Ӧ�е���������״̬�����Ӧ��
char test1_predict_out[4];

void init_two_bit_test1_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	two_bit_test1_param.index_len=index_length;
	two_bit_test1_param.inst_num=0;
	two_bit_test1_param.miss_num=0;
	two_bit_test1_param.mid_miss=0;
	two_bit_test1_param.play_step=b_result.step;
	two_bit_test1_param.table=(unsigned char*)malloc(entry_num);
	
	//��ʼ��ÿ��״̬��Ӧ���������ʼΪ��λ���ͼ���������λ����Ԥ��
	test1_predict_out[0]=0;
	test1_predict_out[1]=0;
	test1_predict_out[2]=1;
	test1_predict_out[3]=1;

	//��ʼ��test1_outputΪ0
	for(int i=0;i<8;i++)
		test1_output[i/2][i%2]=0;
	for(int i=0;i<entry_num;i++)
		two_bit_test1_param.table[i]=init_state;
}

//ÿ���ڵõ�ʵ�ʷ�֧��ת���֮�󣬽����¼��output��
void test1_record_output(int64 pc,int taken)
{
	//����pc��ȡBHT�����е�״ֵ̬
	int index=get_bit(pc,0,two_bit_test1_param.index_len);
	//����״ֵ̬�õ�test1_output�ж�Ӧ������ֵ
	index=two_bit_test1_param.table[index];

	//��������ֵ������test1_output
	if(taken==1)
		test1_output[index][1]++;
	else
		test1_output[index][0]++;
}

//���step֮�󣬸���output�м�¼����Ϣ������ÿ��״̬��Ӧ�����
void test1_change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ��ת
		if(test1_output[i][1]>test1_output[i][0])
			test1_predict_out[i]=1;
		else if(test1_output[i][1]<test1_output[i][0])//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ����ת
			test1_predict_out[i]=0;
		//test1_output[i][0]=0;
		//test1_output[i][1]=0;
	}
}

//Ԥ�⣬����pc�õ�����������BHT��״̬�����Ӧ�����Ԥ��
int two_bit_test1_predict(int64 pc)
{
	//����pc��ȡBHT������
	int index=get_bit(pc,0,two_bit_test1_param.index_len);
	//����BHT������BHT�����е�״̬���ҵ�״̬������е�Ԥ�����
	return test1_predict_out[two_bit_test1_param.table[index]];
}

//����BHT�еı��ͼ�����
void two_bit_test1_update_saturation(int64 pc, int taken)
{
	int index=get_bit(pc,0,two_bit_test1_param.index_len);
	if(taken==1)
	{
		if(two_bit_test1_param.table[index]<3)
			two_bit_test1_param.table[index]++;
	}
	else
	{
		if((int)two_bit_test1_param.table[index]>0)
			two_bit_test1_param.table[index]--;
	}
}

//��λ��֧Ԥ�����Ĳ���2��������
void two_bit_test1_predictor(int index_len,int init_state)
{
	init_two_bit_test1_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		two_bit_test1_param.inst_num++;
		if(two_bit_test1_param.inst_num%two_bit_test1_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",two_bit_test1_param.play_step,1-1.0*two_bit_test1_param.mid_miss/two_bit_test1_param.play_step);
			//fprintf(log_file,"%d,%lf\n",two_bit_test1_param.inst_num,1-1.0*two_bit_test1_param.mid_miss/two_bit_test1_param.play_step);
			
			//����state out table
			test1_change_predict_out();
			two_bit_test1_param.mid_miss=0;
		}
		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//���ݷ�֧��ʵ�ʽ��������output��
		test1_record_output(pc,taken);

		//predict
		int p=two_bit_test1_predict(pc);
		//judgemet
		if(p!=taken)
		{
			two_bit_test1_param.mid_miss++;
			two_bit_test1_param.miss_num++;
		}
		//update
		two_bit_test1_update_saturation(pc,taken);
	}

	b_result.miss_inst=two_bit_test1_param.miss_num;
	b_result.total_inst=two_bit_test1_param.inst_num;
	b_result.acc=1.0-1.0*two_bit_test1_param.miss_num/two_bit_test1_param.inst_num;

	printf("two bit test1 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(two_bit_test1_param.table);
}