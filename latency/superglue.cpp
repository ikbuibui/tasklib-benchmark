#include <atomic>
#include <cstdint>
#include <chrono>
#include <sg/superglue.hpp>

using namespace std::chrono;

unsigned n_tasks = 10000;

struct Options : public DefaultOptions<Options> {};

struct LatencyTask : Task<Options, 0> {
    time_point<high_resolution_clock> & stop;

    LatencyTask(time_point<high_resolution_clock> & stop)
        : stop(stop)
    {
    }

    void run()
    {
        stop = high_resolution_clock::now();
    }
};

int main(int argc, char* argv[])
{
    SuperGlue<Options> sg(1);

    nanoseconds avg_latency(0);

    for(unsigned i = 0; i < n_tasks; ++i)
    {
        auto start = high_resolution_clock::now();
        auto stop = high_resolution_clock::now();

        sg.submit(new LatencyTask(stop));
        sg.barrier();

        avg_latency += duration_cast<nanoseconds>(stop - start);
    }

    avg_latency /= n_tasks;

    std::cout << "avg latency = " << avg_latency.count() << " ns" << std::endl;

    return 0;
}
