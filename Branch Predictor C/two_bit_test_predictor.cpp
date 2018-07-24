#include"branch_predictor.h"
/*
��λ������֧Ԥ����������1
�������ݣ����step����������ͳ�Ƶ���Ϣ������ÿ��״̬��Ӧ�������Ϣ
          ���step֮��ͳ����Ϣ�����㣬���½���ͳ��
		  step�ɱ䣬�������еı���ֻ��һ��ͳ��ʹ�õ�output
*/

//Ԥ��������
Predictor_Param two_bit_test_param;

//output ���ڼ�¼ÿ��״̬�£�������֧����ת�������Ŀ
int output[4][2];
//���ڸ���ÿ��״̬Ӧ�е�������
char predict_out[4];
void init_two_bit_test_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	two_bit_test_param.index_len=index_length;
	two_bit_test_param.inst_num=0;
	two_bit_test_param.miss_num=0;
	two_bit_test_param.mid_miss=0;
	two_bit_test_param.play_step=b_result.step;
	two_bit_test_param.table=(unsigned char*)malloc(entry_num);

	//��ʼ��ÿ��״̬��Ӧ���������ʼΪ��λ���ͼ���������λ����Ԥ��
	predict_out[0]=0;
	predict_out[1]=0;
	predict_out[2]=1;
	predict_out[3]=1;

	//��ʼ��outputΪ0
	for(int i=0;i<8;i++)
		output[i/2][i%2]=0;
	for(int i=0;i<entry_num;i++)
		two_bit_test_param.table[i]=init_state;
}

//ÿ���ڵõ�ʵ�ʷ�֧��ת���֮�󣬽����¼��output��
void record_output(int64 pc,int taken)
{
	//����pc��ȡBHT�����е�״ֵ̬
	int index=get_bit(pc,0,two_bit_test_param.index_len);
	//����״ֵ̬�õ�output�ж�Ӧ������ֵ
	index=two_bit_test_param.table[index];

	//��������ֵ������output
	if(taken==1)
		output[index][1]++;
	else
		output[index][0]++;
}

//���step֮�󣬸���output�м�¼����Ϣ������ÿ��״̬��Ӧ�����
void change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ��ת
		if(output[i][1]>output[i][0])
			predict_out[i]=1;
		else if(output[i][1]<output[i][0])//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ����ת
			predict_out[i]=0;
		//���֮ǰ��¼����Ϣ
		output[i][0]=0;
		output[i][1]=0;
	}
}

//Ԥ�⣬����pc�õ�����������BHT��״̬�����Ӧ�����Ԥ��
int two_bit_test_predict(int64 pc)
{
	//����pc��ȡBHT������
	int index=get_bit(pc,0,two_bit_test_param.index_len);
	//����BHT������BHT�����е�״̬���ҵ�״̬������е�Ԥ�����
	return predict_out[two_bit_test_param.table[index]];
}

//����BHT�еı��ͼ�����
void two_bit_test_update_saturation(int64 pc, int taken)
{
	int index=get_bit(pc,0,two_bit_test_param.index_len);
	if(taken==1)
	{
		if(two_bit_test_param.table[index]<3)
			two_bit_test_param.table[index]++;
	}
	else
	{
		if((int)two_bit_test_param.table[index]>0)
			two_bit_test_param.table[index]--;
	}
}

//��λ��֧Ԥ�����Ĳ���1��������
void two_bit_test_predictor(int index_len,int init_state)
{
	init_two_bit_test_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		two_bit_test_param.inst_num++;

		//���step֮�󣬸���״̬�����Ӧ��
		if(two_bit_test_param.inst_num%two_bit_test_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",two_bit_test_param.play_step,1-1.0*two_bit_test_param.mid_miss/two_bit_test_param.play_step);
			//fprintf(log_file,"%d,%lf\n",two_bit_test_param.inst_num,1-1.0*two_bit_test_param.mid_miss/two_bit_test_param.play_step);
			
			//����state out table
			change_predict_out();
			two_bit_test_param.mid_miss=0;
		}
		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//���ݷ�֧��ʵ�ʽ��������output��
		record_output(pc,taken);

		//predict
		int p=two_bit_test_predict(pc);
		//judgemet
		if(p!=taken)
		{
			two_bit_test_param.mid_miss++;
			two_bit_test_param.miss_num++;
		}
		//update
		two_bit_test_update_saturation(pc,taken);
	}

	b_result.miss_inst=two_bit_test_param.miss_num;
	b_result.total_inst=two_bit_test_param.inst_num;
	b_result.acc=1.0-1.0*two_bit_test_param.miss_num/two_bit_test_param.inst_num;

	printf("two bit test0 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(two_bit_test_param.table);
}