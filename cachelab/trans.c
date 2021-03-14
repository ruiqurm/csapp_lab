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
const unsigned BLOCK_SIZE = 4;
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){
    int t1,t2,t3,t4,t5,t6,t7,t8;
    int i,j;
    int k,l;
    if (M==N&&M==32){
        for(j=0;j<M;j+=8)
            for(i=0;i<N;i++){
                t1 = A[i][j];
                t2 = A[i][j+1];
                t3 = A[i][j+2];
                t4 = A[i][j+3];
                t5 = A[i][j+4];
                t6 = A[i][j+5];
                t7 = A[i][j+6];
                t8 = A[i][j+7];
                B[j][i] = t1;
                B[j+1][i] = t2;
                B[j+2][i] = t3;
                B[j+3][i] = t4;
                B[j+4][i] = t5;
                B[j+5][i] = t6;
                B[j+6][i] = t7  ;
                B[j+7][i] = t8;
            }
        }
    else if (M==N&&M==64){
        for(i=0;i<N;i+=8){
            for(j=0;j<M;j+=8){
                for(k=0;k<4;k++){
                    t1 = A[i+k][j];
                    t2 = A[i+k][j+1];
                    t3 = A[i+k][j+2];
                    t4 = A[i+k][j+3];
                    t5 = A[i+k][j+4];
                    t6 = A[i+k][j+5];
                    t7 = A[i+k][j+6];
                    t8 = A[i+k][j+7];
                    B[j][i+k] = t1;
                    B[j+1][i+k] = t2;
                    B[j+2][i+k] = t3;
                    B[j+3][i+k] = t4;
                    // 暂存，竖着存
                    B[j][i+k+4] = t5;
                    B[j+1][i+k+4] = t6;
                    B[j+2][i+k+4] = t7;
                    B[j+3][i+k+4] = t8;
                }
            for(k=0;k<4;k++){
                    t5 = B[j+k][i+4];
                    t6 = B[j+k][i+5];
                    t7 = B[j+k][i+6];
                    t8 = B[j+k][i+7];

                    t1 = A[i+4][j+k];
                    t2 = A[i+5][j+k];
                    t3 = A[i+6][j+k];
                    t4 = A[i+7][j+k];
                    B[j+k][i+4] = t1;
                    B[j+k][i+5] = t2;
                    B[j+k][i+6] = t3;
                    B[j+k][i+7] = t4;

                    B[j+4+k][i] = t5;
                    B[j+4+k][i+1] = t6;
                    B[j+4+k][i+2] = t7;
                    B[j+4+k][i+3] = t8;
                }
                for(k=4;k<8;k++)
                    for(l=4;l<8;l++){
                        B[j+l][i+k] = A[i+k][j+l];
                    }
            }
        }
    }else{
         for(j=0;j<M;j+=16){
            for(i=0;i<N;i++){
                    t1 = A[i][j];
                    t2 = A[i][j+1];
                    t3 = A[i][j+2];
                    t4 = A[i][j+3];
                    t5 = A[i][j+4];
                    t6 = A[i][j+5];
                    t7 = A[i][j+6];
                    t8 = A[i][j+7];
                    B[j][i] = t1;
                    B[j+1][i] = t2;
                    B[j+2][i] = t3;
                    B[j+3][i] = t4;
                    B[j+4][i] = t5;
                    B[j+5][i] = t6;
                    B[j+6][i] = t7;
                    B[j+7][i] = t8;
                    t1 = A[i][j+8];
                    t2 = A[i][j+9];
                    t3 = A[i][j+10];
                    t4 = A[i][j+11];
                    t5 = A[i][j+12];
                    t6 = A[i][j+13];
                    t7 = A[i][j+14];
                    t8 = A[i][j+15];
                    B[j+8][i] = t1;
                    B[j+9][i] = t2;
                    B[j+10][i] = t3;
                    B[j+11][i] = t4;
                    B[j+12][i] = t5;
                    B[j+13][i] = t6;
                    B[j+14][i] = t7;
                    B[j+15][i] = t8;
                    // t1 = A[i][j+16];
                    // t2 = A[i][j+17];
                    // t3 = A[i][j+18];
                    // t4 = A[i][j+19];
                    // t5 = A[i][j+20];
                    // t6 = A[i][j+21];
                    // t7 = A[i][j+22];
                    // t8 = A[i][j+23];
                    // B[j+16][i] = t1;
                    // B[j+17][i] = t2;
                    // B[j+18][i] = t3;
                    // B[j+19][i] = t4;
                    // B[j+20][i] = t5;
                    // B[j+21][i] = t6;
                    // B[j+22][i] = t7;
                    // B[j+23][i] = t8;

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
void trans(int M, int N, int A[N][M], int B[M][N])
{
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
void registerFunctions()
{
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
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
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

