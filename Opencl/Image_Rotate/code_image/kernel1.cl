__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|
						CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP; 
__kernel void rotate_image(
	const int W,const int H,
	const int dW,const int dH,
	const double cosrad, const double sinrad,
	__read_only image2d_t srcImage,
	__write_only image2d_t dstImage)
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
		float4 value=read_imagef(srcImage,sampler,(int2)(rx,ry));
		write_imagef(dstImage,(int2)(x,y),value);
	}
}
