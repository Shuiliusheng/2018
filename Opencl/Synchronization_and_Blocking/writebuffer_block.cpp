#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <string.h>
using namespace std;
bool detail=false;

void help();

void check_event(const char *cmd,unsigned int status)
{
	if(!detail)
		return;
	if(status == CL_QUEUED)
		printf("      %s : been queued \n",cmd);
	else if(status == CL_SUBMITTED)
		printf("      %s : been submitted \n",cmd);
	else if(status == CL_RUNNING)
		printf("      %s : is running \n",cmd);
	else if(status == CL_COMPLETE)
		printf("      %s : is completed \n",cmd);
}

void runtime_info(int argv,char **args)
{
	cout <<"--------setting---------"<<endl;
	if(strcmp(args[2],"-b1")==0||strcmp(args[2],"-b12")==0)
		cout <<"Write buffer1 mode: Block"<<endl;
	else
		cout <<"Write buffer1 mode: UnBlock"<<endl;
	
	if(strcmp(args[2],"-b2")==0||strcmp(args[2],"-b12")==0)
		cout <<"Write buffer2 mode: Block"<<endl;
	else
		cout <<"Write buffer2 mode: UnBlock"<<endl;
	
	if(strcmp(args[3],"-wwl")==0)
		cout <<"clEnqueueWriteBuffer use wait list: Yes"<<endl;
	else
		cout <<"clEnqueueWriteBuffer use wait list: No"<<endl;
	
	if(strcmp(args[4],"-wwe")==0)
		cout <<"Wait two write buffer event over: Yes"<<endl;
	else
		cout <<"Wait two write buffer event over: No"<<endl;
	
	if(strcmp(args[5],"-ewl")==0)
		cout <<"Kernel2 wait Kernel1: Yes"<<endl;
	else
		cout <<"Kernel2 wait Kernel1: No"<<endl;
	
	if(strcmp(args[6],"-war")==0)
		cout <<"Wait all Kernel: Yes"<<endl;
	else
		cout <<"Wait all Kernel: No"<<endl;
	
	if(strcmp(args[7],"-rwl")==0)
		cout <<"clEnqueueReadBuffer use wait list: Yes"<<endl;
	else
		cout <<"clEnqueueReadBuffer use wait list: No"<<endl;
	
	if(strcmp(args[8],"-rb")==0)
		cout <<"Read Buffer mode: Block"<<endl;
	else
		cout <<"Read Buffer mode: UnBlock"<<endl;
	cout <<endl;
}

long file_length(char filename[])
{
	FILE *p=fopen(filename,"r");
	if(p==NULL)
	{
		cout <<filename<<" is not exist!"<<endl;
		return -1;
	}
	fseek(p,0,SEEK_END);
	const long length=ftell(p);
	fclose(p);
	return length;
}

void read_kernel(char filename[],char *source,long length)
{
	FILE *p=fopen(filename,"rb");
	int t=fread(source,1,length,p);
	source[t]='\0';
	fclose(p);
}

