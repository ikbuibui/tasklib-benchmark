
#include <mutex>
#include <thread>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <quark.h>

using namespace std::chrono;

unsigned n_tasks = 1000;

std::condition_variable cv;
std::mutex m;
bool ready = false;

int main(int argc, char* argv[])
{
    if( argc > 1 )
        n_tasks = atoi(argv[1]);

    Quark * quark = QUARK_New(1);

    time_point<high_resolution_clock> start;

    QUARK_Insert_Task(quark,
                      [](Quark * quark) {
                          time_point<high_resolution_clock> * start;
                          quark_unpack_args_1( quark, start );

                          std::this_thread::sleep_for(seconds(1));

                          *start = high_resolution_clock::now();
                      },
                      NULL,
                      sizeof(time_point<high_resolution_clock>), &start, INOUT,
                      0);

    
    auto task_create_start = high_resolution_clock::now();

    for(unsigned i = 0; i < n_tasks; ++i)
        QUARK_Insert_Task(quark,
                          [](Quark * quark)
                          {
                          },
                      NULL,
                      sizeof(time_point<high_resolution_clock>), &start, INOUT,
                      0);

    auto task_create_end = high_resolution_clock::now();

    QUARK_Insert_Task(quark,
                      [](Quark * quark)
                      {
                          time_point<high_resolution_clock> * start;
                          quark_unpack_args_1( quark, start );

                          auto end = high_resolution_clock::now();
                          nanoseconds avg_deadtime = duration_cast<nanoseconds>(end - *start);
                          avg_deadtime /= (n_tasks+1);
                          std::cout << "avg deadtime: " << avg_deadtime.count()/1000.0 << " μs" << std::endl;
                      },
                      NULL,
                      sizeof(time_point<high_resolution_clock>), &start, INOUT,
                      0);

    ready = true;
    cv.notify_one();

    QUARK_Waitall(quark);

    std::cout << "avg task emplace time: " << duration_cast<nanoseconds>(task_create_end - task_create_start).count() /1000.0 << " μs" << std::endl;

    return 0;
}

