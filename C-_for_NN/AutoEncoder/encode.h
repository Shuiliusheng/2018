#include<iostream>
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include <time.h> 
using namespace std;

#define LEN 784
#define HIDDEN 10

#define N (7840*2)   //wµÄ¸öÊý

extern int GROUP;
extern double eta;
extern double lambda;
extern double eta1;
extern double lambda1;

extern double **in;//[GROUP][LEN];

extern double **W1;//[HIDDEN][LEN];// the first argu
extern double **W2;//[LEN][HIDDEN];// the second argu, setting like the formula

extern double **Z1;//[GROUP][HIDDEN];//the first layer input and output
extern double **A1;//[GROUP][HIDDEN];
extern double **delta1;//[GROUP][HIDDEN];


extern double **Z2;//[GROUP][LEN];//the second layer input and output
extern double **A2;//[GROUP][LEN];
extern double **delta2;//[GROUP][LEN];

extern double **A;//[GROUP][HIDDEN];
extern double **Z;//[GROUP][HIDDEN];
extern double **W;//[HIDDEN][HIDDEN];

extern FILE *file;
extern char image_name[300];
extern char label_name[300];

void init();
bool init_image();
bool init_label();

void read_data(double **dst);
void read_label(double **dst);

void fuction(double z,double &a);
void fore_spread_group_once(double **input,double **Z,double **A,double **w,int row, int col);

void fore_spread();
void back_spread();

void encode_training();
double d_fuction(double a);

void classify_training();
void classify_save_param(double **w,double lost,int row,int col);

bool load_param(char name[],double **w,int row,int col);