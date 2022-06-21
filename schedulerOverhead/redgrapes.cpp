
#include <mutex>
#include <thread>
#include <condition_variable>
#include <cstdint>
#include <redGrapes/redGrapes.hpp>
#include <redGrapes/resource/ioresource.hpp>

namespace rg = redGrapes;
using namespace std::chrono;

unsigned n_tasks = 1000;

std::condition_variable cv;
std::mutex m;
bool ready = false;

int main(int argc, char* argv[])
{
    if( argc > 1 )
        n_tasks = atoi(argv[1]);

    rg::init(1);

    rg::IOResource<time_point<high_resolution_clock>> start;

    /* warmup */
    {
        for(unsigned i = 0; i < 64; ++i)
            rg::emplace_task([](auto start){},
                             start.write());
        rg::barrier();
    }

    /* measure */

    rg::emplace_task(
       []( auto start ) {
           std::unique_lock<std::mutex> l(m);
           cv.wait(l, []{ return ready; });

            *start = high_resolution_clock::now();
        },
        start.write());

    auto task_create_start = high_resolution_clock::now();

    for(unsigned i = 0; i < n_tasks; ++i)
        rg::emplace_task(
            [](auto start){},
            start.write());

    auto task_create_end = high_resolution_clock::now();

    rg::emplace_task(
        []( auto start ) {
            auto end = high_resolution_clock::now();
            nanoseconds avg_deadtime = duration_cast<nanoseconds>(end - *start);
            avg_deadtime /= (n_tasks+1);
            std::cout << "avg deadtime: " << avg_deadtime.count()/1000.0 << " μs" << std::endl;
        },
        start.write());

    {
        std::unique_lock<std::mutex> l(m);
        ready = true;
    }
    cv.notify_one();

    rg::barrier();

    auto avg_task_create_overhead = duration_cast<nanoseconds>(task_create_end - task_create_start) / n_tasks;
    std::cout << "avg task emplace time: " << avg_task_create_overhead.count()/1000.0 << " μs" << std::endl;
    
    return 0;
}
