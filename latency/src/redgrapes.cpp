#include <cstdint>
#include <iostream>
#include <redGrapes/redGrapes.hpp>
#include <redGrapes/resource/ioresource.hpp>
#include "common.h"

using namespace std::chrono;

unsigned n_threads = 1;
unsigned n_tasks = 10000;

int main(int argc, char* argv[])
{
    auto rg = redGrapes::init(n_threads);

    /* warmup */
    {
        for(unsigned i = 0; i < 64; ++i)
            rg.emplace_task(
                             []{});
        rg.barrier();
    }

    /* measure */
    nanoseconds avg_latency(0);

    for(unsigned i = 0; i < n_tasks; ++i)
    {
        auto start = high_resolution_clock::now();
        auto stop = rg.emplace_task(
            []() {
                return high_resolution_clock::now();
            }).get();

        avg_latency += duration_cast<nanoseconds>(stop - start);
    }

    avg_latency /= n_tasks;

    std::cout << "avg latency = " << avg_latency.count()/1000.0 << " μs" << std::endl;

    return 0;
}
    
