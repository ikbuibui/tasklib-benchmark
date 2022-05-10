#include <chrono>
#include <vector>
#include <iostream>
#include "quark.h"
#include "common.h"

using namespace std::chrono;

void myTask(Quark * quark)
{
    sleep( task_duration );
}

int main(int argc, char* argv[])
{
    read_args(argc, argv);

    Quark * quark = QUARK_New(n_threads);
    std::vector< int > resources(n_resources);

    auto start = high_resolution_clock::now();

    for( int i = 0; i < n_tasks; ++i )
        QUARK_Insert_Task(quark, myTask, NULL,
#if DEPENDENCIES_PER_TASK >= 1
                          sizeof(int), &resources[ rand()%n_resources ], INOUT,
#endif
#if DEPENDENCIES_PER_TASK >= 2
                          sizeof(int), &resources[ rand()%n_resources ], INOUT,
#endif
#if DEPENDENCIES_PER_TASK >= 4
                          sizeof(int), &resources[ rand()%n_resources ], INOUT,
                          sizeof(int), &resources[ rand()%n_resources ], INOUT,
#endif
#if DEPENDENCIES_PER_TASK >= 8
                          sizeof(int), &resources[ rand()%n_resources ], INOUT,
                          sizeof(int), &resources[ rand()%n_resources ], INOUT,
                          sizeof(int), &resources[ rand()%n_resources ], INOUT,
                          sizeof(int), &resources[ rand()%n_resources ], INOUT,
#endif
                          0);

    QUARK_Waitall(quark);
    auto end = high_resolution_clock::now();

    std::cout << "total " << duration_cast<microseconds>(end-start).count() << " μs" << std::endl;

    QUARK_Delete(quark);
    
    return 0;
}


