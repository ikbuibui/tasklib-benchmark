#include <chrono>
#include <redGrapes/redGrapes.hpp>
#include <redGrapes/resource/ioresource.hpp>
#include "common.h"

#include <iomanip>
#include <cblas.h>
#include <lapacke.h>

using namespace std::chrono;

int main(int argc, char *argv[]) {
    read_args(argc, argv);
    auto rg = redGrapes::init(n_workers);
    using TTask = decltype(rg)::RGTask;
    // initialize tiled matrix in column-major layout
    std::vector<redGrapes::IOResource<double*,TTask>> A(nblks * nblks);

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
                    (*A[ja * nblks + ia])[jb * blksz + ib] = Alin[(ia * blksz + ib) + (ja * blksz + jb) * N];

    auto start = high_resolution_clock::now();

    // calculate cholesky decomposition
    for(size_t j = 0; j < nblks; j++)
    {
        for(size_t k = 0; k < j; k++)
        {
            for(size_t i = j + 1; i < nblks; i++)
            {
                // A[i,j] = A[i,j] - A[i,k] * (A[j,k])^t
                rg.emplace_task(
                    [](auto a, auto b, auto c)
                    {
                        cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans,
                                    blksz, blksz, blksz, -1.0, *a, blksz, *b, blksz, 1.0, *c, blksz);
                    },
                    A[k * nblks + i].read(),
                    A[k * nblks + j].read(),
                    A[j * nblks + i].write());
            }
        }

        for(size_t i = 0; i < j; i++)
        {
            // A[j,j] = A[j,j] - A[j,i] * (A[j,i])^t
            rg.emplace_task(
                [](auto a, auto c)
                {
                    cblas_dsyrk(CblasColMajor, CblasLower, CblasNoTrans,
                                blksz, blksz, -1.0, *a, blksz, 1.0, *c, blksz);
                },
                A[i * nblks + j].read(),
                A[j * nblks + j].write());
        }

        // Cholesky Factorization of A[j,j]
        rg.emplace_task(
            [j](auto a)
            {
                LAPACKE_dpotrf(LAPACK_COL_MAJOR, 'L', blksz, *a, blksz);
            },
            A[j * nblks + j].write());

        for(size_t i = j + 1; i < nblks; i++)
        {
            // A[i,j] <- A[i,j] = X * (A[j,j])^t
            rg.emplace_task(
                [](auto a, auto b)
                {
                    cblas_dtrsm(CblasColMajor,
                                CblasRight, CblasLower, CblasTrans, CblasNonUnit,
                                blksz, blksz, 1.0, *a, blksz, *b, blksz);
                },
                A[j * nblks + j].read(),
                A[j * nblks + i].write());
        }
    }
        
    auto end = high_resolution_clock::now();

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "total " << duration_cast<nanoseconds>(end-start).count()/1000000.0 << " ms" << std::endl;

    return 0;
}
