#include"branch_predictor.h"

/*
	gshare predictor test1:������gshareһ�£�
	��������gshare test1������output��predictout���Ӷ���̬�ĵ����Զ���״̬�����
	��ѡ������PC��BHRʹ�õ�λ���PHT�����еĳ�ʼ״̬
*/

//Ԥ��������
Predictor_Param gshare_test1_param;
//ȫ��BHR�Ĵ���
int64 gshare_test1_bhr;
//����ͳ��ÿ��״̬�Ϸ����ķ�֧����Ϣ
int gshare_test1_output[4][2];
//״̬�����Ӧ��
char gshare_test1_predict_out[4];

//��ʼ��
void init_gshare_test1_predictor(int index_length,int init_state)
{
	int entry_num=pow_int2(index_length);
	gshare_test1_param.index_len=index_length;
	gshare_test1_param.inst_num=0;
	gshare_test1_param.miss_num=0;
	gshare_test1_param.mid_miss=0;
	gshare_test1_param.play_step=b_result.step;
	gshare_test1_param.table=(unsigned char*)malloc(entry_num);
	//��ʼ��BHRΪ0
	gshare_test1_bhr=0;
	//��ʼ��ÿ��״̬��Ӧ���������ʼΪ��λ���ͼ���������λ����Ԥ��
	gshare_test1_predict_out[0]=0;
	gshare_test1_predict_out[1]=0;
	gshare_test1_predict_out[2]=1;
	gshare_test1_predict_out[3]=1;
	//��ʼ��outputΪ0
	for(int i=0;i<8;i++)
		gshare_test1_output[i/2][i%2]=0;

	for(int i=0;i<entry_num;i++)
		gshare_test1_param.table[i]=init_state;
}

//ÿ���ڵõ�ʵ�ʷ�֧��ת���֮�󣬽����¼��output��
void gshare_test1_record_output(int64 pc,int taken)
{
	//����BHR����ȡ����1
	int index=get_bit(gshare_test1_bhr,0,gshare_test1_param.index_len);
	//����pc����ȡ����2
	int index1=get_bit(pc,0,gshare_test1_param.index_len);
	//���������������������õ����յ�����
	index=index1^index;
	//�����������ҵ�PHT�еı���õ����е�״ֵ̬
	index=gshare_test1_param.table[index];
	//����״ֵ̬����output
	if(taken==1)
		gshare_test1_output[index][1]++;
	else
		gshare_test1_output[index][0]++;
}

//���step֮�󣬸���output�м�¼����Ϣ������ÿ��״̬��Ӧ�����
void gshare_test1_change_predict_out()
{
	for(int i=0;i<4;i++)
	{
		if(gshare_test1_output[i][1]>gshare_test1_output[i][0])
			gshare_test1_predict_out[i]=1;
		else if(gshare_test1_output[i][1]<gshare_test1_output[i][0])
			gshare_test1_predict_out[i]=0;
		gshare_test1_output[i][0]=0;
		gshare_test1_output[i][1]=0;
	}
}

//Ԥ��
int gshare_test1_predict(int64 pc)
{
	//ʹ��BHR��ȡ����1
	int index=get_bit(gshare_test1_bhr,0,gshare_test1_param.index_len);
	//ʹ��pc��ȡ����2����Ϊ��index_lenλ
	int index1=get_bit(pc,0,gshare_test1_param.index_len);
	//���������������������õ�ʵ�ʵ�����
	index=index1^index;
	//����PHT������PHT��predictout����Ԥ��
	return gshare_test1_predict_out[gshare_test1_param.table[index]];
}

//����PHT�ı���
void gshare_test1_update_saturation(int64 pc,int taken)
{
	int index=get_bit(gshare_test1_bhr,0,gshare_test1_param.index_len);
	int index1=get_bit(pc,0,gshare_test1_param.index_len);
	index=index1^index;
	if(taken==1)
	{
		if(gshare_test1_param.table[index]<3)
			gshare_test1_param.table[index]++;
	}
	else
	{
		if((int)gshare_test1_param.table[index]>0)
			gshare_test1_param.table[index]--;
	}
}
//����BHR����λ�Ĵ���
void gshare_test1_update_gshare_test1_bhr(int taken)
{
	gshare_test1_bhr=((gshare_test1_bhr<<1)+taken);
}
//gshare test1Ԥ����������
void gshare_test1_predictor(int index_len,int init_state)
{
	init_gshare_test1_predictor(index_len,init_state);
	int64 pc,target_pc;
	int taken;
	while(1)
	{
		gshare_test1_param.inst_num++;
		if(gshare_test1_param.inst_num%gshare_test1_param.play_step==0)
		{
			//printf("step len:%d, hit rate: %lf\n",gshare_test1_param.play_step,1-1.0*gshare_test1_param.mid_miss/gshare_test1_param.play_step);
			//fprintf(log_file,"%d,%lf\n",gshare_test1_param.inst_num,1-1.0*gshare_test1_param.mid_miss/gshare_test1_param.play_step);
			gshare_test1_param.mid_miss=0;
			
			//����state out table
			gshare_test1_change_predict_out();
		}
		//��ȡ��һ��ָ��
		if(!read_next_inst(pc,taken,target_pc))
			break;

		//���ݷ�֧��ʵ�ʽ��������output��
		gshare_test1_record_output(pc,taken);

		//predict
		int p=gshare_test1_predict(pc);
		//judgemet
		if(p!=taken)
		{
			gshare_test1_param.mid_miss++;
			gshare_test1_param.miss_num++;
		}
		//update
		gshare_test1_update_saturation(pc,taken);

		//����BHR
		gshare_test1_update_gshare_test1_bhr(taken);
	}

	b_result.miss_inst=gshare_test1_param.miss_num;
	b_result.total_inst=gshare_test1_param.inst_num;
	b_result.acc=1.0-1.0*gshare_test1_param.miss_num/gshare_test1_param.inst_num;

	printf("gshare_test1 predictor (entry number:%d, initial state:%d)\n",pow_int2(index_len),init_state);
	free(gshare_test1_param.table);
}