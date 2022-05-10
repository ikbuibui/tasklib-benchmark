#include <chrono>
#include <redGrapes/redGrapes.hpp>
#include <redGrapes/resource/ioresource.hpp>

#define RESOURCES_PER_TASK 0

namespace rg = redGrapes;
using namespace std::chrono;

void sleep( nanoseconds d )
{
    auto end = high_resolution_clock::now() + d;
    while( high_resolution_clock::now() < end );
}

int main(int argc, char* argv[])
{
    unsigned n_resources = 5;
    unsigned n_tasks = 1000;
    unsigned n_threads = 4;

    rg::init( n_threads );

    std::vector< rg::IOResource<int> > resources( n_resources );

    auto start = high_resolution_clock::now();    
    for( int i = 0; i < n_tasks; ++i )
        rg::emplace_task(
            [](
#if RESOURCES_PER_TASK >= 1
               auto ra1
#endif
#if RESOURCES_PER_TASK >= 2
               , auto ra2
#endif
#if RESOURCES_PER_TASK >= 4
               , auto ra3
               , auto ra4
#endif
#if RESOURCES_PER_TASK >= 8
               , auto ra5
               , auto ra6
               , auto ra7
               , auto ra8
#endif
            ) {
                sleep( nanoseconds(10000) );
            }
#if RESOURCES_PER_TASK >= 1
            , resources[ rand()%n_resources ].write()
#endif
#if RESOURCES_PER_TASK >= 2
            , resources[ rand()%n_resources ].write()
#endif
#if RESOURCES_PER_TASK >= 4
            , resources[ rand()%n_resources ].write()
            , resources[ rand()%n_resources ].write()
#endif
#if RESOURCES_PER_TASK >= 8
            , resources[ rand()%n_resources ].write()
            , resources[ rand()%n_resources ].write()
            , resources[ rand()%n_resources ].write()
            , resources[ rand()%n_resources ].write()
#endif
        );

    rg::barrier();
    auto end = high_resolution_clock::now();

    fmt::print("total {} μs\n", duration_cast<microseconds>(end-start).count());
    
    rg::finalize();
    
    return 0;
}


