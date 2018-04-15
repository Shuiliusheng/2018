__kernel void matrix_mult(
    const int Ndim,
    const int Mdim,
    const int Pdim,
    __global const double* A, 
    __global const double* B, 
    __global double* C)
{
    int i = get_global_id(0);
    int j = get_global_id(1);

    int k;
    double tmp;

    if ((i < Ndim) && (j < Mdim)) {
        tmp = 0.0;
		int temp1=i*Pdim;
		int temp2=j;
        for (k = 0; k < Pdim; k++)
		{
			tmp += A[temp1 + k] * B[temp2];
			temp2=temp2+Mdim;
		}
        C[i*Mdim + j] = tmp;
    }
}