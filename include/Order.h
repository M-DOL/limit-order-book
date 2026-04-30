#pragma once

#include <cstdint>

enum class Side : uint8_t { Buy, Sell };

// A single resting limit order.
// All logic lives in PriceLevel and OrderBook.
struct Order {
  uint64_t id;
  Side side;
  uint64_t price;    // price in ticks (integer — no floats in HFT)
  uint64_t quantity; // remaining quantity
  uint64_t timestamp;
};
