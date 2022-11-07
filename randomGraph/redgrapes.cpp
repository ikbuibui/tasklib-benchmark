#include "common.h"

#include <cstdint>
#include <redGrapes/redGrapes.hpp>
#include <redGrapes/resource/ioresource.hpp>

#include <iomanip>
#include <condition_variable>

namespace rg = redGrapes;
using namespace std::chrono;

std::mutex m;
std::condition_variable cv;
volatile bool start_flag = false;

int main(int argc, char* argv[])
{
    spdlog::set_level( spdlog::level::debug );
    spdlog::set_pattern("[thread %t] %^[%l]%$ %v");

    read_args(argc, argv);
    generate_access_pattern();

    rg::init(n_workers);

    std::vector<rg::IOResource<std::array<uint64_t, 8>>> resources( n_resources );

    if( block_execution )
    {
        for( unsigned i = 0; i < n_workers; ++i )
            rg::emplace_task([&cv]() {
                std::cout << "blocking task" << std::endl;
                std::unique_lock<std::mutex> l(m);
                cv.wait(l, [] {
                    return start_flag;
                });
                std::cout << "blocking task released" << std::endl;    
            });

        std::this_thread::sleep_for( std::chrono::milliseconds(100) );
    }

    auto start = high_resolution_clock::now();

    for(int i = 0; i < n_tasks; ++i)
        switch(access_pattern[i].size())
        {
        case 0:
            rg::emplace_task([i]() {
                task_begin[i] = high_resolution_clock::now();

                sleep(task_duration[i]);

                task_end[i] = high_resolution_clock::now();
            });
            break;

        case 1:
            rg::emplace_task(
                [i](auto ra1)
                {
                    task_begin[i] = high_resolution_clock::now();

                    sleep(task_duration[i]);
                    hash(i, *ra1);

                    task_end[i] = high_resolution_clock::now();
                },
                resources[access_pattern[i][0]].write());
            break;

        case 2:
            rg::emplace_task(
                [i](auto ra1, auto ra2)
                {
                    task_begin[i] = high_resolution_clock::now();

                    sleep(task_duration[i]);
                    hash(i, *ra1);
                    hash(i, *ra2);

                    task_end[i] = high_resolution_clock::now();
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write());
            break;

        case 3:
            rg::emplace_task(
                [i](auto ra1, auto ra2, auto ra3)
                {
                    task_begin[i] = high_resolution_clock::now();

                    sleep(task_duration[i]);
                    hash(i, *ra1);
                    hash(i, *ra2);
                    hash(i, *ra3);

                    task_end[i] = high_resolution_clock::now();
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write(),
                resources[access_pattern[i][2]].write());
            break;

        case 4:
            rg::emplace_task(
                [i](auto ra1, auto ra2, auto ra3, auto ra4)
                {
                    task_begin[i] = high_resolution_clock::now();

                    sleep(task_duration[i]);
                    hash(i, *ra1);
                    hash(i, *ra2);
                    hash(i, *ra3);
                    hash(i, *ra4);

                    task_end[i] = high_resolution_clock::now();
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write(),
                resources[access_pattern[i][2]].write(),
                resources[access_pattern[i][3]].write());
            break;

        case 5:
            rg::emplace_task(
                [i](auto ra1, auto ra2, auto ra3, auto ra4, auto ra5)
                {
                    task_begin[i] = high_resolution_clock::now();

                    sleep(task_duration[i]);
                    hash(i, *ra1);
                    hash(i, *ra2);
                    hash(i, *ra3);
                    hash(i, *ra4);
                    hash(i, *ra5);

                    task_end[i] = high_resolution_clock::now();
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write(),
                resources[access_pattern[i][2]].write(),
                resources[access_pattern[i][3]].write(),
                resources[access_pattern[i][4]].write());
            break;
        }

    auto mid = high_resolution_clock::now();

    if( block_execution )
    {
        // trigger execution of tasks
        {
            std::unique_lock<std::mutex> l(m);
            start_flag = true;
        }
        cv.notify_all();
    }

    // wait for execution to finish
    rg::barrier();

    auto end = high_resolution_clock::now();

    rg::finalize();

    for(int i = 0; i < n_resources; ++i)
        if(*resources[i] != expected_hash[i])
        {
            std::cout << "error: invalid result!" << std::endl;
            return -1;
        }

    std::cout << "success" << std::endl;

    
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "total " << duration_cast<nanoseconds>(end-start).count()/1000.0 << " μs" << std::endl;
    std::cout << "emplacement " << duration_cast<nanoseconds>(mid-start).count()/1000.0 << " μs" << std::endl;
    std::cout << "execution " << duration_cast<nanoseconds>(end-mid).count()/1000.0 << " μs" << std::endl;
    std::cout << "scheduling gap " << duration_cast<nanoseconds>(get_scheduling_gap()).count() / 1000.0 << " μs" << std::endl;

    get_critical_path();

    return 0;
}
