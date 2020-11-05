#define LOG_TAG "BENCH"
#include "simple_logger.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <iosfwd>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

/* Returns microseconds since epoch */
uint64_t timestamp_now()
{
    return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
}

void slog_benchmark()
{
    int const iterations = 100000;
    char const* const benchmark = "benchmark";
    uint64_t begin = timestamp_now();
    for (int i = 0; i < iterations; ++i)
        slog_i("Logging %s %d %d %s %.2f", "Logging", i, 0, "K", -42.42);
    uint64_t end = timestamp_now();
    long int avg_latency = (end - begin) * 1000 / iterations;
    printf("\tAverage SimpleLogger Latency = %ld nanoseconds\n", avg_latency);
}

template <typename Function>
void run_benchmark(Function&& f, int thread_count)
{
    printf("Thread count: %d\n", thread_count);
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(std::thread(f));
    }
    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }
}

int main()
{
    // does not print log to terminal
    slog_set_stdout_redirection(NULL);
    // disable color output
    slog_set_color_output(0);
    // save log to file
    slog_set_output_to_file(1);
    // start logger
    slog_start();
    // run benchmark
    for (auto threads : { 1,2,3,4,5 })
        run_benchmark(slog_benchmark, threads);
    // stop logger
    slog_stop();
    return 0;
}
