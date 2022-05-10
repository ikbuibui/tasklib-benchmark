#include <chrono>
#include <sg/superglue.hpp>

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "sg/superglue.hpp"

#define RESOURCES_PER_TASK 4

using namespace std::chrono;

void sleep( nanoseconds d )
{
    auto end = high_resolution_clock::now() + d;
    while( high_resolution_clock::now() < end );
}

struct Options : public DefaultOptions<Options> {};

struct MyTask : Task<Options, RESOURCES_PER_TASK> {
    MyTask(
#if RESOURCES_PER_TASK >= 1
           Handle<Options> & h1
#endif
#if RESOURCES_PER_TASK >= 2
           , Handle<Options> & h2
#endif
#if RESOURCES_PER_TASK >= 4
           , Handle<Options> & h3
           , Handle<Options> & h4
#endif
#if RESOURCES_PER_TASK >= 8
           , Handle<Options> & h5
           , Handle<Options> & h6
           , Handle<Options> & h7
           , Handle<Options> & h8
#endif           
    )
    {
#if RESOURCES_PER_TASK >= 1
        register_access(ReadWriteAdd::write, h1);
#endif
#if RESOURCES_PER_TASK >= 2
        register_access(ReadWriteAdd::write, h2);
#endif
#if RESOURCES_PER_TASK >= 4
        register_access(ReadWriteAdd::write, h3);
        register_access(ReadWriteAdd::write, h4);
#endif
#if RESOURCES_PER_TASK >= 8
        register_access(ReadWriteAdd::write, h5);
        register_access(ReadWriteAdd::write, h6);
        register_access(ReadWriteAdd::write, h7);
        register_access(ReadWriteAdd::write, h8);
#endif        
    }

    void run()
    {
        std::cout << "test" << std::endl;
        sleep( nanoseconds(10000) );
    }
};

SuperGlue<Options> tm( 4 );
Handle<Options> *resources;

int main(int argc, char* argv[])
{
    unsigned n_resources = 5;
    unsigned n_tasks = 1000;
    unsigned n_threads = 8;

    resources = new Handle<Options>[n_resources];

    auto start = high_resolution_clock::now();
 
    for( int i = 0; i < n_tasks; ++i )
        tm.submit(new MyTask(
#if RESOURCES_PER_TASK >= 1
    resources[ rand()%n_resources ]
#endif
#if RESOURCES_PER_TASK >= 2
    , resources[ rand()%n_resources ]
#endif
#if RESOURCES_PER_TASK >= 4
    , resources[ rand()%n_resources ]
    , resources[ rand()%n_resources ]
#endif
#if RESOURCES_PER_TASK >= 8
    , resources[ rand()%n_resources ]
    , resources[ rand()%n_resources ]
    , resources[ rand()%n_resources ]
    , resources[ rand()%n_resources ]
#endif                      
        ));

    tm.barrier();
    
    auto end = high_resolution_clock::now();

    std::cout << "total " << duration_cast<microseconds>(end-start).count() << "μs" << std::endl;

    return 0;
}

