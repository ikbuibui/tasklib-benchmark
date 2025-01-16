#include "common.h"
#include <rg.hpp>

#include <cblas.h>
#include <chrono>
#include <iomanip>
#include <lapacke.h>

using namespace std::chrono;

auto cholesky([[maybe_unused]] rg::ThreadPool *ptr) -> rg::InitTask<int> {
  // initialize tiled matrix in column-major layout
  std::vector<rg::Resource<std::shared_ptr<double *>>> A(nblks * nblks);

  // allocate each tile (also in column-major layout)
  for (size_t j = 0; j < nblks; ++j)
    for (size_t i = 0; i < nblks; ++i)
      A[j * nblks + i] =
          rg::Resource(std::make_shared<double *>(new double[blksz * blksz]));

  /* ia: row of outer matrix
     ib: row of inner matrix
     ja: col of outer matrix
     jb: col of inner matrix */
  double *Alin = init_matrix();
  for (size_t ia = 0; ia < nblks; ++ia)
    for (size_t ib = 0; ib < blksz; ++ib)
      for (size_t ja = 0; ja < nblks; ++ja)
        for (size_t jb = 0; jb < blksz; ++jb)
          (*A[ja * nblks + ia].get())[jb * blksz + ib] =
              Alin[(ia * blksz + ib) + (ja * blksz + jb) * N];

  auto start = high_resolution_clock::now();

  // calculate cholesky decomposition
  for (size_t j = 0; j < nblks; j++) {
    for (size_t k = 0; k < j; k++) {
      for (size_t i = j + 1; i < nblks; i++) {
        // A[i,j] = A[i,j] - A[i,k] * (A[j,k])^t
        co_await rg::dispatch_task(
            [](auto a, auto b, auto c) -> rg::Task<void> {
              cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, blksz, blksz,
                          blksz, -1.0, *a, blksz, *b, blksz, 1.0, *c, blksz);
            },
            A[k * nblks + i].rg_read(), A[k * nblks + j].rg_read(),
            A[j * nblks + i].rg_write());
      }
    }

    for (size_t i = 0; i < j; i++) {
      // A[j,j] = A[j,j] - A[j,i] * (A[j,i])^t
      co_await rg::dispatch_task(
          [](auto a, auto c) -> rg::Task<void> {
            cblas_dsyrk(CblasColMajor, CblasLower, CblasNoTrans, blksz, blksz,
                        -1.0, *a, blksz, 1.0, *c, blksz);
          },
          A[i * nblks + j].rg_read(), A[j * nblks + j].rg_write());
    }

    // Cholesky Factorization of A[j,j]
    co_await rg::dispatch_task(
        [j](auto a) -> rg::Task<void> {
          LAPACKE_dpotrf(LAPACK_COL_MAJOR, 'L', blksz, *a, blksz);
        },
        A[j * nblks + j].rg_write());

    for (size_t i = j + 1; i < nblks; i++) {
      // A[i,j] <- A[i,j] = X * (A[j,j])^t
      co_await rg::dispatch_task(
          [](auto a, auto b) -> rg::Task<void> {
            cblas_dtrsm(CblasColMajor, CblasRight, CblasLower, CblasTrans,
                        CblasNonUnit, blksz, blksz, 1.0, *a, blksz, *b, blksz);
          },
          A[j * nblks + j].rg_read(), A[j * nblks + i].rg_write());
    }
  }

  auto end = high_resolution_clock::now();

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "total "
            << duration_cast<nanoseconds>(end - start).count() / 1000000.0
            << " ms" << std::endl;
}

int main(int argc, char *argv[]) {
  read_args(argc, argv);
  auto poolObj = rg::init(n_workers);
  auto a = cholesky(poolObj.pool_ptr());

  return 0;
}
