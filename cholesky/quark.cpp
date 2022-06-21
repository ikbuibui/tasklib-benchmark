#include <chrono>
#include <iostream>
#include <cstring>
#include "quark.h"

using namespace std::chrono;

double *A;

int n_threads = 4;
int DIM = 5;
microseconds delay(10);

void sleep()
{
    auto end = std::chrono::high_resolution_clock::now() + delay;
    while(std::chrono::high_resolution_clock::now() < end);
}

void potrf(Quark *quark) { sleep(); }
void trsm(Quark *quark) { sleep(); }
void syrk(Quark *quark) { sleep(); }
void gemm(Quark *quark) { sleep(); }

int main(int argc, char *argv[]) {
    if (argc > 1)
        DIM = atoi(argv[1]);
    if (argc > 2)
        delay = microseconds(atoi(argv[2]));
    if (argc > 3)
        n_threads = atoi(argv[3]);

    Quark *quark = QUARK_New(n_threads);
    A = (double*) malloc(DIM*DIM*sizeof(double));

    auto start = high_resolution_clock::now();

    int k, m, n;
    for (k = 0; k < DIM; ++k) {
        QUARK_Insert_Task(quark, potrf, NULL, 
                          1*sizeof(double), &A[k*DIM+k], INOUT,
                          0);

        for (m = k+1; m < DIM; ++m) {
            QUARK_Insert_Task(quark, trsm, NULL, 
                              1*sizeof(double), &A[k*DIM+k], INPUT,
                              1*sizeof(double), &A[m*DIM+k], INOUT,
                              0);
        }

        for (m = k+1; m < DIM; ++m) {
            QUARK_Insert_Task(quark, syrk, NULL, 
                              1*sizeof(double), &A[m*DIM+k], INPUT,
                              1*sizeof(double), &A[m*DIM+m], INOUT,
                              0);

            for (n = k+1; n < m; ++n) {
                QUARK_Insert_Task(quark, gemm, NULL, 
                                  1*sizeof(double), &A[m*DIM+k], INPUT,
                                  1*sizeof(double), &A[n*DIM+k], INPUT,
                                  1*sizeof(double), &A[m*DIM+n], INOUT,
                                  0);
            }
        }
    }
    
    QUARK_Waitall(quark);
    auto end = high_resolution_clock::now();

    std::cout << "total " << duration_cast<nanoseconds>(end-start).count()/1000000.0 << " ms" << std::endl;
    
    QUARK_Delete(quark);    
    
    return 0;
}
