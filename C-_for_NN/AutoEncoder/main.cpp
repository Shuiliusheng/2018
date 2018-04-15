#include"encode.h"
//encode param
double eta=0.2;   //¦Ç
double lambda=0.35;  //¦Ë

//classify param
double eta1=0.3;
double lambda1=0.1;

int GROUP=3000;

int main()
{
	int choice=0;
	cout <<"input the train group number:";
	cin >>GROUP;
	cout <<"---------------*---------------"<<endl;
	cout <<"   1:encode network training   "<<endl;
	cout <<"   2:classify network training "<<endl;
	cout <<"   3:testing   "<<endl;
	cout <<"input your choice:";
	cin >>choice;

	init();
	if(choice==1)
	{
		cout <<"input the encode training file name:"<<endl;
		cin >>image_name;
		cout <<"input the encode param -- eta :";
		cin >>eta;
		cout <<"input the encode param -- lambda :";
		cin >>lambda;

		encode_training();
	}
	else 
	{
		cout <<"input the *encode* training file name:"<<endl;
		cin >>image_name;
		cout <<"input the *encode* param -- eta :";
		cin >>eta;
		cout <<"input the *encode* param -- lambda :";
		cin >>lambda;

		cout <<"input the *classify* training file name:"<<endl;
		cin >>label_name;
		cout <<"input the *classify* param -- eta :";
		cin >>eta1;
		cout <<"input the *classify* param -- lambda :";
		cin >>lambda1;

		char filename[300],filename1[300];
		sprintf(filename,"Encode_Group_%d_eta_%lf_lambda_%lf.txt",GROUP,eta,lambda);
		sprintf(filename1,"Classify_Group_%d_eta_%lf_lambda_%lf.txt",GROUP,eta1,lambda1);

		if(!load_param(filename,W1,HIDDEN,LEN))
		{
			getchar();
			return 0;
		}
		

		if(choice==2)
		{
			classify_training();
		}
		else if(choice==3)
		{
			if(load_param(filename1,W,HIDDEN,HIDDEN))
			{

			}
		}
	}
	cout <<"input 0 to exit:";
	cin >>choice;
	getchar();
	return 0;
}