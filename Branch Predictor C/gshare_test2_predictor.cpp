#include"branch_predictor.h"
/*
	gshare predictor test1:������gshare test1һ�£�
	��������ʹ���˾��ж������output���״̬�����Ӧ��
	��ѡ������PC��BHRʹ�õ�λ��PHT�����еĳ�ʼ״̬��output�ı������
*/

//Ԥ��������
Predictor_Param gshare_test2_param;

//ȫ��BHR
int64 gshare_test2_bhr;
//���ж�������output���������8192������
int gshare_test2_output[8192][4][2];
//���ж�������predictout��
char gshare_test2_predict_out[8192][4];
//����output��predictout�ı���λ��
int gshare_test2_out_index_bits;

//��ʼ��
void init_gshare_test2_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	gshare_test2_param.index_len=index_length;
	gshare_test2_param.inst_num=0;
	gshare_test2_param.miss_num=0;
	gshare_test2_param.mid_miss=0;
	gshare_test2_param.play_step=b_result.step;
	gshare_test2_param.table=(unsigned char*)malloc(entry_num);
	
	//����BHR
	gshare_test2_bhr=0;

	//�������б���
	int out_entry_num=pow_int2(gshare_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
	{
		//��ʼ��predictoutΪ����״̬�����λԤ���״̬
		gshare_test2_predict_out[n][0]=0;
		gshare_test2_predict_out[n][1]=0;
		gshare_test2_predict_out[n][2]=1;
		gshare_test2_predict_out[n][3]=1;
		//��ʼ��outputΪ0
		for(int i=0;i<8;i++)
			gshare_test2_output[n][i/2][i%2]=0;
	}

	for(int i=0;i<entry_num;i++)
		gshare_test2_param.table[i]=init_state;
}

//ÿ���ڵõ�ʵ�ʷ�֧��ת���֮�󣬽����¼��output��
void gshare_test2_record_output(int64 pc,int taken)
{
	//����BHR����ȡ����1
	int index=get_bit(gshare_test2_bhr,0,gshare_test2_param.index_len);
	//����pc����ȡ����2
	int index1=get_bit(pc,0,gshare_test2_param.index_len);
	//���������������������õ����յ�����
	index=index1^index;

	//����PHT����������ȡoutput�ĵ�һά����
	index1=get_bit(index,0,gshare_test2_out_index_bits);

	//����PHT�����м�¼��״ֵ̬����ȡoutput�ĵڶ�ά����
	index=gshare_test2_param.table[index];
	//����״ֵ̬����output
	if(taken==1)
		gshare_test2_output[index1][index][1]++;
	else
		gshare_test2_output[index1][index][0]++;
}

//���step֮�󣬸���output�м�¼����Ϣ������ÿ��״̬��Ӧ�����
void gshare_test2_change_predict_out()
{
	//�������е�predictout����
	int out_entry_num=pow_int2(gshare_test2_out_index_bits);
	for(int n=0;n<out_entry_num;n++)
		for(int i=0;i<4;i++)
		{
			//��ת�Ĵ������ڲ���ת�Ĵ�������Ԥ��ֵӦΪ��ת
			if(gshare_test2_output[n][i][1]>gshare_test2_output[n][i][0])
				gshare_test2_predict_out[n][i]=1;
			else if(gshare_test2_output[n][i][1]<gshare_test2_output[n][i][0])
				gshare_test2_predict_out[n][i]=0;
			//���֮ǰ��¼����Ϣ
			gshare_test2_output[n][i][0]=0;
			gshare_test2_output[n][i][1]=0;
		}
}

//����pc��BHR������PHT��predictout,����Ԥ��
int gshare_test2_predict(int64 pc)
{
	//ʹ��BHR��ȡ����1
	int index=get_bit(gshare_test2_bhr,0,gshare_test2_param.index_len);
	//ʹ��pc��ȡ����2����Ϊ��index_lenλ
	int index1=get_bit(pc,0,gshare_test2_param.index_len);
	//���������������������õ�ʵ�ʵ�����
	index=index1^index;
	//����PHT����������ȡpredictout�ĵ�һά����
	index1=get_bit(index,0,gshare_test2_out_index_bits);
	//����PHT��������predictout������������PHT��predictout����Ԥ��
	return gshare_test2_predict_out[index1][gshare_test2_param.table[index]];
}

//����PHT�ı���
void gshare_test2_update_saturation(int64 pc,int taken)
{
	int index=get_bit(gshare_test2_bhr,0,gshare_test2_param.index_len);
	int index1=get_bit(pc,0,gshare_test2_param.index_len);
	index=index1^index;

	if(taken==1)
	{
		if(gshare_test2_param.table[index]<3)
			gshare_test2_param.table[index]++;
	}
	else
	{
		if((int)gshare_test2_param.table[index]>0)
			gshare_test2_param.table[index]--;
	}
}

//����BHR����λ�Ĵ���
void gshare_test2_update_gshare_test2_bhr(int taken)
{
	gshare_test2_bhr=((gshare_test2_bhr<<1)+taken);
}

//gshare test2Ԥ����������
void gshare_test2_predictor(int index_len,int init_state,int out_index_len)
{
	gshare_test2_out_index_bits=out_index_len;
	init_gshare_test2_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		gshare_test2_param.inst_num++;
		if(gshare_test2_param.inst_num%gshare_test2_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",gshare_test2_param.play_step,1-1.0*gshare_test2_param.mid_miss/gshare_test2_param.play_step);
			fprintf(log_file,"%d,%lf\n",gshare_test2_param.inst_num,1-1.0*gshare_test2_param.mid_miss/gshare_test2_param.play_step);
			gshare_test2_param.mid_miss=0;
			
			//����state out table
			gshare_test2_change_predict_out();
		}
		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//���ݷ�֧��ʵ�ʽ��������output��
		gshare_test2_record_output(pc,taken);

		//predict
		int p=gshare_test2_predict(pc);
		//judgemet
		if(p!=taken)
		{
			gshare_test2_param.mid_miss++;
			gshare_test2_param.miss_num++;
		}
		//update
		gshare_test2_update_saturation(pc,taken);

		//����BHR
		gshare_test2_update_gshare_test2_bhr(taken);
	}

	b_result.miss_inst=gshare_test2_param.miss_num;
	b_result.total_inst=gshare_test2_param.inst_num;
	b_result.acc=1.0-1.0*gshare_test2_param.miss_num/gshare_test2_param.inst_num;

	printf("gshare_test2 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(gshare_test2_param.table);
}