__kernel void naive_mat_mult(
    __global float* A,
    __global float* B,
    __global float* C,
    const int M,
    const int N,
    const int K)
{
    int row = get_global_id(0);
    int col = get_global_id(1);

    if (row < M && col < K) {
        float sum = 0.0f;
        for (int i = 0; i < N; i++) {
            sum += A[row * N + i] * B[i * K + col];
        }
        C[row * K + col] = sum;
    }
}