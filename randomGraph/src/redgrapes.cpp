#define REDGRAPES 1
#include "common.h"

#include <cstdint>
#include <redGrapes/redGrapes.hpp>
#include <redGrapes/resource/ioresource.hpp>

#include <iomanip>
#include <mutex>
#include <condition_variable>

using namespace std::chrono;

std::mutex m;
std::condition_variable cv;
std::atomic<bool> start_flag{ false };

int main(int argc, char* argv[])
{
    spdlog::set_level( spdlog::level::off );
    spdlog::set_pattern("[thread %t] %^[%l]%$ %v");

    read_args(argc, argv);
    generate_access_pattern();

    auto rg = redGrapes::init(n_workers);
    using TTask = decltype(rg)::RGTask;

#if REDGRAPES_ENABLE_TRACE
    auto ts = StartTracing();
#endif

    std::vector<redGrapes::IOResource<std::array<uint64_t, 8>,TTask>> resources( n_resources );

    if( block_execution )
    {
        std::atomic_int count(0);
        
        for( unsigned i = 0; i < n_workers; ++i )
            rg.emplace_task([i, &count]() {
                wait_task_begin[i] = steady_clock::now();
                wait_task_worker[i] = *(redGrapes::TaskFreeCtx::current_worker_id);
                count.fetch_add(1);

                // block this worker until start flag
                while( ! start_flag );

                wait_task_end[i] = steady_clock::now();
            });

        // wait until all block-tasks are up and running
        int last_count = count;
        while( count < n_workers ) {

            if(last_count !=  count)
                std::cout << count << std::endl;

            last_count = count;
        }
    }

    auto start = steady_clock::now();
    
    for(int i = 0; i < n_tasks; ++i)
        switch(access_pattern[i].size())
        {
        case 0:
            rg.emplace_task([i]() {
                task_begin[i] = steady_clock::now();

                task_worker[i] = *(redGrapes::TaskFreeCtx::current_worker_id);
                sleep(task_duration[i]);

                task_end[i] = steady_clock::now();
            });
            break;

        case 1:
            rg.emplace_task(
                [i](auto ra1)
                {
                    task_begin[i] = steady_clock::now();

                    //spdlog::info("task {}, res {}", i, access_pattern[i][0]);
                    sleep(task_duration[i]);
                    task_worker[i] = *(redGrapes::TaskFreeCtx::current_worker_id);
                    hash(i, *ra1);

                    task_end[i] = steady_clock::now();
                },
                resources[access_pattern[i][0]].write());
            break;

        case 2:
            rg.emplace_task(
                [i](auto ra1, auto ra2)
                {
                    task_begin[i] = steady_clock::now();

                    sleep(task_duration[i]);
                    task_worker[i] = *(redGrapes::TaskFreeCtx::current_worker_id);
                    hash(i, *ra1);
                    hash(i, *ra2);

                    task_end[i] = steady_clock::now();
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write());
            break;

        case 3:
            rg.emplace_task(
                [i](auto ra1, auto ra2, auto ra3)
                {
                    task_begin[i] = steady_clock::now();

                    sleep(task_duration[i]);
                    task_worker[i] = *(redGrapes::TaskFreeCtx::current_worker_id);

                    hash(i, *ra1);
                    hash(i, *ra2);
                    hash(i, *ra3);

                    task_end[i] = steady_clock::now();
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write(),
                resources[access_pattern[i][2]].write());
            break;

        case 4:
            rg.emplace_task(
                [i](auto ra1, auto ra2, auto ra3, auto ra4)
                {
                    task_begin[i] = steady_clock::now();

                    sleep(task_duration[i]);
                    task_worker[i] = *(redGrapes::TaskFreeCtx::current_worker_id);

                    hash(i, *ra1);
                    hash(i, *ra2);
                    hash(i, *ra3);
                    hash(i, *ra4);

                    task_end[i] = steady_clock::now();
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write(),
                resources[access_pattern[i][2]].write(),
                resources[access_pattern[i][3]].write());
            break;

        case 5:
            rg.emplace_task(
                [i](auto ra1, auto ra2, auto ra3, auto ra4, auto ra5)
                {
                    task_begin[i] = steady_clock::now();

                    sleep(task_duration[i]);
                    task_worker[i] = *(redGrapes::TaskFreeCtx::current_worker_id);

                    hash(i, *ra1);
                    hash(i, *ra2);
                    hash(i, *ra3);
                    hash(i, *ra4);
                    hash(i, *ra5);

                    task_end[i] = steady_clock::now();
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write(),
                resources[access_pattern[i][2]].write(),
                resources[access_pattern[i][3]].write(),
                resources[access_pattern[i][4]].write());
            break;
        }

    auto mid = steady_clock::now();

    if( block_execution )
    {
        spdlog::info("+++ emplacement done, start executing tasks...");
        // trigger execution of tasks
        start_flag = true;
    }

    // wait for execution to finish
    rg.barrier();

    auto end = steady_clock::now();

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

    output_svg(std::ofstream("trace_redgrapes.svg"));

#if REDGRAPES_ENABLE_TRACE
    StopTracing(std::move(ts));
#endif

    return 0;
}
