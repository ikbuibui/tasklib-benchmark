#include <cassert>
#include <chrono>
#include <random>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <cstring>
#include "sha256.c"

using namespace std::chrono;

microseconds min_task_duration(25);
microseconds max_task_duration(25);
unsigned n_resources = 5;
unsigned n_tasks = 1000;
unsigned n_workers = 4;
unsigned min_dependencies = 0;
unsigned max_dependencies = 0;
bool block_execution = false;

std::mt19937 gen;

std::vector<microseconds> task_duration;
std::vector<time_point<high_resolution_clock>> task_begin;
std::vector<time_point<high_resolution_clock>> task_end;

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
                  << "] [n_workers = " << n_workers
                  << "] [block_execution = " << block_execution
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
        n_workers = atoi(argv[7]);
    if(argc > 8)
    {
        if( strcmp(argv[8], "true") == 0 )
            block_execution = true;
        else
            block_execution = false;
    }
    if(argc > 9)
        gen.seed(atoi(argv[9]));

    assert(min_dependencies <= max_dependencies);
    assert(max_dependencies <= n_resources);
    assert(min_task_duration <= max_task_duration);
    assert(max_dependencies <= 5);
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

    task_begin.reserve(n_tasks);
    task_end.reserve(n_tasks);
    task_duration.reserve(n_tasks);
    access_pattern.reserve(n_tasks);
    
    for(unsigned i = 0; i < n_tasks; ++i)
    {
        task_duration.emplace_back(distrib_duration(gen));
        access_pattern.emplace_back();
        
        if( min_dependencies == 1 && max_dependencies == 1 )
        {
            // chains with equal length
            unsigned resource_id = i % n_resources;
            access_pattern[i].push_back(resource_id);
            hash(i, expected_hash[resource_id]);
        }
        else
        {
            unsigned n_dependencies = distrib_n_deps(gen);
            for(int j = 0; j < n_dependencies; ++j)
            {
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
        max_path_length = (min_task_duration * n_tasks) / n_workers;

    std::cout << "critical path " << max_path_length.count() << " μs" << std::endl;
    
    return max_path_length;
}

// only in case of chains
nanoseconds get_scheduling_gap()
{
    nanoseconds sum(0);
    for(unsigned i = n_resources; i < n_tasks; ++i)
    {
        sum += duration_cast<nanoseconds>(task_begin[i] - task_end[i - n_resources]);
    }
    return sum / (n_tasks - n_resources);
}

