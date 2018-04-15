__kernel void matrix_mult(
    const int Ndim,
    const int Mdim,
    const int Pdim,
    __global const double* A, 
    __global const double* B, 
    __global double* C)
{
    const int row=get_local_id(0);
	const int col=get_local_id(1);
	const int g_row=32*get_group_id(0)+row;
	const int g_col=32*get_group_id(1)+col;
	
	__local double Asub[32][32];
	__local double Bsub[32][32];
	
	double acc=0.0f;
	
	const int num_tiles=Pdim/32;
	
	for(int t=0;t<num_tiles;t++)
	{
		const int temp=32*t;
		const int tile_row=temp+row;
		const int tile_col=temp+col;
		Asub[row][col]=A[g_row*Pdim+tile_col];
		Bsub[row][col]=B[tile_row*Mdim+g_col];
		
		barrier(CLK_LOCAL_MEM_FENCE);
		
		for(int k=0;k<32;k++)
		{
			acc+=Asub[row][k]*Bsub[k][col];
		}
		//it will calculate for many times and
		//then add those result together
		//
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	C[g_row*Mdim+g_col]=acc;
}
