#include<iostream>
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include <time.h> 
using namespace std;

//动量项参数
#define R 0.001

//输入层长度
#define LEN 784

//第一层长度
#define HIDDEN1 200

//第二层长度
#define HIDDEN2 50

//输出层长度
#define OUT 10

//batchsize 
extern int GROUP;
//learning rate
extern double eta;
//regulatization parameter
extern double lambda;
//filename
extern char image_name[300];
extern char label_name[300];
//training times
extern int train_times;

extern double **in;//[GROUP][LEN];
extern double **out;//[GROUP][OUT];

extern double **W1;//[HIDDEN1][LEN];// the first argu
extern double **W2;//[HIDDEN2][HIDDEN1];// the second argu, setting like the formula
extern double **W3;//[OUT][HIDDEN2]

extern double **V1;//[HIDDEN1][LEN];// the first argu
extern double **V2;//[HIDDEN2][HIDDEN1];// the second argu, setting like the formula
extern double **V3;//[OUT][HIDDEN2]

extern double **Z1;//[GROUP][HIDDEN1];//the first layer input and output
extern double **A1;//[GROUP][HIDDEN1];
extern double **delta1;//[GROUP][HIDDEN1];


extern double **Z2;//[GROUP][HIDDEN2];//the second layer input and output
extern double **A2;//[GROUP][HIDDEN2];
extern double **delta2;//[GROUP][HIDDEN2];

extern double **Z3;//[GROUP][OUT];//the second layer input and output
extern double **A3;//[GROUP][OUT];
extern double **delta3;//[GROUP][OUT];


void init();
bool init_image();
bool init_label();

void read_data(double **dst);
void read_label(double **dst);

//activation fuction
void fuction(double z,double &a);
double d_fuction(double a);

void train_clear_memory();
void train_init_memory();
void fore_spread();
void back_spread();
void training();

int find_max(double *src,int len);
bool load_param(char name[],double **W1,double **W2,double **W3,int l0,int l1,int l2,int l3);
void testing(char str[]);



