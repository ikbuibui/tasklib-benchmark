#include <chrono>
#include <redGrapes/redGrapes.hpp>
#include <redGrapes/resource/ioresource.hpp>

using namespace std::chrono;
namespace rg = redGrapes;

int n_threads = 4;
int DIM = 5;
microseconds delay(10);

void sleep()
{
    auto end = std::chrono::high_resolution_clock::now() + delay;
    while(std::chrono::high_resolution_clock::now() < end);
}

int main(int argc, char *argv[]) {
    if (argc > 1)
        DIM = atoi(argv[1]);
    if (argc > 2)
        delay = microseconds(atoi(argv[2]));
    if (argc > 3)
        n_threads = atoi(argv[3]);

    rg::init(n_threads);
    std::vector<redGrapes::IOResource<int>> A( DIM*DIM );
    
    auto start = high_resolution_clock::now();

    for (int k = 0; k < DIM; ++k) {
        rg::emplace_task(
            [](auto a) { sleep(); },
            A[k*DIM + k].write());

        for (int m = k+1; m < DIM; ++m) {
            rg::emplace_task(
                [k, m](auto a, auto b) {
                    sleep();
                },
                A[k*DIM + k].read(),
                A[m*DIM + k].write());
        }

        for (int m=k+1; m < DIM; ++m) {
            rg::emplace_task(
                [k, m](auto a, auto b) {
                    sleep();
                },
                A[m*DIM + k].read(),
                A[m*DIM + m].write());

            for (int n=k+1; n < m; ++n) {
                rg::emplace_task(
                    [](auto a, auto b, auto c) {
                        sleep();
                    },
                    A[m*DIM + k].read(),
                    A[n*DIM + k].read(),
                    A[m*DIM + n].write());
            }
        }
    }

    rg::barrier();
    
    auto end = high_resolution_clock::now();

    std::cout << "total " << duration_cast<nanoseconds>(end-start).count()/1000000.0 << " ms" << std::endl;

    return 0;
}
