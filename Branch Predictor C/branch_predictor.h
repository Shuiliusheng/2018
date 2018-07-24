#include<stdio.h>
#include<stdlib.h>
#include<iostream>
using namespace std;

typedef unsigned long long int64;

typedef struct {
	int step;//设置预测器的测量步长
	int total_inst;//所有指令的个数
	int miss_inst;//预测错误的个数
	double acc;//准确率
}Branch_result;


typedef struct{
	unsigned char *table;//预测器使用的表BHT
	int index_len;//预测表索引位宽
	int inst_num;//预测器经历的指令数
	int miss_num;//预测错误的指令数
	int mid_miss;//每个step期间的预测错误的个数
	int play_step;//step的长度
}Predictor_Param;

extern Branch_result b_result;
extern FILE *log_file;

bool init_trace(char filename[]);
bool read_next_inst(int64 &pc, int &taken, int64 &taget_pc);


int64 get_bit(int64 src, int start, int num);
int pow_int2(int len);

//index_len : the length of index for look up table
//init_state: the initial state of table
void one_bit_predictor(int index_len,int init_state);

void two_bit_predictor(int index_len,int init_state);

void two_bit_test_predictor(int index_len,int init_state);
void two_bit_test1_predictor(int index_len,int init_state);
void two_bit_test2_predictor(int index_len,int init_state,int out_index_len);

void two_bit_test3_predictor(int index_len,int init_state);
void two_bit_test4_predictor(int index_len,int init_state,int out);


void GAg_predictor(int index_len,int init_state);
void GAg_test1_predictor(int index_len,int init_state);
void GAg_test2_predictor(int index_len,int init_state,int out_index_len);

void gshare_predictor(int index_len,int init_state);
void gshare_test1_predictor(int index_len,int init_state);
void gshare_test2_predictor(int index_len,int init_state,int out_index_len);

void PAg_predictor(int pht_index_len,int init_state,int bht_index_length);
void PAg_test1_predictor(int pht_index_len,int init_state,int bht_index_length);
void PAg_test2_predictor(int pht_index_len,int init_state,int bht_index_length,int out_index_len);