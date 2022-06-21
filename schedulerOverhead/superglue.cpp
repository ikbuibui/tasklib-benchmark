
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <sg/superglue.hpp>

using namespace std::chrono;

unsigned n_tasks = 1000;

time_point<high_resolution_clock> start;
std::condition_variable cv;
std::mutex m;
bool ready = false;

struct Options : public DefaultOptions<Options> {};

struct StartTask : Task<Options, 1> {
    StartTask(Handle<Options> & h0)
    {
        register_access(ReadWriteAdd::write, h0);
    }

    void run()
    {
        std::unique_lock<std::mutex> l(m);
        cv.wait(l, []{ return ready; });

        start = high_resolution_clock::now();
    }
};

struct EmptyTask : Task<Options, 1> {
    EmptyTask(Handle<Options> & h0)
    {
        register_access(ReadWriteAdd::write, h0);
    }

    void run()
    {
    }
};

struct EndTask : Task<Options, 1> {
    EndTask(Handle<Options> & h0)
    {
        register_access(ReadWriteAdd::write, h0);
    }

    void run()
    {
        auto end = high_resolution_clock::now();
        nanoseconds avg_deadtime = duration_cast<nanoseconds>(end - start);
        avg_deadtime /= (n_tasks+1);
        std::cout << "avg deadtime: " << avg_deadtime.count()/1000.0 << " ms" << std::endl;
    }
};

int main(int argc, char* argv[])
{
    if( argc > 1 )
        n_tasks = atoi(argv[1]);

    SuperGlue<Options> sg(1);

    Handle<Options> h0;

    /* warmup */
    {
        for(unsigned i = 0; i < 64; ++i)
            sg.submit(new EmptyTask(h0));
        sg.barrier();
    }

    /* measure */
    sg.submit(new StartTask(h0));

    auto task_create_start = high_resolution_clock::now();
    for(unsigned i = 0; i < n_tasks; ++i)
        sg.submit(new EmptyTask(h0));

    auto task_create_end = high_resolution_clock::now();

    sg.submit(new EndTask(h0));

    {
        std::unique_lock<std::mutex> l(m);
        ready = true;
    }
    cv.notify_one();

    sg.barrier();

    auto avg_task_create_overhead = duration_cast<nanoseconds>(task_create_end - task_create_start) / n_tasks;
    std::cout << "avg task emplace time: " << avg_task_create_overhead.count()/1000.0 << " μs" << std::endl;

    return 0;
}

