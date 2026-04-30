#include "OrderBook.h"
#include <benchmark/benchmark.h>

// Benchmark: add non-crossing orders (they all rest in the book)
static void BM_AddOrders(benchmark::State &state) {
  for (auto _ : state) {
    OrderBook book;
    uint64_t id = 1;
    for (int i = 0; i < state.range(0); ++i) {
      book.addOrder(id, Side::Buy, 100 - (i % 10), 10, id);
      ++id;
      book.addOrder(id, Side::Sell, 110 + (i % 10), 10, id);
      ++id;
    }
    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_AddOrders)->Range(64, 4096);

// Benchmark: add a single aggressing order that matches N resting levels
static void BM_MatchSweep(benchmark::State &state) {
  int levels = state.range(0);
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    uint64_t id = 1;
    // Place one resting ask per price level (100, 101, ..., 100+levels-1)
    for (int i = 0; i < levels; ++i) {
      book.addOrder(id, Side::Sell, 100 + i, 10, id);
      ++id;
    }
    state.ResumeTiming();

    // Aggressor buy that crosses all resting asks
    book.addOrder(id, Side::Buy, 100 + levels, levels * 10, id);
    ++id;
    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_MatchSweep)->Range(8, 512);

// Benchmark: cancel orders by ID
static void BM_CancelOrders(benchmark::State &state) {
  auto n = static_cast<uint64_t>(state.range(0));
  for (auto _ : state) {
    state.PauseTiming();
    OrderBook book;
    for (uint64_t id = 1; id <= n; ++id) {
      book.addOrder(id, Side::Buy, 100 - (id % 10), 10, id);
    }
    state.ResumeTiming();

    for (uint64_t id = 1; id <= n; ++id) {
      book.cancelOrder(id);
    }
    benchmark::DoNotOptimize(book);
  }
}
BENCHMARK(BM_CancelOrders)->Range(64, 4096);

// Benchmark: bestBid / bestAsk hot path
static void BM_BestPriceQuery(benchmark::State &state) {
  OrderBook book;
  uint64_t id = 1;
  for (int i = 0; i < 256; ++i) {
    book.addOrder(id, Side::Buy, 100 - (i % 10), 10, id);
    ++id;
    book.addOrder(id, Side::Sell, 110 + (i % 10), 10, id);
    ++id;
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(book.bestBid());
    benchmark::DoNotOptimize(book.bestAsk());
  }
}
BENCHMARK(BM_BestPriceQuery);

BENCHMARK_MAIN();
