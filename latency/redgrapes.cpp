#include <cstdint>
#include <redGrapes/redGrapes.hpp>
#include <redGrapes/resource/ioresource.hpp>
#include "common.h"

namespace rg = redGrapes;
using namespace std::chrono;

unsigned n_threads = 1;
unsigned n_tasks = 10000;

int main(int argc, char* argv[])
{
    rg::init(n_threads);

    nanoseconds avg_latency(0);

    for(unsigned i = 0; i < n_tasks; ++i)
    {
        auto start = high_resolution_clock::now();
        auto stop = rg::emplace_task(
            [start]() {
                return high_resolution_clock::now();
            }).get();

        avg_latency += duration_cast<nanoseconds>(stop - start);
    }

    avg_latency /= n_tasks;

    std::cout << "avg latency = " << avg_latency.count() << " ns" << std::endl;

    return 0;
}
    
