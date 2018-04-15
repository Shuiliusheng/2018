#include<stdio.h>
#include<iostream>
#include<CL/cl.h>
#include<string.h>
using namespace std;

void checkErr(cl_int err,int num)
{
	if(CL_SUCCESS != err)
		printf("OpenCL error(%d) at %d\n",err,num-1);
}

int main()
{
	cl_device_id *device;
	cl_platform_id *platform;
    cl_uint num_platform;
	cl_int err;
	cl_uint numDevice;
	
	//choose the first platform
	
	 err = clGetPlatformIDs(0, NULL, &num_platform);
    platform = (cl_platform_id *)malloc(sizeof(cl_platform_id) * num_platform);
    err = clGetPlatformIDs(num_platform, platform, NULL);
	cout <<"the platform number is: "<<num_platform<<endl<<endl;
	for (int j = 0; j < num_platform; j++)
	{
		// get platform name
		size_t size;
        err = clGetPlatformInfo(platform[j], CL_PLATFORM_NAME, 0, NULL, &size);
        char *name = (char *)malloc(size);
        err = clGetPlatformInfo(platform[j], CL_PLATFORM_NAME, size, name, NULL);
        cout <<"the platform name is:"<<name<<endl<<endl;
		
		err=clGetDeviceIDs(platform[j],CL_DEVICE_TYPE_ALL,0,NULL,&numDevice);
		checkErr(err,1);
		device=(cl_device_id *)malloc(sizeof(cl_device_id)*numDevice);
		
		//choose GPU
		err = clGetDeviceIDs(platform[j],CL_DEVICE_TYPE_ALL,numDevice,device,NULL);
		checkErr(err, 1 );
		
		for(int i=0;i<numDevice;i++)
		{
			//check device name
			char buffer[100];
			err=clGetDeviceInfo(device[i],CL_DEVICE_NAME,100,buffer,NULL);
			checkErr(err,1);
			cout <<"Device name is:"<<buffer<<endl<<endl;
			
			//check the max calculate units number
			cl_uint unitnum;
			err=clGetDeviceInfo(device[i],CL_DEVICE_MAX_COMPUTE_UNITS,sizeof(cl_uint),&unitnum,NULL);
			checkErr(err,1);
			cout <<"computer units number is :"<<unitnum<<endl<<endl;
			
			//check the frequency of core
			cl_uint frequency;
			err=clGetDeviceInfo(device[i],CL_DEVICE_MAX_CLOCK_FREQUENCY,sizeof(cl_uint),&frequency,NULL);
			checkErr(err,1);
			cout <<"device frequency is:"<<frequency<<endl<<endl;
			
			//check the global memory size of device
			cl_ulong globalsize;
			err=clGetDeviceInfo(device[i],CL_DEVICE_GLOBAL_MEM_SIZE,sizeof(cl_ulong),&globalsize,NULL);
			checkErr(err,1);
			cout <<"device global memory size is:"<<globalsize/1024.0/1024.0<<" MB"<<endl<<endl;
			
			//check the global cache lines
			cl_uint globalcacheline;
			err=clGetDeviceInfo(device[i],CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,sizeof(cl_uint),&globalcacheline,NULL);
			checkErr(err,1);
			cout <<"device global cacheline size is:"<<globalcacheline<<" byte"<<endl<<endl;
			
			//check the version of opencl
			char deviceversion[100];
			err=clGetDeviceInfo(device[i],CL_DEVICE_VERSION,100,deviceversion,NULL);
			checkErr(err,1);
			cout <<"Device version for opencl is:"<<deviceversion<<endl<<endl;
			
			//check the extension name of device
			char *deviceExtension;
			cl_ulong ExtenNum;
			err=clGetDeviceInfo(device[i],CL_DEVICE_EXTENSIONS,0,NULL,&ExtenNum);
			checkErr(err,1);
			deviceExtension=(char *)malloc(ExtenNum);
			err=clGetDeviceInfo(device[i],CL_DEVICE_EXTENSIONS,ExtenNum,deviceExtension,NULL);
			checkErr(err,1);
			
			for(int i=0;i<strlen(deviceExtension);i++)
			{
				if(deviceExtension[i]==' ')
					deviceExtension[i]='\n';
			}
			cout <<"device extension name is:"<<deviceExtension<<endl<<endl;
			
			free(deviceExtension);
		}
	}
	free(device);
	return 0;
	
}