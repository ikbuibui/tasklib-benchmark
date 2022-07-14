#include <chrono>
#include <iostream>
#include "sg/superglue.hpp"
#include "common.h"

#include <cblas.h>
#include <lapacke.h>

using namespace std::chrono;

struct Options : public DefaultOptions<Options> {};

struct potrf : public Task<Options, 1> {
    double * a;

    potrf(double * A, Handle<Options> &h)
        : a(A)
    {
        register_access(ReadWriteAdd::write, h);
    }

    void run()
    {
        LAPACKE_dpotrf(LAPACK_COL_MAJOR, 'L', blksz, a, blksz);
    }
};

struct gemm : public Task<Options, 3> {
    double const * a;
    double const * b;
    double * c;

    gemm(double const * A, double const * B, double * C,
         Handle<Options> &hA, Handle<Options> &hB, Handle<Options> &hC)
        : a(A), b(B), c(C)
    {
        register_access(ReadWriteAdd::read, hA);
        register_access(ReadWriteAdd::read, hB);
        register_access(ReadWriteAdd::write, hC);
    }

    void run()
    {
        cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans,
                    blksz, blksz, blksz, -1.0, a, blksz, b, blksz, 1.0, c, blksz);
    }
};

struct trsm : public Task<Options, 2> {
    double const * a;
    double * b;

    trsm(double const * A, double * B,
         Handle<Options> &hA, Handle<Options> &hB)
        : a(A), b(B)
    {
        register_access(ReadWriteAdd::read, hA);
        register_access(ReadWriteAdd::write, hB);
    }

    void run()
    {
        cblas_dtrsm(CblasColMajor,
                    CblasRight, CblasLower, CblasTrans, CblasNonUnit,
                    blksz, blksz, 1.0, a, blksz, b, blksz);
    }
};

struct syrk : public Task<Options, 2>
{
    double const * a;
    double * c;
    syrk(double const * A, double * C,
         Handle<Options> &hA, Handle<Options> &hC)
        : a(A), c(C)
    {
        register_access(ReadWriteAdd::read, hA);
        register_access(ReadWriteAdd::write, hC);
    }

    void run() {
        cblas_dsyrk(CblasColMajor, CblasLower, CblasNoTrans,
                    blksz, blksz, -1.0, a, blksz, 1.0, c, blksz);
    }
};

int main(int argc, char *argv[]) {
    read_args(argc, argv);
    SuperGlue<Options> tm(n_workers);

    // initialize tiled matrix in column-major layout
    std::vector<double*> A(nblks * nblks);

    // allocate each tile (also in column-major layout)
    for(size_t j = 0; j < nblks; ++j)
        for(size_t i = 0; i < nblks; ++i)
            A[j * nblks + i] = new double[blksz * blksz];

    Handle<Options> *h = new Handle<Options>[nblks * nblks];

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
                tm.submit(new gemm(A[k * nblks + i], A[k * nblks + j], A[j * nblks + i],
                                   h[k * nblks + i], h[k * nblks + j], h[j * nblks + i]) );
            }
        }

        for(size_t i = 0; i < j; i++)
        {
            tm.submit(new syrk(A[i * nblks + j], A[j * nblks + j],
                                h[i * nblks + j], h[j* nblks + j]));
        }

        tm.submit(new potrf(A[j * nblks + j], h[j * nblks + j]));

        for(size_t i = j + 1; i < nblks; i++)
        {
            tm.submit(new trsm(A[j*nblks + j], A[j * nblks + i],
                               h[j * nblks + j], h[j * nblks + i]));
        }
    }

    tm.barrier();
    
    auto end = high_resolution_clock::now();

    std::cout << "total " << duration_cast<nanoseconds>(end-start).count()/1000000.0 << " ms" << std::endl;

    return 0;
}
