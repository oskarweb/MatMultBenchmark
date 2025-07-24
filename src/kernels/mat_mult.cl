#ifndef T
#define T float
#endif

__kernel void naive_mat_mult(
    __global T *A,
    __global T *B,
    __global T *C,
    const uint M,
    const uint N,
    const uint K)
{
    uint row = get_global_id(0);
    uint col = get_global_id(1);

    if (row < M && col < K) {
        T sum = 0;
        for (int i = 0; i < N; i++) {
            sum += A[row * N + i] * B[i * K + col];
        }
        C[row * K + col] = sum;
    }
}