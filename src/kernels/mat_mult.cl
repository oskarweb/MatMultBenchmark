#define DataType float

__kernel void naive_mat_mult(
    __global DataType *A,
    __global DataType *B,
    __global DataType *C,
    const uint M,
    const uint N,
    const uint K)
{
    uint row = get_global_id(0);
    uint col = get_global_id(1);

    if (row < M && col < K) {
        DataType sum = 0;
        for (int i = 0; i < N; i++) {
            sum += A[row * N + i] * B[i * K + col];
        }
        C[row * K + col] = sum;
    }
}