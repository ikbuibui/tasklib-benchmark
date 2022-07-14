#include <chrono>
#include <iostream>
#include <cstring>
#include <vector>
#include <cblas.h>
#include <lapacke.h>

#include "quark.h"
#include "common.h"

using namespace std::chrono;

void potrf(Quark *quark)
{
    double * a;
    quark_unpack_args_1( quark, a );

    LAPACKE_dpotrf(LAPACK_COL_MAJOR, 'L', blksz, a, blksz);
}

void trsm(Quark *quark)
{
    double const * a;
    double * b;
    quark_unpack_args_2( quark, a, b );

    cblas_dtrsm(CblasColMajor,
                CblasRight, CblasLower, CblasTrans, CblasNonUnit,
                blksz, blksz, 1.0, a, blksz, b, blksz);
}

void syrk(Quark *quark)
{
    double const * a;
    double * c;
    quark_unpack_args_2( quark, a, c );

    cblas_dsyrk(CblasColMajor, CblasLower, CblasNoTrans,
                blksz, blksz, -1.0, a, blksz, 1.0, c, blksz);
}

void gemm(Quark *quark)
{
    double const * a;
    double const * b;
    double * c;
    quark_unpack_args_3( quark, a, b, c );

    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans,
                blksz, blksz, blksz, -1.0, a, blksz, b, blksz, 1.0, c, blksz);
}

int main(int argc, char *argv[]) {
    read_args(argc, argv);
    Quark *quark = QUARK_New(n_workers);

    // initialize tiled matrix in column-major layout
    std::vector<double*> A(nblks * nblks);

    // allocate each tile (also in column-major layout)
    for(size_t j = 0; j < nblks; ++j)
        for(size_t i = 0; i < nblks; ++i)
            A[j * nblks + i] = new double[blksz * blksz];
    /* ia: row of outer matrix
       ib: row of inner matrix
       ja: col of outer matrix
       jb: col of inner matrix */
    double * Alin = init_matrix();
    for(size_t ia = 0; ia < nblks; ++ia)
        for(size_t ib = 0; ib < blksz; ++ib)
            for(size_t ja = 0; ja < nblks; ++ja)
                for(size_t jb = 0; jb < blksz; ++jb)
                    (A[ja * nblks + ia])[jb * blksz + ib] = Alin[(ia * blksz + ib) + (ja * blksz + jb) * N];
    
    auto start = high_resolution_clock::now();

    // calculate cholesky decomposition
    for(size_t j = 0; j < nblks; j++)
    {
        for(size_t k = 0; k < j; k++)
        {
            for(size_t i = j + 1; i < nblks; i++)
            {
                QUARK_Insert_Task(quark, gemm, NULL, 
                                  1*sizeof(double*), A[k * nblks + i], INPUT,
                                  1*sizeof(double*), A[k * nblks + j], INPUT,
                                  1*sizeof(double*), A[j * nblks + i], INOUT,
                                  0);
            }
        }

        for(size_t i = 0; i < j; i++)
        {
            QUARK_Insert_Task(quark, syrk, NULL, 
                              1*sizeof(double*), A[i * nblks + j], INPUT,
                              1*sizeof(double*), A[j * nblks + j], INOUT,
                              0);
        }

        QUARK_Insert_Task(quark, potrf, NULL, 
                          1*sizeof(double*), A[j*nblks+j], INOUT,
                          0);

        for(size_t i = j + 1; i < nblks; i++)
        {
            QUARK_Insert_Task(quark, trsm, NULL, 
                              1*sizeof(double*), A[j * nblks + j], INPUT,
                              1*sizeof(double*), A[j * nblks + i], INOUT,
                              0);
        }
    }
   
    QUARK_Waitall(quark);
    auto end = high_resolution_clock::now();

    std::cout << "total " << duration_cast<nanoseconds>(end-start).count()/1000000.0 << " ms" << std::endl;
    
    QUARK_Delete(quark);    
    
    return 0;
}
