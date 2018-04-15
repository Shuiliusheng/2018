#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#define TS 32
using namespace std;

#define NWITEMS 6
//把文本文件读入一个 string 中
int convertToString(const char *filename, std::string& s)
{
    size_t size;
    char* str;
    std::fstream f(filename, (std::fstream::in | std::fstream::binary));
    if (f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);
        str = new char[size + 1];
        if (!str)
        {
            f.close();
            return NULL;
        }
        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
        s = str;
        delete[] str;
        return 0;
    }
    printf("Error: Failed to open file %s\n", filename);
    return 1;
}

double run(int argv,char **args)
{
	cl_uint status;
    cl_platform_id platform;

    //创建平台对象
    status = clGetPlatformIDs(1, &platform, NULL);
    cl_device_id device;
    //创建 GPU 设备
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU,1,&device,NULL);
    
	if(strcmp(args[1],"-D")==0)
	{
		cl_device_fp_config DeviceDouble;
		clGetDeviceInfo(device, CL_DEVICE_DOUBLE_FP_CONFIG,sizeof(cl_device_fp_config), &DeviceDouble, NULL);
		if(DeviceDouble==0)
			cout <<"this device doesn't support double!"<<endl;
		else
			cout <<"this device support double type!"<<endl;
		return 0;
	}
	else
	{
		//创建context
		cl_context context = clCreateContext(NULL,1,&device,NULL, NULL, NULL);
		
		//创建命令队列
		cl_command_queue commandQueue = clCreateCommandQueue(context, device,CL_QUEUE_PROFILING_ENABLE, NULL);

		if (commandQueue == NULL) 
				perror("Failed to create commandQueue for device 0.");

		//建立要传入从机的数据
		/********  创建内核和内存对象 ********/

		int size=0;
		sscanf(args[2],"%d",&size);
		
		int Ndim = size;
		int Mdim = size;
		int Pdim = size;
		unsigned int szA = Ndim * Pdim;
		unsigned int szB = Pdim * Mdim;
		unsigned int szC = Ndim * Mdim;

		double *A;
		double *B;
		double *C;

		A = (double *)malloc(szA * sizeof(double));
		B = (double *)malloc(szB * sizeof(double));
		C = (double *)malloc(szC * sizeof(double));
		int i, j;
		for (i = 0; i < szA; i++)
			A[i] = (double)((double)i + 1.0);
		for (i = 0; i < szB; i++)
			B[i] = (double)((double)i + 1.0);

		//创建三个 OpenCL 内存对象，并把buf1 的内容通过隐式拷贝的方式
		//拷贝到clbuf1, buf2 的内容通过显示拷贝的方式拷贝到clbuf2
		cl_mem memObjects[3] = { 0, 0, 0 };
		memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY |  CL_MEM_COPY_HOST_PTR,
			sizeof(double)* szA, A, NULL);
		memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY |  CL_MEM_COPY_HOST_PTR,
			sizeof(double)* szB, B, NULL);
		memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(double)* szC, C, NULL);
		if (memObjects[0] == NULL || memObjects[1] == NULL ||memObjects[2] == NULL) 
			perror("Error in clCreateBuffer.\n");

	  // cout <<"create memory object successfully"<<endl;
	   
		std::string sourceStr;
		status = convertToString(args[1], sourceStr);
		if (status)
		{
			cout << args[1] << " can't be open !!!!!!!!" << endl;
			return 0;
		}
		const char * source = sourceStr.c_str();
		size_t sourceSize[] = { strlen(source) };
		//create program object
		cl_program program = clCreateProgramWithSource(context,1,&source,sourceSize,NULL);
	//	cout <<"create program successfully"<<endl;
		
		//compile program object
		status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
		if (status != 0)
		{
			printf("clBuild failed:%d\n", status);
			char tbuf[0x10000];
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0x10000, tbuf,
				NULL);
			printf("\n%s\n", tbuf);
		}
		//cout <<"create program object successfully"<<endl;
		
		//create Kernel object
		cl_kernel kernel = clCreateKernel(program, "matrix_mult", NULL);
		//cout <<"create kernel object successfully"<<endl;
		
		//set Kernel parameter
		cl_int clnum = NWITEMS;
		status = clSetKernelArg(kernel, 0, sizeof(int), &Ndim);
		status = clSetKernelArg(kernel, 1, sizeof(int), &Mdim);
		status = clSetKernelArg(kernel, 2, sizeof(int), &Pdim);
		status = clSetKernelArg(kernel, 3, sizeof(cl_mem), &memObjects[0]);
		status = clSetKernelArg(kernel, 4, sizeof(cl_mem), &memObjects[1]);
		status = clSetKernelArg(kernel, 5, sizeof(cl_mem), &memObjects[2]);
		if (status)
			cout << "error when parameter" << endl;
		//cout <<"create kernel parameter successfully"<<endl;
		
		//run kernel
		size_t global[2];
		cl_event prof_event;
		cl_ulong ev_start_time = (cl_ulong)0;
		cl_ulong ev_end_time = (cl_ulong)0;
		double run_time;
		global[0] = (size_t)Ndim;
		global[1] = (size_t)Mdim;
		size_t local[2]={32,32};
		
		status = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global, local, 0, NULL, &prof_event);
		if (status)
			cout << "error when run kernel" << endl;
		

		clWaitForEvents(1, &prof_event);
		
		//read time 
		status = clGetEventProfilingInfo(prof_event,CL_PROFILING_COMMAND_QUEUED,
			sizeof(cl_ulong),&ev_start_time,NULL);
		status = clGetEventProfilingInfo(prof_event,CL_PROFILING_COMMAND_END,
			sizeof(cl_ulong),&ev_end_time,NULL);
			
		if (status) 
			perror("error when read time\n");
		run_time = (double)(ev_end_time - ev_start_time);

		//data copy host memory
		status = clEnqueueReadBuffer(commandQueue, memObjects[2],CL_TRUE, 0,
				sizeof(double)* szC, C,0, NULL, NULL);
		if (status) 
			perror("error when read data\n");

		//show result
		if(argv>4)
		{
			printf("\nArray A:\n");
			for (i = 0; i < Ndim; i++) {
				for (j = 0; j < Pdim; j++)
					printf("%.3f\t", A[i*Pdim + j]);
				printf("\n");
			}
			printf("\nArray B:\n");
			for (i = 0; i < Pdim; i++) {
				for (j = 0; j < Mdim; j++)
					printf("%.3f\t", B[i*Mdim + j]);
				printf("\n");
			}
			printf("\nArray C:\n");
			for (i = 0; i < Ndim; i++) {
				for (j = 0; j < Mdim; j++)
					printf("%.3f\t", C[i*Mdim + j]);
				printf("\n");
			}
			cout << endl;
		}
		if (A)
			free(A);
		if (B)
			free(B);
		if (C)
			free(C);

		//delete OpenCL resource object
		clReleaseMemObject(memObjects[2]);
		clReleaseMemObject(memObjects[1]);
		clReleaseMemObject(memObjects[0]);
		clReleaseProgram(program);
		clReleaseCommandQueue(commandQueue);
		clReleaseContext(context);
		return run_time;
	}
}



int main(int argv, char** args)
{
	if(strcmp(args[1],"-D")==0)
	{
		run(argv,args);
		return 0;
	}
	double t=0;
	printf("metrics multiple calculate (%s,%s)*(%s,%s)\n",args[2],args[2],args[2],args[2]);
	cout <<"run time:"<<endl;
	int n=0;
	sscanf(args[3],"%d",&n);
	for(int i=1;i<=n;i++)
	{
		double t1=run(argv,args);
		t+=t1;
		cout <<t1/1000000.0<<" ";
		if(i%3==0)
			cout <<endl;
	}
	cout <<endl;
	t=t/n;
	printf("Execution time in milliseconds = %0.3f ms\n", (t/1000000.0));
    return 0;
}
