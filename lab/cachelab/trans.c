/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_8x8_block(int M, int N, int A[N][M], int B[M][N]);
void transpose_copy_block(int M, int N, int A[N][M], int B[M][N]);
void trans_32x32(int A[32][32], int B[32][32]);
void trans_64x64(int M, int N, int A[N][M], int B[M][N]);
void transpose_61x67(int M, int N, int A[N][M], int B[M][N]);

void show_matrix(int M, int N, int A[M][N], int block_size) {
    for (int i = 0; i < M; i++) {
        if (i % block_size == 0) {
            for (int k = 0; k < N + (M + block_size - 1) / block_size + 1; k++)
                printf("-");
            printf("\n");
        }
        printf("|");
        for (int j = 0; j < N; j++) {
            printf("%x%s", A[i][j] % (1 << 4),
                   ((j + 1) % block_size == 0) ? "|" : "");
        }
        printf("\n");
    }
    for (int k = 0; k < N + (M + block_size - 1) / block_size; k++) printf("-");
    printf("\n");
}

static inline int min(int x, int y) {
    return x > y ? y : x;
}
/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */

char transpose_submit_desc[] = "Transpose submission";
// line size is 64bytes typically
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    if (M == 32 && N == 32)
        return trans_32x32(A, B);
    else if (M == 64 && N == 64)
        return trans_64x64(M, N, A, B);
    else if (M == 61 || N == 61) {
        return transpose_61x67(M, N, A, B);
    }
    transpose_8x8_block(M, N, A, B);
}

char transpose_64x64_desc[] = "64x64";
void trans_64x64(int M, int N, int A[N][M], int B[M][N]) {
    int i = 0, j = 0, k = 0;
    int tmp;
    int a0, a1, a2, a3;
    int a4, a5, a6, a7;
    const int block_size = 8;
    // show_matrix(N, M, A, block_size / 2);
    for (i = 0; i < N; i += block_size) {
        for (j = 0; j < M; j += block_size) {
            for (k = 0; k < block_size / 2; k++) {
                // A top left
                a0 = A[k + i][j];
                a1 = A[k + i][j + 1];
                a2 = A[k + i][j + 2];
                a3 = A[k + i][j + 3];

                // copy
                // A top right
                a4 = A[k + i][j + 4];
                a5 = A[k + i][j + 5];
                a6 = A[k + i][j + 6];
                a7 = A[k + i][j + 7];

                // B top left
                B[j][k + i] = a0;
                B[j + 1][k + i] = a1;
                B[j + 2][k + i] = a2;
                B[j + 3][k + i] = a3;

                // copy
                // B top right
                B[j + 0][k + 4 + i] = a4;
                B[j + 1][k + 4 + i] = a5;
                B[j + 2][k + 4 + i] = a6;
                B[j + 3][k + 4 + i] = a7;
            }
            for (k = 0; k < block_size / 2; k++) {
                // step 1 2
                a0 = A[i + 4][j + k], a4 = A[i + 4][j + k + 4];
                a1 = A[i + 5][j + k], a5 = A[i + 5][j + k + 4];
                a2 = A[i + 6][j + k], a6 = A[i + 6][j + k + 4];
                a3 = A[i + 7][j + k], a7 = A[i + 7][j + k + 4];
                // step 3
                tmp = B[j + k][i + 4], B[j + k][i + 4] = a0, a0 = tmp;
                tmp = B[j + k][i + 5], B[j + k][i + 5] = a1, a1 = tmp;
                tmp = B[j + k][i + 6], B[j + k][i + 6] = a2, a2 = tmp;
                tmp = B[j + k][i + 7], B[j + k][i + 7] = a3, a3 = tmp;
                // step 4
                B[j + k + 4][i + 0] = a0, B[j + k + 4][i + 4 + 0] = a4;
                B[j + k + 4][i + 1] = a1, B[j + k + 4][i + 4 + 1] = a5;
                B[j + k + 4][i + 2] = a2, B[j + k + 4][i + 4 + 2] = a6;
                B[j + k + 4][i + 3] = a3, B[j + k + 4][i + 4 + 3] = a7;
            }
        }
    }
    // show_matrix(M, N, B, block_size / 2);
}

