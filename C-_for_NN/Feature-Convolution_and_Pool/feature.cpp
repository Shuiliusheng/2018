#include"feature.h"

Data input;
Data conv1,conv2,conv3,conv4;
Data pool1,pool2,pool3,pool4;

char name[200]="";

void test()
{
	init_filters();
	if(strlen(name)==0)
	{
		cout <<"input image filename:"<<endl;
		cin >>name;
	}
	read_data(name,input);

	if(!convolution(input,conv1,18,0,true))
		return ;
	//save_data(conv1,500,500,"g:\\asm\\conv1");
	if(!max_pooling(conv1,pool1,4))
		return ;
	save_data(pool1,800,400,"g:\\asm\\pool1",1);


	if(!convolution(pool1,conv2,36,18,true))
		return ;
	//save_data(conv2,500,500,"g:\\asm\\conv1");
	if(!max_pooling(conv2,pool2,4))
		return ;
	save_data(pool2,400,100,"g:\\asm\\pool2",3);


	if(!convolution(pool2,conv3,161,20,true))
		return ;
	//save_data(conv2,500,500,"g:\\asm\\conv1");
	if(!max_pooling(conv3,pool3,2))
		return ;
	save_data(pool3,400,100,"g:\\asm\\pool3",3);


	if(!convolution(pool3,conv4,528,0,true))
		return ;
	//save_data(conv2,500,500,"g:\\asm\\conv1");
	if(!max_pooling(conv4,pool4,2))
		return ;
	save_data(pool4,400,100,"g:\\asm\\pool4",3);

	cout <<"Successfully!"<<endl;
}


int main(int argv,char **args)
{
	if(argv>1)
		strcpy(name,args[1]);
	test();
	waitKey(0);
	getchar();
	return 0;
}