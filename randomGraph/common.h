#include <cassert>
#include <chrono>
#include <random>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include "sha256.c"

std::chrono::microseconds min_task_duration(25);
std::chrono::microseconds max_task_duration(25);
unsigned n_resources = 5;
unsigned n_tasks = 1000;
unsigned n_threads = 4;
unsigned min_dependencies = 0;
unsigned max_dependencies = 0;
std::mt19937 gen;

std::vector<std::chrono::microseconds> task_duration;
std::vector<std::vector<unsigned>> access_pattern;
std::vector<std::array<uint64_t, 8>> expected_hash;

void read_args(int argc, char* argv[])
{
    if(argc == 1)
        std::cout << "usage: " << argv[0] << " [n_tasks = " << n_tasks << "] [n_resources = " << n_resources
		  << "] [min_dependencies = " << min_dependencies
                  << "] [max_dependencies = " << max_dependencies
                  << "] [min_task_duration (μs) = " << max_task_duration.count()
                  << "] [max_task_duration (μs) = " << min_task_duration.count()
                  << "] [n_threads = " << n_threads
                  << "] [seed = 1]" << std::endl;

    if(argc > 1)
        n_tasks = atoi(argv[1]);
    if(argc > 2)
        n_resources = atoi(argv[2]);
    if(argc > 3)
        min_dependencies = atoi(argv[3]);
    if(argc > 4)
        max_dependencies = atoi(argv[4]);
    if(argc > 5)
        min_task_duration = std::chrono::microseconds(atoi(argv[5]));
    if(argc > 6)
        max_task_duration = std::chrono::microseconds(atoi(argv[6]));
    if(argc > 7)
        n_threads = atoi(argv[7]);
    if(argc > 8)
        gen.seed(atoi(argv[8]));

    assert(min_dependencies <= max_dependencies);
    assert(max_dependencies <= n_resources);
    assert(min_task_duration <= max_task_duration);
    assert(max_dependencies < 6);
}

void sleep(std::chrono::microseconds d)
{
    auto end = std::chrono::high_resolution_clock::now() + d;
    while(std::chrono::high_resolution_clock::now() < end)
        ;
}

void hash(unsigned task_id,
          std::array<uint64_t, 8> & val)
{
    val[0] += task_id;

    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    sha256_process((uint32_t*)&state, (uint8_t*) (uint8_t*)&val[0], 32);
}

void generate_access_pattern()
{
    std::uniform_int_distribution<unsigned> distrib_n_deps(min_dependencies, max_dependencies);
    std::uniform_int_distribution<unsigned> distrib_resource(0, n_resources - 1);
    std::uniform_int_distribution<unsigned> distrib_duration(min_task_duration.count(), max_task_duration.count());

    expected_hash = std::vector<std::array<uint64_t, 8>>(n_resources);

    task_duration.reserve(n_tasks);
    access_pattern.reserve(n_tasks);
    
    for(int i = 0; i < n_tasks; ++i)
    {
        task_duration.emplace_back(distrib_duration(gen));
        access_pattern.emplace_back();

        unsigned n_dependencies = distrib_n_deps(gen);
        for(int j = 0; j < n_dependencies; ++j)
        {
            std::chrono::microseconds max_path_length(0);

            while(1)
            {
                unsigned resource_id = distrib_resource(gen);
                if(std::find(access_pattern[i].begin(), access_pattern[i].end(), resource_id)
                   == access_pattern[i].end())
                {
                    access_pattern[i].push_back(resource_id);
                    hash(i, expected_hash[resource_id]);

                    break;
                }
            }
        }
    }
}

std::chrono::microseconds get_critical_path()
{
    std::vector<std::chrono::microseconds> path_length(n_resources);

    for(int i = 0; i < n_tasks; ++i)
    {
        std::chrono::microseconds max_path_length(0);

        for( unsigned rid : access_pattern[i] )
            if( path_length[rid] > max_path_length )
                max_path_length = path_length[rid];

        for( unsigned rid : access_pattern[i] )
            path_length[rid] = max_path_length + task_duration[i];
    }

    std::chrono::microseconds max_path_length = min_task_duration;
    for( std::chrono::microseconds pl : path_length )
      if( pl > max_path_length )
	max_path_length = pl;

    if( max_dependencies == 0 )
        max_path_length = (min_task_duration * n_tasks) / n_threads;

    std::cout << "critical path " << max_path_length.count() << " μs" << std::endl;
    
    return max_path_length;
}