double run(int argv,char **args)
{
	cl_uint status;
	cl_int ret;
	cl_event evt1,evt2;
    cl_platform_id platform;
	cl_mem src1MemObj = NULL;
	cl_mem src2MemObj = NULL;
	cl_mem dstMemObj = NULL;
	
	int *pHostBuffer = NULL;
	int *pDeviceBuffer = NULL;

    //创建平台对象
    status = clGetPlatformIDs(1, &platform, NULL);
    cl_device_id device;
    //创建 GPU 设备
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU,1,&device,NULL);
    
	//创建context
	cl_context context = clCreateContext(NULL,1,&device,NULL, NULL, NULL);
	
	//创建命令队列
	cl_command_queue commandQueue = clCreateCommandQueue(context, device,CL_QUEUE_PROFILING_ENABLE, NULL);
	if (commandQueue == NULL) 
			perror("Failed to create commandQueue for device 0.");

	//建立要传入从机的数据
	const size_t contentLength = sizeof(int)*16*1024*1024;
	src1MemObj = clCreateBuffer(context,CL_MEM_READ_ONLY,contentLength,NULL,&ret);
	if(src1MemObj==NULL)
	{
		cout <<"Source1 memory object failed to create!"<<endl;
		return 0;
	}
	src2MemObj = clCreateBuffer(context,CL_MEM_READ_ONLY,contentLength,NULL,&ret);
	if(src2MemObj==NULL)
	{
		cout <<"Source2 memory object failed to create!"<<endl;
		return 0;
	}
	pHostBuffer=(int *)malloc(contentLength);
	for(int i=0;i<contentLength/sizeof(int);i++)
		pHostBuffer[i]=i;
	
	//写设备全局存储器1
	struct timeval tsBegin,tsEnd;
	long t1Duration,t2Duration;
	gettimeofday(&tsBegin,NULL);
	
	//对src1MemObj的数据传输，采用非阻塞方式
	//等后续设置完成后通过事件等待机制进行同步
	if(detail)
		cout <<endl<<"Start Write Buffer 1 (clEnqueueWriteBuffer)"<<endl;
	if(strcmp(args[2],"-b1")==0||strcmp(args[2],"-b12")==0)
		status=clEnqueueWriteBuffer(commandQueue,src1MemObj,CL_TRUE,0,contentLength,pHostBuffer,0,NULL,&evt1);
	else
		status=clEnqueueWriteBuffer(commandQueue,src1MemObj,CL_FALSE,0,contentLength,pHostBuffer,0,NULL,&evt1);
	
	if(status!=CL_SUCCESS)
	{
		printf("Src1 data transfer failed!\n");
		return 0;
	}
	
	ret = clGetEventInfo(evt1, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
	if(ret == CL_SUCCESS)
		check_event("Write buffer 1",status);
	
	gettimeofday(&tsEnd,NULL);
	t1Duration=1000000L*(tsEnd.tv_sec-tsBegin.tv_sec)+(tsEnd.tv_usec-tsBegin.tv_usec);
	
	//写设备全局存储器2
	gettimeofday(&tsBegin,NULL);
	
	//对src2MemObj的数据传输，采用非阻塞方式
	//等后续设置完成后通过事件等待机制进行同步
	
	if(detail)
		cout <<endl<<"Start Write Buffer 2 (clEnqueueWriteBuffer)"<<endl;
	if(strcmp(args[2],"-b2")==0||strcmp(args[2],"-b12")==0)
	{
		if(strcmp(args[3],"-wwl")==0)
			status=clEnqueueWriteBuffer(commandQueue,src2MemObj,CL_TRUE,0,contentLength,pHostBuffer,1,&evt1,&evt2);
		else
			status=clEnqueueWriteBuffer(commandQueue,src2MemObj,CL_TRUE,0,contentLength,pHostBuffer,0,NULL,&evt2);
	}	
	else
	{
		if(strcmp(args[3],"-wwl")==0)
			status=clEnqueueWriteBuffer(commandQueue,src2MemObj,CL_FALSE,0,contentLength,pHostBuffer,1,&evt1,&evt2);
		else
			status=clEnqueueWriteBuffer(commandQueue,src2MemObj,CL_FALSE,0,contentLength,pHostBuffer,0,NULL,&evt2);
	}	
	
	if(status!=CL_SUCCESS)
	{
		printf("Src2 data transfer failed!\n");
		return 0;
	}
	
	gettimeofday(&tsEnd,NULL);
	ret = clGetEventInfo(evt2, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
	if(ret == CL_SUCCESS)
		check_event("Write buffer 2",status);
	
	ret = clGetEventInfo(evt1, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
	if(ret == CL_SUCCESS)
		check_event("Write buffer 1",status);
	
	t2Duration=1000000L*(tsEnd.tv_sec-tsBegin.tv_sec)+(tsEnd.tv_usec-tsBegin.tv_usec);
	printf("Write Buffer 1 time: %ld us\nWrite Buffer 2 time: %ld us\n",t1Duration,t2Duration);
	
	//创建用于结果输出的缓存对象
	//对象类型为可读可写是为了在第一个kernel程序执行完之后
	//既能够作为第二个kernel程序的输入，也能够作为第二个kernel程序的输出
	dstMemObj=clCreateBuffer(context,CL_MEM_READ_WRITE,contentLength,NULL,&ret);
	
	
	//创建program
	long length=file_length(args[1]);
	if(length==-1)
		return 0;
	char *source=(char*)malloc(length);
	read_kernel(args[1],source,length);
	size_t sourceSize[] = { length };
	
	cl_program program = clCreateProgramWithSource(context,1,(const char **)&source,sourceSize,NULL);
	
	//编译program
	status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	if (status != 0)
	{
		printf("clBuild failed:%d\n", status);
		char tbuf[0x10000];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0x10000, tbuf,NULL);
		printf("\n%s\n", tbuf);
	}

	//创建kernel1
	cl_kernel kernel1 = clCreateKernel(program, "kernel_test1", NULL);
	if(kernel1==NULL)
	{
		printf("Kernel 1 failed to create!\n");
		return 0;
	}

	status = clSetKernelArg(kernel1, 0, sizeof(cl_mem), (void *)&dstMemObj);
	status != clSetKernelArg(kernel1, 1,sizeof(cl_mem), (void *)&src1MemObj);
	status != clSetKernelArg(kernel1, 2, sizeof(cl_mem), (void *)&src2MemObj);
	if (status!=CL_SUCCESS)
	{
		printf("Arguments of Kernel1 faild to set!\n");
		return 0;
	}
	
	//运行Kernel1
	//获取最大工作组大小
	size_t maxWorkGroupSize=0;
	clGetDeviceInfo(device,CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(maxWorkGroupSize),&maxWorkGroupSize,NULL);
	
	//等待src1MemObj和src2MemObj的数据传输完毕
	cl_event t[2]={evt1,evt2};
	if(strcmp(args[4],"-wwe")==0)
	{
		if(detail)
			cout <<endl<<"Wait for Evt1 and Evt2 (clWaitForEvents)"<<endl;
		clWaitForEvents(2,t);
		ret = clGetEventInfo(evt2, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
		if(ret == CL_SUCCESS)
			check_event("Write buffer 2",status);
		
		ret = clGetEventInfo(evt1, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
		if(ret == CL_SUCCESS)
			check_event("Write buffer 1",status);
	}	
	
	//释放evt1，evt2
	clReleaseEvent(evt1);
	clReleaseEvent(evt2);
	evt1=NULL;
	evt2=NULL;
	
	//指定工作项个数为  contentLength/sizeof(int)
	//工作组中的工作项的个数为 maxWorkGroupSize
	//使用evt1跟踪 kernel1 的执行状态
	size_t global=contentLength/sizeof(int);
	size_t local=maxWorkGroupSize;
	
	if(detail)
		cout <<endl<<"Start Run Kernel 1 (clEnqueueNDRangeKernel)"<<endl;
	status = clEnqueueNDRangeKernel(commandQueue, kernel1, 1, NULL, &global, &local, 0, NULL, &evt1);
	if (status!=CL_SUCCESS)
	{
		printf("Kernel1 execution failed!\n");
		return 0;
	}
	
	ret = clGetEventInfo(evt1, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
	if(ret == CL_SUCCESS)
		check_event("Kernel 1 execution",status);
	
	//初始化第二个kernel程序
	//当计算设备在计算时，主机端能够不受干扰的继续做其他事情
	cl_kernel kernel2=clCreateKernel(program,"kernel_test2",&ret);
	
	//设置参数
	status = clSetKernelArg(kernel2, 0, sizeof(cl_mem), (void *)&dstMemObj);
	status != clSetKernelArg(kernel2, 1,sizeof(cl_mem), (void *)&src1MemObj);
	status != clSetKernelArg(kernel2, 2, sizeof(cl_mem), (void *)&src2MemObj);
	if (status!=CL_SUCCESS)
	{
		printf("Arguments of Kernel2 faild to set!\n");
		return 0;
	}
	
	//Kernel执行
	if(detail)
		cout <<endl<<"Start Run Kernel 2 (clEnqueueNDRangeKernel)"<<endl;
	if(strcmp(args[5],"-ewl")==0)
		status=clEnqueueNDRangeKernel(commandQueue,kernel2,1,NULL,&global,&local,1,&evt1,&evt2);
	else 
		status=clEnqueueNDRangeKernel(commandQueue,kernel2,1,NULL,&global,&local,0,NULL,&evt2);
	
	if (status!=CL_SUCCESS)
	{
		printf("Kernel2 execution failed!\n");
		return 0;
	}
	
	ret = clGetEventInfo(evt1, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
	if(ret == CL_SUCCESS)
		check_event("Kernel 1 execution",status);
	ret = clGetEventInfo(evt2, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
	if(ret == CL_SUCCESS)
		check_event("Kernel 2 execution",status);
	
	
	//等待执行结束
	if(strcmp(args[6],"-war")==0)
	{
		if(detail)
			cout <<endl<<"Wait for Evt1 and Evt2 (clWaitForEvents)"<<endl;
		cl_event t1[2]={evt1,evt2};
		clWaitForEvents(2, t1);
		ret = clGetEventInfo(evt1, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
		if(ret == CL_SUCCESS)
			check_event("Kernel 1 execution",status);
		ret = clGetEventInfo(evt2, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
		if(ret == CL_SUCCESS)
			check_event("Kernel 2 execution",status);
	}
	
	//结果校验
	pDeviceBuffer=(int*)malloc(contentLength);
	
	//读取计算设备端的数据的命令通过evt2进行同步
	//确保kernel2完成执行之后再执行读数据命令
	
	if(detail)
		cout <<endl<<"Start Read Buffer (clEnqueueReadBuffer)"<<endl;
	if(strcmp(args[7],"-rwl")==0)
	{
		if(strcmp(args[8],"-rb")==0)
			clEnqueueReadBuffer(commandQueue,dstMemObj,CL_TRUE,0,contentLength,pDeviceBuffer,1,&evt2,NULL);
		else
			clEnqueueReadBuffer(commandQueue,dstMemObj,CL_FALSE,0,contentLength,pDeviceBuffer,1,&evt2,NULL);
	}	
	else
	{
		if(strcmp(args[8],"-rb")==0)
			clEnqueueReadBuffer(commandQueue,dstMemObj,CL_TRUE,0,contentLength,pDeviceBuffer,0,NULL,NULL);
		else
			clEnqueueReadBuffer(commandQueue,dstMemObj,CL_FALSE,0,contentLength,pDeviceBuffer,0,NULL,NULL);
	}	
	
	ret = clGetEventInfo(evt1, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
	if(ret == CL_SUCCESS)
		check_event("Kernel 1 execution",status);
	ret = clGetEventInfo(evt2, CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status), &status, NULL);
	if(ret == CL_SUCCESS)
		check_event("Kernel 2 execution",status);
	
	int i=0;
	for(i=0;i<contentLength/sizeof(int);i++)
	{
		int test=pHostBuffer[i]+pHostBuffer[i];
		test=test*pHostBuffer[i]-pHostBuffer[i];
		if(test!=pDeviceBuffer[i])
		{
			printf("Error occured @%d, result is: (%d %d)\n",i,test,pDeviceBuffer[i]);
			break;
		}
	}
	if(i==contentLength/sizeof(int))
	{
		printf("Result is OK!\n");
	}

	//释放变量
	clReleaseMemObject(src1MemObj);
	clReleaseMemObject(src2MemObj);
	clReleaseMemObject(dstMemObj);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
	return 0;
}


int main(int argv, char** args)
{
	if(argv<=1)
	{
		help();
		return 0;
	}
	runtime_info(argv,args);
	if(argv>9)
		detail=true;
	if(detail)
		cout <<endl<<"-------------------Runing Info---------------"<<endl;
	run(argv,args);
    return 0;
}


void help()
{
	printf("RUN ARGUMENTS\n");
	printf("./run kernel_name -b1/-b2/-b12/-ub \n");
	printf("		-wwl/-uwwl		-wwe/-uwwe  \n");
	printf("		-ewl/-uewl		-war/-uwar \n");
	printf("		-rwl/-urwl		-rb/-urb    -d\n");
	printf("\n");
	printf("EXPLANATION\n");
	printf("-b1/-b2/-b12/-ub (block)\n");
	printf("	-b1:  Write Buffer1 use Block mode, while Buffer2 use UnBlock mode\n");
	printf("	-b2:  Write Buffer1 use UnBlock mode, while Buffer2 use Block mode\n");
	printf("	-b12: Write Buffer1 and Buffer2 all use Block mode\n");
	printf("	-ub:  Write Buffer1 and Buffer2 all use UnBlock mode\n");
	printf("-wwl/-uwwl (Write buffer use Wait event List)\n");
	printf("	-wwl: clEnqueueWriteBuffer(Buffer2) use wait list for Buffer1 Write event\n");
	printf("	-uwwl:clEnqueueWriteBuffer(Buffer2) don't use wait list\n");
	printf("-wwe/-uwwe (Wait for all Write buffer Events)\n");
	printf("	-wwe: Use clWaitForEvents() wait two Write Events completed\n");
	printf("	-uwwe:Don't use this function\n");
	printf("-ewl/-uewl (EnqueueNDRangeKernel use Wait events List)\n");
	printf("	-ewl: clEnqueueNDRangeKernel(Kernel2) use wait list for Kernel1 Run event\n");
	printf("	-uewl:clEnqueueNDRangeKernel(Kernel2) don't use wait list\n");
	printf("-war/-uwar (Wait All kernel Run events)\n");
	printf("	-war: Use clWaitForEvents() wait two Kernel Run Events completed\n");
	printf("	-uwar:Don't use this function\n");
	printf("-rwl/-urwl (Read buffer use Wait events List )\n");
	printf("	-rwe: clEnqueueReadBuffer(pDeviceBuffer) use wait list for Kernel2 Run event\n");
	printf("	-urwl:clEnqueueReadBuffer(pDeviceBuffer) don't use wait list\n");
	printf("-rb/-urb (Read buffer Block)\n");
	printf("	-rb:  Read pDeviceBuffer use Block mode\n");
	printf("	-urb: Read pDeviceBuffer use UnBlock mode\n");
	printf("-d: Print detail information When run\n");
	printf("Example: ./run kernel.cl -ub -wwl -wwe -ewl -war -rwl -rb -d\n");
}