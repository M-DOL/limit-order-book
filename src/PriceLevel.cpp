#include "PriceLevel.h"
#include <algorithm>

void PriceLevel::addOrder(Order *order)
{
    orders_.push_back(order);
    totalQuantity_ += order->quantity;
}

// Remove a specific order by pointer.
void PriceLevel::removeOrder(Order *order)
{
    orders_.erase(std::find(orders_.begin(), orders_.end(), order));
    totalQuantity_ -= order->quantity;
}

void PriceLevel::adjustQuantity(int64_t delta) {
    totalQuantity_ += delta;
}

// Total quantity resting at this price level.
uint64_t PriceLevel::totalQuantity() const
{
    return totalQuantity_;
}

// True if no orders remain at this level.
bool PriceLevel::isEmpty() const
{
    return orders_.empty();
}

// Access the front order (first to be matched).
Order *PriceLevel::front()
{
    return orders_.front();
}