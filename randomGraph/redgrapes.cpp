#include "common.h"

#include <cstdint>
#include <redGrapes/redGrapes.hpp>
#include <redGrapes/resource/ioresource.hpp>

namespace rg = redGrapes;
using namespace std::chrono;

int main(int argc, char* argv[])
{
    read_args(argc, argv);
    generate_access_pattern();

    rg::init(n_threads);

    std::vector<rg::IOResource<uint64_t>> resources(n_resources);

    auto start = high_resolution_clock::now();

    for(int i = 0; i < n_tasks; ++i)
        switch(access_pattern[i].size())
        {
        case 0:
            rg::emplace_task([i]() { sleep(task_duration); });
            break;

        case 1:
            rg::emplace_task(
                [i](auto ra1)
                {
                    sleep(task_duration);
                    *ra1 = hash(*ra1 + i);
                },
                resources[access_pattern[i][0]].write());
            break;

        case 2:
            rg::emplace_task(
                [i](auto ra1, auto ra2)
                {
                    sleep(task_duration);
                    *ra1 = hash(*ra1 + i);
                    *ra2 = hash(*ra2 + i);
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write());
            break;

        case 3:
            rg::emplace_task(
                [i](auto ra1, auto ra2, auto ra3)
                {
                    sleep(task_duration);
                    *ra1 = hash(*ra1 + i);
                    *ra2 = hash(*ra2 + i);
                    *ra3 = hash(*ra3 + i);
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write(),
                resources[access_pattern[i][2]].write());
            break;

        case 4:
            rg::emplace_task(
                [i](auto ra1, auto ra2, auto ra3, auto ra4)
                {
                    sleep(task_duration);
                    *ra1 = hash(*ra1 + i);
                    *ra2 = hash(*ra2 + i);
                    *ra3 = hash(*ra3 + i);
                    *ra4 = hash(*ra4 + i);
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
                    sleep(task_duration);
                    *ra1 = hash(*ra1 + i);
                    *ra2 = hash(*ra2 + i);
                    *ra3 = hash(*ra3 + i);
                    *ra4 = hash(*ra4 + i);
                    *ra5 = hash(*ra5 + i);
                },
                resources[access_pattern[i][0]].write(),
                resources[access_pattern[i][1]].write(),
                resources[access_pattern[i][2]].write(),
                resources[access_pattern[i][3]].write(),
                resources[access_pattern[i][4]].write());
            break;
        }

    rg::barrier();
    auto end = high_resolution_clock::now();

    fmt::print("total {} μs\n", duration_cast<microseconds>(end - start).count());

    for(int i = 0; i < n_resources; ++i)
        if(*resources[i] != expected_hash[i])
        {
            std::cout << "error: invalid result!" << std::endl;
            return 1;
        }

    std::cout << "success" << std::endl;
    rg::finalize();

    return 0;
}
