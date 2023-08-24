#include <cassert>
#include <chrono>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <thread>
#include <unistd.h>
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
std::vector<time_point<steady_clock>> task_begin;
std::vector<time_point<steady_clock>> task_end;
#ifdef REDGRAPES
std::vector<unsigned> task_worker;
#else
std::vector<std::thread::id> task_thread;
#endif

std::vector<time_point<steady_clock>> wait_task_begin;
std::vector<time_point<steady_clock>> wait_task_end;
#ifdef REDGRAPES
std::vector<unsigned> wait_task_worker;
#else
std::vector<std::thread::id> wait_task_thread;
#endif

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
    auto end = std::chrono::steady_clock::now() + d;
    while(std::chrono::steady_clock::now() < end)
        ;
//    usleep(d.count());
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

    #ifdef REDGRAPES
    task_worker = std::vector<unsigned>(n_tasks);
    #else
    task_thread = std::vector<std::thread::id>(n_tasks);
    #endif

    wait_task_begin = std::vector<time_point<steady_clock>>(n_workers);
    wait_task_end = std::vector<time_point<steady_clock>>(n_workers);

#ifdef REDGRAPES
    wait_task_worker = std::vector<unsigned>(n_workers);
    #else
    wait_task_thread = std::vector<std::thread::id>(n_workers);
    #endif

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
        task_duration[i] = duration_cast<microseconds>(task_end[i] - task_begin[i]);
        
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

void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV) {
  float fC = fV * fS; // Chroma
  float fHPrime = fmod(fH / 60.0, 6);
  float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
  float fM = fV - fC;
  
  if(0 <= fHPrime && fHPrime < 1) {
    fR = fC;
    fG = fX;
    fB = 0;
  } else if(1 <= fHPrime && fHPrime < 2) {
    fR = fX;
    fG = fC;
    fB = 0;
  } else if(2 <= fHPrime && fHPrime < 3) {
    fR = 0;
    fG = fC;
    fB = fX;
  } else if(3 <= fHPrime && fHPrime < 4) {
    fR = 0;
    fG = fX;
    fB = fC;
  } else if(4 <= fHPrime && fHPrime < 5) {
    fR = fX;
    fG = 0;
    fB = fC;
  } else if(5 <= fHPrime && fHPrime < 6) {
    fR = fC;
    fG = 0;
    fB = fX;
  } else {
    fR = 0;
    fG = 0;
    fB = 0;
  }
  
  fR += fM;
  fG += fM;
  fB += fM;
}

void output_svg(std::ofstream f)
{
    time_point<steady_clock> start = wait_task_end[0];
    time_point<steady_clock> end = task_end[0];

    if( block_execution )
    {
        for(unsigned i = 0; i < n_workers; ++i)
        {
            if( wait_task_end[i] < start )
                start = wait_task_end[i];
        }
    }
    else
    {
        start = task_begin[0];
        for(unsigned i = 0; i < n_tasks; ++i)
        {
            if( task_begin[i] < start )
                start = task_begin[i];
        }
    }
    
    for(unsigned i = 0; i < n_tasks; ++i)
    {
        if( task_end[i] > end )
            end = task_end[i];
    }

    unsigned th = 20;

    unsigned width = duration_cast<microseconds>(end - start).count() + 1;
    unsigned height = n_workers * th;

    std::vector<std::string> resource_colors;
    std::uniform_int_distribution<unsigned> col_dist(30, 255);
    for(unsigned i = 0; i < n_resources; ++i)
    {
        float H,S,V;

        H=(360.0 * float(i) / float(n_resources));
        if( i % 2 == 0 ) S =0.8; else S=0.3;
        if( i % 3 == 0 ) V =0.5; else V=0.8;

        float R,G,B;
        HSVtoRGB(R,G,B,H,S,V);
        unsigned r = (R * 255.0);
        unsigned g = (G * 255.0);
        unsigned b = (B * 255.0);

        std::stringstream colstring;
        colstring << std::hex << r << g << b;
        resource_colors.push_back(colstring.str());
    }

    f << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << std::endl;
    f << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << std::endl;
    f << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">" << std::endl;

    // background
    f << "<rect fill=\"#fff\" stroke=\"#000\" x=\"0\" y=\"0\" width=\""<<width<<"\" height=\""<<height<<"\"/>" << std::endl;

    std::unordered_map<std::thread::id, unsigned> tids;
    unsigned next_tid = 0;

    if( block_execution )
    {
        for(unsigned i = 0; i < n_workers; ++i)
        {
            #ifdef REDGRAPES
            unsigned tid = wait_task_worker[i];
            #else
            if( tids.count( wait_task_thread[i] ) == 0 )
                tids.emplace( wait_task_thread[i], next_tid++ );

            unsigned tid = tids[wait_task_thread[i]];
            #endif

            unsigned w = duration_cast<microseconds>(wait_task_end[i] - start).count();
            f << "<rect fill=\"#f00\" stroke=\"#000\" x=\"0\" y=\""<<(th * tid)<<"\" width=\""<<w<<"\" height=\""<<th<<"\"/>" << std::endl;        
        }
    }

    for(unsigned i = 0; i < n_tasks; ++i)
    {
        #ifdef REDGRAPES
        unsigned tid = task_worker[i];
        #else
        if( tids.count( task_thread[i] ) == 0 )
            tids.emplace( task_thread[i], next_tid++ );

        unsigned tid = tids[task_thread[i]];
        #endif

        unsigned x = duration_cast<microseconds>(task_begin[i] - start).count();
        unsigned w = duration_cast<microseconds>(task_end[i] - task_begin[i]).count();
        f << "<rect fill=\"#" << resource_colors[i % n_resources] << "\" stroke=\"#000\" x=\""<<x<<"\" y=\""<<(th * tid)<<"\" width=\""<<w<<"\" height=\""<<th<<"\"/>" << std::endl;        
    }

    
    f << "</svg>" << std::endl;
}




