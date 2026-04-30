#include "PriceLevel.h"

PriceLevel::Iter PriceLevel::addOrder(Order order) {
    totalQuantity_ += order.quantity;
    orders_.push_back(std::move(order));
    return std::prev(orders_.end());
}

Order PriceLevel::removeOrder(Iter it) {
    Order o = std::move(*it);
    totalQuantity_ -= o.quantity;
    orders_.erase(it);
    return o;
}

Order& PriceLevel::front() {
    return orders_.front();
}

uint64_t PriceLevel::totalQuantity() const {
    return totalQuantity_;
}

void PriceLevel::adjustQuantity(int64_t delta) {
    totalQuantity_ += delta;
}

bool PriceLevel::isEmpty() const {
    return orders_.empty();
}
