# Order Book

A C++ central limit order book implementing price-time priority matching for limit orders.

## Features

- Limit order add, cancel, and modify
- Price-time priority matching (FIFO within a price level)
- Multi-level, multi-order matching in a single aggressor sweep
- Modify preserves queue position on quantity decrease; loses priority on increase

## Design Decisions

### Integer prices
Prices are represented as `uint64_t` ticks rather than floating-point. Floating-point arithmetic introduces rounding error and comparison ambiguity that is unacceptable in a matching engine. All price logic is exact.

### `std::map` for price levels
Bids are stored in a `std::map<uint64_t, PriceLevel, std::greater<>>` (descending) and asks in a `std::map<uint64_t, PriceLevel>` (ascending). This gives O(log n) insert and erase by price, and O(1) best bid/ask access via `begin()` — the map's natural ordering keeps the best price at the front at all times.

### `std::list` for order queues
Orders within a price level are stored in a `std::list` to maintain FIFO priority. The key property is iterator stability: erasing a node from a `std::list` does not invalidate any other iterators or pointers, which makes O(1) cancel possible when iterators are cached. A `std::deque` would offer better cache performance for pure FIFO access, but arbitrary mid-queue removal (required for cancel) is O(n) with element shifting — unacceptable for a cancel-heavy workload.

### `std::unordered_map` for order lookup
A `std::unordered_map<uint64_t, PriceLevel::Iter>` maps order IDs to their stable `std::list` iterators, giving O(1) average-case lookup, cancel, and modify. `PriceLevel` owns the `Order` objects; this map is a pure index into them.

## Complexity

| Operation | Complexity |
|-----------|------------|
| `addOrder` (no match) | O(log n) — map insert by price |
| `addOrder` (with match) | O(k log n) — k trades across n price levels |
| `cancelOrder` | O(1) — iterator lookup + `list::erase` |
| `modifyOrder` (decrease) | O(1) — in-place field update |
| `modifyOrder` (increase) | O(log n) — cancel + re-insert |
| `bestBid` / `bestAsk` | O(1) |

## Build

Requires CMake 3.16+, C++17, and Google Benchmark (for the bench target).

```bash
cmake -S . -B build -Dbenchmark_DIR=/opt/homebrew/opt/google-benchmark/lib/cmake/benchmark
cmake --build build
./build/orderbook
```

To run benchmarks:
```bash
cmake --build build --target bench_orderbook
./build/bench_orderbook
```

To run linting:
```bash
cmake --build build --target lint
```

## Example

Resting orders: SELL 10 @ 102, SELL 5 @ 101, SELL 8 @ 101. Incoming: BUY 12 @ 102.

The buy crosses at 101 first (best ask), generating two trades:
- 5 qty @ 101 against the first resting sell
- 7 qty @ 101 against the second (fully consuming the incoming order)

The SELL 10 @ 102 remains untouched. Output:

```
ASKS:
	102 | qty: 10
BIDS:
	EMPTY
```