#pragma once

#include "Order.h"
#include <list>
#include <cstdint>

// All resting orders at a single price point.
// Orders are stored in FIFO order: price-time priority.
// std::list gives O(1) insert at back and O(1) erase anywhere
// given an iterator, which is how cancel will work.
class PriceLevel {
public:
    // Add a new order to the back of the queue.
    void addOrder(Order* order);

    // Remove a specific order by pointer.
    void removeOrder(Order* order);

    // Total quantity resting at this price level.
    uint64_t totalQuantity() const;

    void adjustQuantity(int64_t delta);

    // True if no orders remain at this level.
    bool isEmpty() const;

    // Access the front order (first to be matched).
    Order* front();

private:
    std::list<Order*> orders_;
    uint64_t          totalQuantity_ = 0;
};
