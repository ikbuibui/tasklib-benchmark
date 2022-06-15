#include <chrono>
#include <vector>
#include <iostream>


#include <string.h> // need to include this to compile
#include "quark.h"
#include "common.h"

using namespace std::chrono;

void myTask0(Quark * quark)
{
    sleep( task_duration );
}
void myTask1(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1;
    quark_unpack_args_2( quark, task_id, data1 );

    sleep( task_duration );
    hash(task_id, *data1);
}
void myTask2(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1, *data2;
    quark_unpack_args_3( quark, task_id, data1, data2 );

    sleep( task_duration );
    hash(task_id, *data1);
    hash(task_id, *data2);
}
void myTask3(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1, *data2, *data3;
    quark_unpack_args_4( quark, task_id, data1, data2, data3 );

    sleep( task_duration );
    hash(task_id, *data1);
    hash(task_id, *data2);
    hash(task_id, *data3);
}
void myTask4(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1, *data2, *data3, *data4;
    quark_unpack_args_5( quark, task_id, data1, data2, data3, data4 );

    sleep( task_duration );
    hash(task_id, *data1);
    hash(task_id, *data2);
    hash(task_id, *data3);
    hash(task_id, *data4);
}
void myTask5(Quark * quark)
{
    unsigned task_id;
    std::array<uint64_t, 8> *data1, *data2, *data3, *data4, *data5;
    quark_unpack_args_6( quark, task_id, data1, data2, data3, data4, data5 );

    sleep( task_duration );
    hash(task_id, *data1);
    hash(task_id, *data2);
    hash(task_id, *data3);
    hash(task_id, *data4);
    hash(task_id, *data5);
}

int main(int argc, char* argv[])
{
    read_args(argc, argv);
    generate_access_pattern();

    Quark * quark = QUARK_New(n_threads);

    std::vector< std::array<uint64_t, 8> > resources(n_resources);

    auto start = high_resolution_clock::now();

    for( unsigned i = 0; i < n_tasks; ++i )
        switch( access_pattern[i].size() )
        {
        case 0:
            QUARK_Insert_Task(quark, myTask0, NULL, 0);
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

    QUARK_Waitall(quark);
    auto end = high_resolution_clock::now();

    std::cout << "total " << duration_cast<microseconds>(end-start).count() << " μs" << std::endl;

    for(int i = 0; i < n_resources; ++i)
        if(resources[i] != expected_hash[i])
        {
            std::cout << "error: invalid result!" << std::endl;
            return 1;
        }

    std::cout << "success" << std::endl;

    QUARK_Delete(quark);
    
    return 0;
}


