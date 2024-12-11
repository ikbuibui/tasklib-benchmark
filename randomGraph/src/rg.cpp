#include "common.h"

#include <rg.hpp>

#include <condition_variable>
#include <cstdint>
#include <iomanip>
#include <mutex>

using namespace std::chrono;

std::mutex m;
std::condition_variable cv;
std::atomic<bool> start_flag{false};

auto randomGraph(rg::ThreadPool *ptr) -> rg::InitTask<int> {
  std::vector<rg::Resource<std::shared_ptr<std::array<uint64_t, 8>>>> resources(
      n_resources);

  for (auto &res : resources) {
    res = rg::Resource(std::make_shared<std::array<uint64_t, 8>>());
  }

  if (block_execution) {
    std::atomic_int count(0);
    if (n_workers != n_resources) {
      std::cout << "workers != resources, blocking is probably broken"
                << std::endl;
    }

    for (unsigned i = 0; i < n_workers; ++i) {

      co_await rg::dispatch_task(
          [](auto i, auto &count, auto blockRes) -> rg::Task<void> {
            wait_task_begin[i] = steady_clock::now();
            wait_task_thread[i] = std::this_thread::get_id();

            count.fetch_add(1);

            // block this worker until start flag
            while (!start_flag)
              ;
            wait_task_end[i] = steady_clock::now();
            co_return;
          },
          i, count, resources[i].rg_write());
    }
    // wait until all block-tasks are up and running
    int last_count = count;
    while (count < n_workers) {

      if (last_count != count)
        std::cout << count << std::endl;

      last_count = count;
    }
  }

  auto start = steady_clock::now();

  for (int i = 0; i < n_tasks; ++i) {
    switch (access_pattern[i].size()) {
    case 0:
      co_await rg::dispatch_task(
          [](auto i) -> rg::Task<void> {
            task_begin[i] = steady_clock::now();

            task_thread[i] = std::this_thread::get_id();
            sleep(task_duration[i]);

            task_end[i] = steady_clock::now();
            co_return;
          },
          i);
      break;

    case 1:
      co_await rg::dispatch_task(
          [](auto ra1, auto i) -> rg::Task<void> {
            task_begin[i] = steady_clock::now();

            // spdlog::info("task {}, res {}", i, access_pattern[i][0]);
            sleep(task_duration[i]);
            task_thread[i] = std::this_thread::get_id();
            hash(i, *ra1);

            task_end[i] = steady_clock::now();
            co_return;
          },
          resources[access_pattern[i][0]].rg_write(), i);

      break;

    case 2:
      co_await rg::dispatch_task(
          [](auto ra1, auto ra2, auto i) -> rg::Task<void> {
            task_begin[i] = steady_clock::now();

            sleep(task_duration[i]);
            task_thread[i] = std::this_thread::get_id();
            hash(i, *ra1);
            hash(i, *ra2);

            task_end[i] = steady_clock::now();
            co_return;
          },
          resources[access_pattern[i][0]].rg_write(),
          resources[access_pattern[i][1]].rg_write(), i);
      break;

    case 3:
      co_await rg::dispatch_task(
          [](auto ra1, auto ra2, auto ra3, auto i) -> rg::Task<void> {
            task_begin[i] = steady_clock::now();

            sleep(task_duration[i]);
            task_thread[i] = std::this_thread::get_id();

            hash(i, *ra1);
            hash(i, *ra2);
            hash(i, *ra3);

            task_end[i] = steady_clock::now();
            co_return;
          },
          resources[access_pattern[i][0]].rg_write(),
          resources[access_pattern[i][1]].rg_write(),
          resources[access_pattern[i][2]].rg_write(), i);
      break;

    case 4:
      co_await rg::dispatch_task(
          [](auto ra1, auto ra2, auto ra3, auto ra4, auto i) -> rg::Task<void> {
            task_begin[i] = steady_clock::now();

            sleep(task_duration[i]);
            task_thread[i] = std::this_thread::get_id();

            hash(i, *ra1);
            hash(i, *ra2);
            hash(i, *ra3);
            hash(i, *ra4);

            task_end[i] = steady_clock::now();
            co_return;
          },
          resources[access_pattern[i][0]].rg_write(),
          resources[access_pattern[i][1]].rg_write(),
          resources[access_pattern[i][2]].rg_write(),
          resources[access_pattern[i][3]].rg_write(), i);
      break;

    case 5:
      co_await rg::dispatch_task(
          [](auto ra1, auto ra2, auto ra3, auto ra4, auto ra5,
             auto i) -> rg::Task<void> {
            task_begin[i] = steady_clock::now();

            sleep(task_duration[i]);
            task_thread[i] = std::this_thread::get_id();

            hash(i, *ra1);
            hash(i, *ra2);
            hash(i, *ra3);
            hash(i, *ra4);
            hash(i, *ra5);

            task_end[i] = steady_clock::now();
            co_return;
          },
          resources[access_pattern[i][0]].rg_write(),
          resources[access_pattern[i][1]].rg_write(),
          resources[access_pattern[i][2]].rg_write(),
          resources[access_pattern[i][3]].rg_write(),
          resources[access_pattern[i][4]].rg_write(), i);
      break;
    }
  }
  auto mid = steady_clock::now();

  if (block_execution) {
    // spdlog::info("+++ emplacement done, start executing tasks...");
    // trigger execution of tasks
    start_flag = true;
  }

  // wait for execution to finish
  co_await rg::BarrierAwaiter{};

  auto end = steady_clock::now();

  for (int i = 0; i < n_resources; ++i) {
    if (*resources[i].get() != expected_hash[i]) {
      std::cout << "error: invalid result!" << std::endl;
      co_return -1;
    }
  }

  std::cout << "success" << std::endl;

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "total "
            << duration_cast<nanoseconds>(end - start).count() / 1000.0 << " μs"
            << std::endl;
  std::cout << "emplacement "
            << duration_cast<nanoseconds>(mid - start).count() / 1000.0 << " μs"
            << std::endl;
  std::cout << "execution "
            << duration_cast<nanoseconds>(end - mid).count() / 1000.0 << " μs"
            << std::endl;
  std::cout << "scheduling gap "
            << duration_cast<nanoseconds>(get_scheduling_gap()).count() / 1000.0
            << " μs" << std::endl;

  get_critical_path();

  output_svg(std::ofstream("trace_redgrapes.svg"));
}

int main(int argc, char *argv[]) {
  // spdlog::set_level(spdlog::level::trace);
  // spdlog::set_pattern("[thread %t] %^[%l]%$ %v");

  read_args(argc, argv);
  generate_access_pattern();

  auto poolObj = rg::init(n_workers);
  auto a = randomGraph(poolObj.pool_ptr());
  a.finalize();

  return 0;
}
