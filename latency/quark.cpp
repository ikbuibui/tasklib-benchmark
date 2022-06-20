#include <chrono>
#include <vector>
#include <iostream>


#include <string.h> // need to include this to compile
#include "quark.h"
#include "common.h"

using namespace std::chrono;

unsigned n_threads = 1;
unsigned n_tasks = 10000;

time_point<high_resolution_clock> stop;

void myTask0(Quark * quark)
{
    stop = high_resolution_clock::now();
}

int main(int argc, char* argv[])
{
    Quark * quark = QUARK_New(n_threads);

    nanoseconds avg_latency(0);
    
    for( unsigned i = 0; i < n_tasks; ++i )
    {
        auto start = high_resolution_clock::now();
        QUARK_Insert_Task(quark, myTask0, NULL, 0);
        QUARK_Waitall(quark);

        avg_latency += duration_cast<nanoseconds>(stop - start);
    }

    avg_latency /= n_tasks;
    std::cout << "avg latency = " << avg_latency.count()/1000.0 << " μs" << std::endl;

    return 0;
}

