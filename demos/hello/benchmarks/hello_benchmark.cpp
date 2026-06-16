#include <benchmark/benchmark.h>

static void BM_HelloIncrement(benchmark::State& state) {
    for (auto _ : state) {
        int value = 1;
        benchmark::DoNotOptimize(value += 1);
    }
}

BENCHMARK(BM_HelloIncrement);
BENCHMARK_MAIN();
