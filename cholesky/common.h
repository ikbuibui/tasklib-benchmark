
#include <iostream>

unsigned n_workers = 1;
size_t blksz = 1;
size_t nblks = 1;
size_t N;

void read_args(int argc, char* argv[])
{
  if (argc > 1)
    blksz = atoi(argv[1]);
  if (argc > 2)
    nblks = atoi(argv[2]);
  if (argc > 3)
    n_workers = atoi(argv[3]);

  N = blksz * nblks;

  std::cout << "nblks = " << nblks << std::endl;
  std::cout << "blksz = " << blksz << std::endl;
  std::cout << "matrix width = " << nblks*blksz << std::endl;
}

double * init_matrix()
{
    // allocate input matrix
    double* Alin = (double*) malloc(N * N * sizeof(double));

    // fill the matrix with random values
    for(size_t i = 0; i < N * N; i++)
        Alin[i] = ((double) rand()) / ((double) RAND_MAX);

    // make it positive definite
    for(size_t i = 0; i < N; i++)
        Alin[i * N + i] += N;

    return Alin;
}

