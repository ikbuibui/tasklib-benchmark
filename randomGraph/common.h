#include <cassert>
#include <chrono>
#include <random>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include "sha256.c"

std::chrono::microseconds task_duration;
unsigned n_resources = 5;
unsigned n_tasks = 1000;
unsigned n_threads = 4;
unsigned min_dependencies = 0;
unsigned max_dependencies = 0;
std::mt19937 gen;

std::vector<std::vector<unsigned>> access_pattern;
std::vector<std::array<uint64_t, 8>> expected_hash;

void read_args(int argc, char* argv[])
{
    if(argc == 1)
        std::cout << "usage: " << argv[0] << " [n_tasks = " << n_tasks << "] [n_resources = " << n_resources
		  << "] [min_dependencies = " << min_dependencies
                  << "] [max_dependencies = " << max_dependencies
                  << "] [task_duration (μs) = " << task_duration.count() << "] [n_threads = " << n_threads
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
        task_duration = std::chrono::microseconds(atoi(argv[5]));
    if(argc > 6)
        n_threads = atoi(argv[6]);
    if(argc > 7)
        gen.seed(atoi(argv[7]));

    assert(min_dependencies <= max_dependencies);
    assert(max_dependencies <= n_resources);
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

    for(int i=0; i<8; ++i)
        val[i] = state[i];
}

void generate_access_pattern()
{
    std::uniform_int_distribution<unsigned> distrib_n_deps(min_dependencies, max_dependencies);
    std::uniform_int_distribution<unsigned> distrib_resource(0, n_resources - 1);

    expected_hash = std::vector<std::array<uint64_t, 8>>(n_resources);
    std::vector<unsigned> path_length(n_resources);

    for(int i = 0; i < n_tasks; ++i)
    {
        access_pattern.emplace_back();
        unsigned n_dependencies = distrib_n_deps(gen);
        for(int j = 0; j < n_dependencies; ++j)
        {
	    unsigned max_path_length = 0;

            while(1)
            {
                unsigned resource_id = distrib_resource(gen);
                if(std::find(access_pattern[i].begin(), access_pattern[i].end(), resource_id)
                   == access_pattern[i].end())
                {
                    access_pattern[i].push_back(resource_id);
                    hash(i, expected_hash[resource_id]);

		    if( path_length[resource_id] > max_path_length )
		      max_path_length = path_length[resource_id];

                    break;
                }
            }

	    for( unsigned rid : access_pattern[i] )
	      path_length[rid] = max_path_length + 1;
        }
    }

    unsigned max_path_length = 1;
    for( unsigned pl : path_length )
      if( pl > max_path_length )
	max_path_length = pl;

    if( max_dependencies == 0 )
        max_path_length = n_tasks / n_threads;

    std::cout << "max path length = " << max_path_length << std::endl;
}

