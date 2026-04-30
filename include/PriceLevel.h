#pragma once

#include "Order.h"
#include <cstdint>
#include <list>

// All resting orders at a single price point, stored by value.
// addOrder returns a stable iterator; callers (OrderBook) store it in orderMap_
// for O(1) cancel and modify without any additional heap allocation.
class PriceLevel {
public:
  using Iter = std::list<Order>::iterator;

  Iter addOrder(Order order); // O(1); returns stable iterator
  Order removeOrder(Iter it); // O(1); returns Order so modify can reuse it
  Order &front();
  uint64_t totalQuantity() const;
  void reduceQuantity(uint64_t amount);
  bool isEmpty() const;

private:
  std::list<Order> orders_;
  uint64_t totalQuantity_ = 0;
};
