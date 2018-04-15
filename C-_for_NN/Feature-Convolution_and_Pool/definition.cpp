#include"feature.h"

double Filters[MAX_FILTERS][3][3];

//filters define
double filter0[3][3]={{0,0,0},{0,1,0},{0,0,0}};
double filter1[3][3]={{1/16.0,1/16.0,1/16.0},{1/16.0,8/16.0,1/16.0},{1/16.0,1/16.0,1/16.0}};

double filter2[3][3]={{-1,0,0},{0,1,0},{0,0,0}};
double filter3[3][3]={{0,-1,0},{0,1,0},{0,0,0}};
double filter4[3][3]={{0,0,0},{-1,0,1},{0,0,0}};
double filter5[3][3]={{1.0/9,1.0/9,1.0/9},{1.0/9,1.0/9,1.0/9},{1.0/9,1.0/9,1.0/9}};
double filter6[3][3]={{0,0,0},{-1,2,-1},{0,0,0}};
double filter7[3][3]={{0,1,0},{1,-4,1},{0,1,0}};

double filter8[3][3]={{0,0,-1},{0,1,0},{0,0,0}};
double filter9[3][3]={{0,0,0},{-1,1,0},{0,0,0}};
double filter10[3][3]={{0,-1,0},{0,0,0},{0,1,0}};
double filter11[3][3]={{1/16.0,2/16.0,1/16.0},{2/16.0,4/16.0,2/16.0},{1/16.0,2/16.0,1/16.0}};
double filter12[3][3]={{0,-1,0},{0,2,0},{0,-1,0}};
double filter13[3][3]={{1,1,1},{1,-7,1},{1,1,1}};

double filter14[3][3]={{0,0,0},{0,1,-1},{0,0,0}};
double filter15[3][3]={{0,0,0},{0,1,0},{-1,0,0}};
double filter16[3][3]={{-1,0,0},{0,0,0},{0,0,1}};
double filter17[3][3]={{1/16.0,1/16.0,1/16.0},{2/16.0,6/16.0,2/16.0},{1/16.0,1/16.0,1/16.0}};
double filter18[3][3]={{-1,0,0},{0,2,0},{0,0,-1}};
double filter19[3][3]={{-1,-1,-1},{-1,8,-1},{-1,-1,-1}};

double filter20[3][3]={{0,0,0},{0,1,0},{0,-1,0}};
double filter21[3][3]={{0,0,0},{0,1,0},{0,0,-1}};
double filter22[3][3]={{0,0,-1},{0,0,0},{1,0,0}};
double filter23[3][3]={{1/16.0,2/16.0,1/16.0},{1/16.0,6/16.0,1/16.0},{1/16.0,2/16.0,1/16.0}};
double filter24[3][3]={{0,0,-1},{0,2,0},{-1,0,0}};
double filter25[3][3]={{0,-1,0},{-1,5,-1},{0,-1,0}};


void set_filters(int n,double filter[3][3])
{
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			Filters[n][i][j]=filter[i][j];
}

void init_filters()
{
	set_filters(0,filter0);
	set_filters(1,filter1);
	set_filters(2,filter2);
	set_filters(3,filter3);
	set_filters(4,filter4);
	set_filters(5,filter5);
	set_filters(6,filter6);
	set_filters(7,filter7);
	set_filters(8,filter8);
	set_filters(9,filter9);
	set_filters(10,filter10);
	set_filters(11,filter11);
	set_filters(12,filter12);
	set_filters(13,filter13);
	set_filters(14,filter14);
	set_filters(15,filter15);
	set_filters(16,filter16);
	set_filters(17,filter17);
	set_filters(18,filter18);
	set_filters(19,filter19);
	set_filters(20,filter20);
	set_filters(21,filter21);
	set_filters(22,filter22);
	set_filters(23,filter23);
	set_filters(24,filter24);
	set_filters(25,filter25);
}
