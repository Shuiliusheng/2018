#include"nn_minst.h"
#include<string.h>
//param
double eta=0.4;   //¦Ç
double lambda=0.1;  //¦Ë
int GROUP=6000;

void help()
{
	cout <<"**********parameters meanings************"<<endl;
	cout <<"image_name : the image filename"<<endl;
	cout <<"label_name : the corresponding label filename"<<endl;
	cout <<"batchsize  : the batch size."<<endl;
	cout <<"eta        : the learning rate.(W=W-eta*J(w,b))"<<endl;
	cout <<"lamda      : the regularization parameter.(Lost=J(w,b)+lambda*sigma(W*W)/2)"<<endl;
	cout <<"train_times: the max times when training"<<endl;
	cout <<"param_filename: the file save the net parameters after training"<<endl;
	cout <<"                Traintimes_2000_Group_128_eta_0.300000_lambda_0.000100"<<endl<<endl;

	cout <<"**********command line parameter************"<<endl;
	cout <<"training parameters"<<endl;
	cout <<"  -train image_name label_name batchsize eta lambda train_times"<<endl;
	cout <<endl<<"testing parameters"<<endl;
	cout <<"  -test image_name label_name param_filename batchsize"<<endl;
	cout <<"       !! batchsize need be all testing image number"<<endl;
	cout <<endl;
	cout <<endl;

}
int main(int argc,char *argv[])
{
	if(strcmp(argv[1],"-train")==0)
	{
		GROUP=-1;
		train_times=-1;
		strcpy(image_name,argv[2]);
		strcpy(label_name,argv[3]);
		sscanf(argv[4],"%d",&GROUP);
		sscanf(argv[5],"%lf",&eta);
		sscanf(argv[6],"%lf",&lambda);
		sscanf(argv[7],"%d",&train_times);
		
		if(GROUP<=0||train_times<=0)
		{
			printf("parameter is wrong!\n");
			return 0;
		}
		cout <<"image_name:"<<image_name<<endl;
		cout <<"label_name:"<<label_name<<endl;
		cout <<"GROUP:"<<GROUP<<endl;
		cout <<"eta:"<<eta<<endl;
		cout <<"lambda:"<<lambda<<endl;
		cout <<"train times:"<<train_times<<endl;

		init();
		training();
	}
	else if(strcmp(argv[1],"-test")==0)
	{
		char filename[300];
		strcpy(image_name,argv[2]);
		strcpy(label_name,argv[3]);
		strcpy(filename,argv[4]);
		sscanf(filename,"Traintimes_%d_Group_%d_eta_%lf_lambda_%lf.param",&train_times,&GROUP,&eta,&lambda);
		sscanf(argv[5],"%d",&GROUP);

		cout <<"image_name:"<<image_name<<endl;
		cout <<"label_name:"<<label_name<<endl;
		cout <<"param_name:"<<filename<<endl;
		cout <<"GROUP:"<<GROUP<<endl;
		cout <<"eta:"<<eta<<endl;
		cout <<"lambda:"<<lambda<<endl;

		init();
		if(!load_param(filename,W1,W2,W3,LEN,HIDDEN1,HIDDEN2,OUT))
		{
			return 0;
		}
		testing(filename);
	}
	else
	{
		help();
	}
	return 0;
}