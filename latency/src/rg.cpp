#include "common.h"

#include <rg.hpp>

#include <cstdint>
#include <iostream>

using namespace std::chrono;

unsigned n_threads = 1;
unsigned n_tasks = 10000;

auto latencyTest(rg::ThreadPool *ptr) -> rg::InitTask<int> {
  /* warmup */
  {
    for (unsigned i = 0; i < 64; ++i) {
      co_await rg::dispatch_task([]() -> rg::Task<int> { co_return 0; });
    }
    co_await rg::BarrierAwaiter{};
  }

  /* measure */
  nanoseconds avg_latency(0);

  for (unsigned i = 0; i < n_tasks; ++i) {
    auto start = high_resolution_clock::now();
    auto stop = co_await rg::dispatch_task([]() -> rg::Task<decltype(start)> {
      co_return high_resolution_clock::now();
    });

    avg_latency += duration_cast<nanoseconds>(co_await stop.get() - start);
  }

  avg_latency /= n_tasks;

  std::cout << "avg latency = " << avg_latency.count() / 1000.0 << " μs"
            << std::endl;
}

int main(int argc, char *argv[]) {

  auto poolObj = rg::init(n_threads);
  auto a = latencyTest(poolObj.pool_ptr());
  a.finalize();

  return 0;
}