char transpose_64x64_plus_desc[] = "64x64";
void trans_64x64_plus(int M, int N, int A[N][M], int B[M][N]) {
    int i = 0, j = 0, k = 0, s = 0;
    int tmp;
    int a0, a1, a2, a3;
    int a4, a5, a6, a7;
    const int block_size = 8;
    // show_matrix(N, M, A, 4);
    // for (i = 0; i < N; i += 1) {
    //     for (j = 0; j < M; j += 1) {
    //         B[j][i] = 0;
    //     }
    // }
    for (i = 0; i < N; i += block_size) {
        for (j = 0; j < M; j += block_size) {
            for (k = 0; k < block_size / 2; k++) {
                // A top left
                a0 = A[k + i][j];
                a1 = A[k + i][j + 1];
                a2 = A[k + i][j + 2];
                a3 = A[k + i][j + 3];

                // copy
                // A top right
                a4 = A[k + i][j + 4];
                a5 = A[k + i][j + 5];
                a6 = A[k + i][j + 6];
                a7 = A[k + i][j + 7];

                B[j + k][i + 0] = a0;
                B[j + k][i + 1] = a1;
                B[j + k][i + 2] = a2;
                B[j + k][i + 3] = a3;
                B[j + k][i + 4] = a4;
                B[j + k][i + 5] = a5;
                B[j + k][i + 6] = a6;
                B[j + k][i + 7] = a7;
            }
            // show_matrix(M, N, B, 4);
            for (k = 0; k < 4; k++) {
                for (s = k + 1; s < 4; s++) {
                    tmp = B[j + k][i + s], B[j + k][i + s] = B[j + s][i + k],
                    B[j + s][i + k] = tmp;
                    tmp = B[j + k][i + 4 + s],
                    B[j + k][i + 4 + s] = B[j + s][i + 4 + k],
                    B[j + s][i + 4 + k] = tmp;
                }
            }
            // show_matrix(M, N, B, 4);

            for (k = 0; k < block_size / 2; k++) {
                // step 1 2
                a0 = A[i + 4][j + k], a4 = A[i + 4][j + k + 4];
                a1 = A[i + 5][j + k], a5 = A[i + 5][j + k + 4];
                a2 = A[i + 6][j + k], a6 = A[i + 6][j + k + 4];
                a3 = A[i + 7][j + k], a7 = A[i + 7][j + k + 4];
                // step 3
                tmp = B[j + k][i + 4], B[j + k][i + 4] = a0, a0 = tmp;
                tmp = B[j + k][i + 5], B[j + k][i + 5] = a1, a1 = tmp;
                tmp = B[j + k][i + 6], B[j + k][i + 6] = a2, a2 = tmp;
                tmp = B[j + k][i + 7], B[j + k][i + 7] = a3, a3 = tmp;
                // step 4
                B[j + k + 4][i + 0] = a0, B[j + k + 4][i + 4 + 0] = a4;
                B[j + k + 4][i + 1] = a1, B[j + k + 4][i + 4 + 1] = a5;
                B[j + k + 4][i + 2] = a2, B[j + k + 4][i + 4 + 2] = a6;
                B[j + k + 4][i + 3] = a3, B[j + k + 4][i + 4 + 3] = a7;
            }
        }
    }
}

void transpose_61x67(int M, int N, int A[N][M], int B[M][N]) {
    int i = 0, j = 0, k = 0, s = 0;
    int a0, a1, a2, a3, a4, a5, a6, a7;
    // const int len = 8;
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 23) {
            if (i + 8 <= N && j + 23 <= M) {
                for (s = j; s < j + 23; s++) {
                    a0 = A[i][s];
                    a1 = A[i + 1][s];
                    a2 = A[i + 2][s];
                    a3 = A[i + 3][s];
                    a4 = A[i + 4][s];
                    a5 = A[i + 5][s];
                    a6 = A[i + 6][s];
                    a7 = A[i + 7][s];
                    B[s][i + 0] = a0;
                    B[s][i + 1] = a1;
                    B[s][i + 2] = a2;
                    B[s][i + 3] = a3;
                    B[s][i + 4] = a4;
                    B[s][i + 5] = a5;
                    B[s][i + 6] = a6;
                    B[s][i + 7] = a7;
                }
            } else {
                for (k = i; k < min(i + 8, N); k++) {
                    for (s = j; s < min(j + 23, M); s++) {
                        B[s][k] = A[k][s];
                    }
                }
            }
        }
    }
}

