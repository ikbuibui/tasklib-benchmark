#include <chrono>
#include <vector>
#include <iostream>


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

    task_end[task_id] = high_resolution_clock::now();
}
void myTask1(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1;
    quark_unpack_args_2( quark, task_id, data1 );

    task_begin[task_id] = high_resolution_clock::now();

    sleep(task_duration[task_id]);
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
    hash(task_id, *data1);
    hash(task_id, *data2);
    hash(task_id, *data3);
    hash(task_id, *data4);
    hash(task_id, *data5);

    task_end[task_id] = high_resolution_clock::now();
}

int main(int argc, char* argv[])
{
    read_args(argc, argv);
    generate_access_pattern();

    Quark * quark = QUARK_New(n_workers);

    std::vector< std::array<uint64_t, 8> > resources(n_resources);

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

    std::cout << "total " << duration_cast<nanoseconds>(end-start).count()/1000.0 << " μs" << std::endl;
    std::cout << "emplacement " << duration_cast<nanoseconds>(mid-start).count()/1000.0 << " μs" << std::endl;
    std::cout << "scheduling gap " << duration_cast<nanoseconds>(get_scheduling_gap()).count() / 1000.0 << " μs" << std::endl;

    get_critical_path();
    
    return 0;
}


