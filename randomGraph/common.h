#include <chrono>

#ifndef DEPENDENCIES_PER_TASK
#define DEPENDENCIES_PER_TASK 0
#endif

std::chrono::microseconds task_duration;
unsigned n_resources = 5;
unsigned n_tasks = 1000;
unsigned n_threads = 4;

void read_args(int argc, char *argv[]) {
  if (argc == 1)
    std::cout << "usage: " << argv[0]
	      << " [n_tasks = " << n_tasks
              << "] [n_resources = " << n_resources
              << "] [task_duration (μs) = " << task_duration.count()
              << "] [n_threads = " << n_threads << "]" << std::endl;

  if (argc > 1)
    n_tasks = atoi(argv[1]);
  if (argc > 2)
    n_resources = atoi(argv[2]);
  if (argc > 3)
    task_duration = std::chrono::microseconds(atoi(argv[3]) * 1000);
  if (argc > 4)
    n_threads = atoi(argv[4]);
}

void sleep(std::chrono::microseconds d) {
  auto end = std::chrono::high_resolution_clock::now() + d;
  while (std::chrono::high_resolution_clock::now() < end)
    ;
}

