#include <chrono>
#include <sg/superglue.hpp>

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "sg/superglue.hpp"
#include "common.h"

using namespace std::chrono;

struct Options : public DefaultOptions<Options> {};

struct MyTask : Task<Options, DEPENDENCIES_PER_TASK> {
    MyTask(
#if DEPENDENCIES_PER_TASK >= 1
           Handle<Options> & h1
#endif
#if DEPENDENCIES_PER_TASK >= 2
           , Handle<Options> & h2
#endif
#if DEPENDENCIES_PER_TASK >= 4
           , Handle<Options> & h3
           , Handle<Options> & h4
#endif
#if DEPENDENCIES_PER_TASK >= 8
           , Handle<Options> & h5
           , Handle<Options> & h6
           , Handle<Options> & h7
           , Handle<Options> & h8
#endif           
    )
    {
#if DEPENDENCIES_PER_TASK >= 1
        register_access(ReadWriteAdd::write, h1);
#endif
#if DEPENDENCIES_PER_TASK >= 2
        register_access(ReadWriteAdd::write, h2);
#endif
#if DEPENDENCIES_PER_TASK >= 4
        register_access(ReadWriteAdd::write, h3);
        register_access(ReadWriteAdd::write, h4);
#endif
#if DEPENDENCIES_PER_TASK >= 8
        register_access(ReadWriteAdd::write, h5);
        register_access(ReadWriteAdd::write, h6);
        register_access(ReadWriteAdd::write, h7);
        register_access(ReadWriteAdd::write, h8);
#endif        
    }

    void run()
    {
        sleep( task_duration );
    }
};

int main(int argc, char* argv[])
{
    read_args(argc, argv);

    SuperGlue<Options> tm( n_threads );
    std::vector< Handle<Options> > resources( n_resources );

    auto start = high_resolution_clock::now();
 
    for( int i = 0; i < n_tasks; ++i )
        tm.submit(new MyTask(
#if DEPENDENCIES_PER_TASK >= 1
    resources[ rand()%n_resources ]
#endif
#if DEPENDENCIES_PER_TASK >= 2
    , resources[ rand()%n_resources ]
#endif
#if DEPENDENCIES_PER_TASK >= 4
    , resources[ rand()%n_resources ]
    , resources[ rand()%n_resources ]
#endif
#if DEPENDENCIES_PER_TASK >= 8
    , resources[ rand()%n_resources ]
    , resources[ rand()%n_resources ]
    , resources[ rand()%n_resources ]
    , resources[ rand()%n_resources ]
#endif                      
        ));

    tm.barrier();
    /*
    for( int i = 0; i < n_resources; ++i )
        tm.wait( resources[i] );
    */
    auto end = high_resolution_clock::now();

    std::cout << "total " << duration_cast<microseconds>(end-start).count() << " μs" << std::endl;

    return 0;
}

