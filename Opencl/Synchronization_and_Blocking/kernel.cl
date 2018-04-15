__kernel void kernel_test1(__global int *pDst,__global int *pSrc1,__global int *pSrc2)
{
	int index=get_global_id(0);
	pDst[index]=pSrc1[index]+pSrc2[index];
}

__kernel void kernel_test2(__global int *pDst,__global int *pSrc1,__global int *pSrc2)
{
	int index=get_global_id(0);
	pDst[index]=pDst[index]*pSrc1[index]-pSrc2[index];
}