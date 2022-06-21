#include <chrono>
#include <iostream>
#include "sg/superglue.hpp"


using namespace std::chrono;

int n_threads = 4;
int DIM = 5;
microseconds delay(10);

void sleep()
{
    auto end = std::chrono::high_resolution_clock::now() + delay;
    while(std::chrono::high_resolution_clock::now() < end);
}

struct Options : public DefaultOptions<Options> {};

struct potrf : public Task<Options, 1> {
    potrf(double *A, Handle<Options> &h) {
        register_access(ReadWriteAdd::write, h);
    }
    void run() { sleep(); }
};

struct gemm : public Task<Options, 3> {
    gemm(double *A, double *B, double *C,
         Handle<Options> &hA, Handle<Options> &hB, Handle<Options> &hC) {
        register_access(ReadWriteAdd::read, hA);
        register_access(ReadWriteAdd::read, hB);
        register_access(ReadWriteAdd::write, hC);
    }
    void run() { sleep(); }
};

struct trsm : public Task<Options, 2> {
    trsm(double *T, double *B,
         Handle<Options> &hT, Handle<Options> &hB) {
        register_access(ReadWriteAdd::read, hT);
        register_access(ReadWriteAdd::write, hB);
    }
    void run() { sleep(); }
};

struct syrk : public Task<Options, 2> {
    syrk(double *A, double *C,
         Handle<Options> &hA, Handle<Options> &hC) {
        register_access(ReadWriteAdd::read, hA);
        register_access(ReadWriteAdd::write, hC);
    }
    void run() { sleep(); }
};

int main(int argc, char *argv[]) {
    if (argc > 1)
        DIM = atoi(argv[1]);
    if (argc > 2)
        delay = microseconds(atoi(argv[2]));
    if (argc > 3)
        n_threads = atoi(argv[3]);

    SuperGlue<Options> tm(n_threads);

    double *A = (double*) malloc(DIM*DIM*sizeof(double));
    Handle<Options> *h = new Handle<Options>[DIM*DIM];

    auto start = high_resolution_clock::now();

    for (int k = 0; k < DIM; ++k) {
        tm.submit(new potrf(&A[k*DIM+k], h[k*DIM+k]));

        for (int m = k+1; m < DIM; ++m) {
            tm.submit(new trsm(&A[k*DIM+k], &A[m*DIM+k],
                               h[k*DIM+k], h[m*DIM+k]));
        }

        for (int m=k+1; m < DIM; ++m) {
            tm.submit(new syrk(&A[m*DIM+k], &A[m*DIM+m],
                                h[m*DIM+k], h[m*DIM+m]));

            for (int n=k+1; n < m; ++n) {
                tm.submit(new gemm(&A[m*DIM+k], &A[n*DIM+k], &A[m*DIM+n],
                                   h[m*DIM+k], h[n*DIM+k], h[m*DIM+n]) );
            }
        }
    }

    tm.barrier();
    
    auto end = high_resolution_clock::now();

    std::cout << "total " << duration_cast<nanoseconds>(end-start).count()/1000000.0 << " ms" << std::endl;

    return 0;
}
