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
        for (k = 0; k < Pdim; k++)
            tmp += A[i*Pdim + k] * B[k*Mdim + j];
        C[i*Mdim + j] = tmp;
    }
}