void trans_32x32(int A[32][32], int B[32][32]) {
    int i = 0, j = 0, k = 0;
    int a0, a1, a2, a3, a4, a5, a6, a7;
    for (i = 0; i < 32; i += 8) {
        for (j = 0; j < 32; j += 8) {
            for (k = i; k < i + 8; k++) {
                a0 = A[k][j];
                a1 = A[k][j + 1];
                a2 = A[k][j + 2];
                a3 = A[k][j + 3];
                a4 = A[k][j + 4];
                a5 = A[k][j + 5];
                a6 = A[k][j + 6];
                a7 = A[k][j + 7];
                B[j][k] = a0;
                B[j + 1][k] = a1;
                B[j + 2][k] = a2;
                B[j + 3][k] = a3;
                B[j + 4][k] = a4;
                B[j + 5][k] = a5;
                B[j + 6][k] = a6;
                B[j + 7][k] = a7;
            }
        }
    }
}

char transpose_by_copy_desc[] = "copy and then trans";
void transpose_copy_block(int M, int N, int A[N][M], int B[M][N]) {
    // 32 x 32
    int i = 0, j = 0, k = 0, s = 0;
    int a0, a1, a2, a3, a4, a5, a6, a7;
    const int len = 8;
    for (i = 0; i < N; i += len) {
        for (j = 0; j < N; j += len) {
            // copy
            for (k = i, s = j; k < i + len; k++, s++) {
                a0 = A[k][j];
                a1 = A[k][j + 1];
                a2 = A[k][j + 2];
                a3 = A[k][j + 3];
                a4 = A[k][j + 4];
                a5 = A[k][j + 5];
                a6 = A[k][j + 6];
                a7 = A[k][j + 7];
                B[s][i] = a0;
                B[s][i + 1] = a1;
                B[s][i + 2] = a2;
                B[s][i + 3] = a3;
                B[s][i + 4] = a4;
                B[s][i + 5] = a5;
                B[s][i + 6] = a6;
                B[s][i + 7] = a7;
            }
            // transpose
            // show_matrix(N, M, A, len);

            // show_matrix(M, N, B, len);
            for (k = 0; k < len; k++) {
                for (s = k + 1; s < len; s++) {
                    a0 = B[k + j][s + i];
                    B[k + j][s + i] = B[s + j][k + i];
                    B[s + j][k + i] = a0;
                }
            }
            // printf("\n");
        }
    }
}

char transpose_8x8_block_desc[] = "8x8 block";
void transpose_8x8_block(int M, int N, int A[N][M], int B[M][N]) {
    printf("A:%lx, B:%lx", (long)A, (long)B);
    int i = 0, j = 0, k = 0, s = 0;
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            for (k = i; k < min(i + 8, N); k++) {
                for (s = j; s < min(j + 8, M); s++) {
                    B[s][k] = A[k][s];
                }
            }
        }
    }
}
char transpose_diagonal_desc[] = "diagonal first";
void transpose_diagonal(int M, int N, int A[N][M], int B[M][N]) {
    // printf("A:%lx, B:%lx", (long)A, (long)B);
    int i = 0, j = 0, k = 0, s = 0;
    for (i = 0; i < N; i += 8)
        for (j = 0; j < M; j += 8)
            for (k = 0; k < 8; k++) {
                for (s = k; s < 8; s++) B[k + j][s + i] = A[i + s][j + k];
                for (s = k - 1; s >= 0; s--) B[k + j][s + i] = A[i + s][j + k];
            }
}
/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc);
    registerTransFunction(trans_64x64_plus, transpose_64x64_plus_desc);
    // registerTransFunction(transpose_8x8_block, transpose_8x8_block_desc);
    // registerTransFunction(transpose_copy_block, transpose_by_copy_desc);
    // registerTransFunction(transpose_diagonal, transpose_diagonal_desc);
    // registerTransFunction(trans_64x64, transpose_64x64_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
