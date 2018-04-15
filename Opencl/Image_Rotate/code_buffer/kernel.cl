__kernel void rotate_image(
	const int W,const int H,
	const int dW,const int dH,
	const double cosrad, const double sinrad,
	__global const unsigned char* src,
	__global unsigned char* dst)
{
	const int x=get_global_id(0);
	const int y=get_global_id(1);
	
	int t1=0,t2=0;
	int rx=(x-dW/2)*cosrad+(y-dH/2)*sinrad;
	int ry=(y-dH/2)*cosrad-(x-dW/2)*sinrad;
	rx=rx+W/2;
	ry=ry+H/2;
	if(rx>=0&&rx<W&&ry>=0&&ry<H)
	{
		t1=y*(dW*3)+x*3;
		t2=ry*(W*3)+rx*3;
		dst[t1+0]=src[t2+0];
		dst[t1+1]=src[t2+1];
		dst[t1+2]=src[t2+2];
	}
}
