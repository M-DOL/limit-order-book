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
Bids are stored in a `std::map<uint64_t, PriceLevel, std::greater<uint64_t>>` (descending) and asks in a `std::map<uint64_t, PriceLevel>` (ascending). This gives O(log n) insert and erase by price, and O(1) best bid/ask access via `begin()` — the map's natural ordering keeps the best price at the front at all times.

### `std::list` for order queues
Orders within a price level are stored in a `std::list` to maintain FIFO priority. The key property is iterator stability: erasing a node from a `std::list` does not invalidate any other iterators or pointers, which makes O(1) cancel possible when iterators are cached. A `std::deque` would offer better cache performance for pure FIFO access, but arbitrary mid-queue removal (required for cancel) is O(n) with element shifting — unacceptable for a cancel-heavy workload.

### `std::unordered_map` for order lookup
A `std::unordered_map<uint64_t, Order*>` maps order IDs to order pointers, giving O(1) average-case lookup for cancel and modify. Order IDs are integers, so hashing is cheap with negligible collision probability in practice.

## Complexity

| Operation | Complexity |
|-----------|------------|
| `addOrder` (no match) | O(log n) — map insert by price |
| `addOrder` (with match) | O(k log n) — k trades across n price levels |
| `cancelOrder` | O(m) — linear scan within price level of size m |
| `modifyOrder` (decrease) | O(m) — linear scan for cancel path |
| `modifyOrder` (increase) | O(m) — remove + re-insert |
| `bestBid` / `bestAsk` | O(1) |

> Note: cancel and modify become O(1) within the price level if order insertion iterators are cached in a side map — a straightforward extension not yet implemented.

## Build

Requires C++17 and g++:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude src/PriceLevel.cpp src/OrderBook.cpp src/main.cpp -o orderbook
./orderbook
```

## Example

```
// Resting orders:
// SELL 10 @ 102
// SELL 5  @ 101
// SELL 8  @ 101
//
// Incoming: BUY 12 @ 102
//
// Trades generated:
//   [trade] aggressor=4 passive=2 price=101 qty=5
//   [trade] aggressor=4 passive=3 price=101 qty=7
//
// Remaining book:
// ASKS:
//     102 | qty: 10
// BIDS:
//     (empty)
```