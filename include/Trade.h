#pragma once

#include <cstdint>

// Produced whenever two orders cross.
struct Trade {
  uint64_t aggressorId; // the incoming order that triggered the match
  uint64_t passiveId;   // the resting order that got hit
  uint64_t price;       // price of the passive (resting) order
  uint64_t quantity;    // quantity traded
};
