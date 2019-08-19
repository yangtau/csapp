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

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
static inline int min(int x, int y) {
    return x > y ? y : x;
}
char transpose_submit_desc[] = "Transpose submission";
// line size is 64bytes typically
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    const int col_len = 8, row_len = 4;
    for (int i = 0; i < N; i += col_len) {
        int i_bound = min(i + col_len, N);
        for (int j = 0; j < M; j += row_len) {
            int j_bound = min(j + row_len, M);
            if (i+4>i_bound) {
                for (int k = j; k < j_bound; k++) {
                    for (int s = i; s < i_bound; s++) {
                        B[k][s] = A[s][k];
                    }
                }
                continue;
            }
            for (int k = j; k < j_bound; k++) {
                for (int s = i; s < i+4; s++) {
                    B[k][s] = A[s][k];
                }
            }
            for (int k = j; k < j_bound; k++) {
                for (int s = i+4; s < i_bound; s++) {
                    B[k][s] = A[s][k];
                }
            }
        }
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
    registerTransFunction(trans, trans_desc);
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
