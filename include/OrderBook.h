#pragma once

#include "Order.h"
#include "PriceLevel.h"
#include "Trade.h"

#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

// Central limit order book.
//
// Bids: highest price has priority → stored in descending order (std::greater)
// Asks: lowest price has priority  → stored in ascending order (default)
//
// Matching is price-time priority: best price first, FIFO within a price level.
class OrderBook {
public:
  // Submit a new limit order.
  // If it crosses the opposite side, match it first (may produce trades).
  // Any remaining quantity rests in the book.
  // Returns the trades generated (may be empty).
  std::vector<Trade> addOrder(uint64_t id, Side side, uint64_t price,
                              uint64_t quantity, uint64_t timestamp);

  // Cancel a resting order by ID.
  // Returns true if found and cancelled, false if ID unknown.
  bool cancelOrder(uint64_t id);

  // Modify quantity of a resting order.
  // Decreasing quantity preserves queue position (in-place update).
  // Increasing quantity re-queues at the back via cancel + add (loses
  // priority). Returns false if ID unknown or newQuantity == 0.
  bool modifyOrder(uint64_t id, uint64_t newQuantity);

  // Best bid price. Returns 0 if book is empty on bid side.
  [[nodiscard]] uint64_t bestBid() const;

  // Best ask price. Returns 0 if book is empty on ask side.
  [[nodiscard]] uint64_t bestAsk() const;

  // Print a human-readable view of the book (for debugging).
  void print() const;

private:
  // Bids: descending price order
  std::map<uint64_t, PriceLevel, std::greater<>> bids_;

  // Asks: ascending price order
  std::map<uint64_t, PriceLevel> asks_;

  // Stable iterators into PriceLevel's list — O(1) lookup, cancel, and modify.
  // PriceLevel owns the Order objects; this map just indexes into them.
  std::unordered_map<uint64_t, PriceLevel::Iter> orderMap_;

  // Internal match loop — called by addOrder before resting.
  std::vector<Trade> match(Order &incoming);

  // Helper: remove an empty price level from the book.
  void pruneEmptyLevel(Side side, uint64_t price);

  // Compile-time side dispatch: select the right map, best-price, and crossing
  // predicate.
  template <Side S> auto &sideBook();
  template <Side S> [[nodiscard]] uint64_t bestPrice() const;
  template <Side S>
  [[nodiscard]] bool crosses(uint64_t ip, uint64_t bp) const;

  // Logic written once, instantiated for each side.
  template <Side S> void cancelSide(PriceLevel::Iter it);
  template <Side S> std::vector<Trade> matchSide(Order &incoming);
};
