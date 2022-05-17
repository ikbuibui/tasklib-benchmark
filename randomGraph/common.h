#include <cassert>
#include <chrono>
#include <random>
#include <iostream>
#include <vector>
#include <algorithm>

std::chrono::microseconds task_duration;
unsigned n_resources = 5;
unsigned n_tasks = 1000;
unsigned n_threads = 4;
unsigned min_dependencies = 0;
unsigned max_dependencies = 0;
std::mt19937 gen;

std::vector<std::vector<unsigned>> access_pattern;
std::vector<uint64_t> expected_hash;

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

uint64_t hash(uint64_t x)
{
    return (x << 2) + x % 4;
}

void generate_access_pattern()
{
    std::uniform_int_distribution<unsigned> distrib_n_deps(min_dependencies, max_dependencies);
    std::uniform_int_distribution<unsigned> distrib_resource(0, n_resources - 1);

    expected_hash = std::vector<uint64_t>(n_resources);
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
                    expected_hash[resource_id] = hash(expected_hash[resource_id] + i);

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

    std::cout << "max path length = " << max_path_length << std::endl;
}

