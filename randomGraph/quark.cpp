#include <chrono>
#include <vector>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iomanip>
#include <string.h> // need to include this to compile
#include "quark.h"
#include "common.h"

using namespace std::chrono;

void myTask0(Quark * quark)
{
    unsigned task_id;
    quark_unpack_args_1( quark, task_id );

    task_begin[task_id] = high_resolution_clock::now();
    sleep(task_duration[task_id]);
    task_thread[task_id] = std::this_thread::get_id();

    task_end[task_id] = high_resolution_clock::now();
}
void myTask1(Quark * quark)
{
    auto begin = high_resolution_clock::now();

    unsigned task_id;
    std::array<uint64_t, 8> *data1;
    quark_unpack_args_2( quark, task_id, data1 );

    task_begin[task_id] = begin;

    sleep(task_duration[task_id]);
    task_thread[task_id] = std::this_thread::get_id();
    hash(task_id, *data1);

    task_end[task_id] = high_resolution_clock::now();
}
void myTask2(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1, *data2;
    quark_unpack_args_3( quark, task_id, data1, data2 );

    task_begin[task_id] = high_resolution_clock::now();

    sleep(task_duration[task_id]);
    task_thread[task_id] = std::this_thread::get_id();
    hash(task_id, *data1);
    hash(task_id, *data2);

    task_end[task_id] = high_resolution_clock::now();
}
void myTask3(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1, *data2, *data3;
    quark_unpack_args_4( quark, task_id, data1, data2, data3 );

    task_begin[task_id] = high_resolution_clock::now();

    sleep(task_duration[task_id]);
    task_thread[task_id] = std::this_thread::get_id();
    hash(task_id, *data1);
    hash(task_id, *data2);
    hash(task_id, *data3);

    task_end[task_id] = high_resolution_clock::now();
}
void myTask4(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1, *data2, *data3, *data4;
    quark_unpack_args_5( quark, task_id, data1, data2, data3, data4 );

    task_begin[task_id] = high_resolution_clock::now();
    
    sleep(task_duration[task_id]);
    task_thread[task_id] = std::this_thread::get_id();
    hash(task_id, *data1);
    hash(task_id, *data2);
    hash(task_id, *data3);
    hash(task_id, *data4);

    task_end[task_id] = high_resolution_clock::now();
}
void myTask5(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1, *data2, *data3, *data4, *data5;
    quark_unpack_args_6( quark, task_id, data1, data2, data3, data4, data5 );

    task_begin[task_id] = high_resolution_clock::now();

    sleep(task_duration[task_id]);
    task_thread[task_id] = std::this_thread::get_id();
    hash(task_id, *data1);
    hash(task_id, *data2);
    hash(task_id, *data3);
    hash(task_id, *data4);
    hash(task_id, *data5);

    task_end[task_id] = high_resolution_clock::now();
}

std::mutex m;
std::condition_variable cv;
volatile bool start_flag = false;

void blocking_task(Quark * quark)
{
    unsigned i;
    quark_unpack_args_1( quark, i );

    wait_task_begin[i] = high_resolution_clock::now();
    wait_task_thread[i] = std::this_thread::get_id();

    /*
    std::unique_lock<std::mutex> l(m);
    cv.wait(l, [] {
        return start_flag;
    });
    */
    while( !start_flag );

    wait_task_end[i] = high_resolution_clock::now();
}

int main(int argc, char* argv[])
{
    read_args(argc, argv);
    generate_access_pattern();

    Quark * quark = QUARK_New(n_workers);

    std::vector< std::array<uint64_t, 8> > resources( n_resources );

    if( block_execution )
    {
        for( unsigned i = 0; i < n_workers; ++i )
        {
            Quark_Task_Flags tflags = Quark_Task_Flags_Initializer;
            QUARK_Task_Flag_Set( &tflags, TASK_LOCK_TO_THREAD, i );
            QUARK_Insert_Task(quark, blocking_task, &tflags,
                              sizeof(unsigned), &i, VALUE,
                              0);
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(500) );
    }
    
    auto start = high_resolution_clock::now();

    for( unsigned i = 0; i < n_tasks; ++i )
        switch( access_pattern[i].size() )
        {
        case 0:
            QUARK_Insert_Task(quark, myTask0, NULL,
                              sizeof(unsigned), &i, VALUE,
                              0);
            break;
        case 1:
            QUARK_Insert_Task(quark, myTask1, NULL,
                              sizeof(unsigned), &i, VALUE,
                              sizeof(std::array<uint64_t,8>), &resources[ access_pattern[i][0] ], INOUT,
                              0);
            break;
        case 2:
            QUARK_Insert_Task(quark, myTask2, NULL,
                              sizeof(unsigned), &i, VALUE,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][0] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][1] ], INOUT,
                              0);
            break;
        case 3:
            QUARK_Insert_Task(quark, myTask3, NULL,
                              sizeof(unsigned), &i, VALUE,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][0] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][1] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][2] ], INOUT,
                              0);
            break;
        case 4:
            QUARK_Insert_Task(quark, myTask4, NULL,
                              sizeof(unsigned), &i, VALUE,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][0] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][1] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][2] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][3] ], INOUT,
                              0);
            break;
        case 5:
            QUARK_Insert_Task(quark, myTask5, NULL,
                              sizeof(unsigned), &i, VALUE,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][0] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][1] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][2] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][3] ], INOUT,
                              sizeof(std::array<uint64_t, 8>), &resources[ access_pattern[i][4] ], INOUT,
                              0);
            break;
        }

    auto mid = high_resolution_clock::now();

    if( block_execution )
    {
        // trigger execution of tasks
        //std::cout << "emplacement done, start tasks" << std::endl;
        {
            //std::unique_lock<std::mutex> l(m);
            start_flag = true;
        }
        //cv.notify_all();
    }

    QUARK_Waitall(quark);

    auto end = high_resolution_clock::now();

    QUARK_Delete(quark);

    for(int i = 0; i < n_resources; ++i)
        if(resources[i] != expected_hash[i])
        {
            std::cout << "error: invalid result!" << std::endl;
            return 1;
        }

    std::cout << "success" << std::endl;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "total " << duration_cast<nanoseconds>(end-start).count()/1000.0 << " μs" << std::endl;
    std::cout << "emplacement " << duration_cast<nanoseconds>(mid-start).count()/1000.0 << " μs" << std::endl;
    std::cout << "execution " << duration_cast<nanoseconds>(end-mid).count()/1000.0 << " μs" << std::endl;
    std::cout << "scheduling gap " << duration_cast<nanoseconds>(get_scheduling_gap()).count() / 1000.0 << " μs" << std::endl;

    get_critical_path();
    output_svg(std::ofstream("trace_quark.svg"));
    return 0;
}


