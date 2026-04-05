#include "OrderBook.h"
#include <iostream>

int main() {
    OrderBook book;

    // A simple scenario to test with once you have addOrder working:
    //
    // Add some resting asks:
    //   SELL 10 @ 102
    //   SELL 5  @ 101
    //   SELL 8  @ 101
    //
    // Add a crossing bid:
    //   BUY 12 @ 102  → should match against 101 asks first, then 102
    //
    // Expected trades:
    //   trade: 5  @ 101  (hits first ask at 101)
    //   trade: 7  @ 101  (partially hits second ask at 101)
    //   BUY order rests with 0 remaining (fully filled)
    //
    // Uncomment and extend as you implement:

    auto trades = book.addOrder(1, Side::Sell, 102, 10, 1);
    auto trades2 = book.addOrder(2, Side::Sell, 101, 5,  2);
    auto trades3 = book.addOrder(3, Side::Sell, 101, 8,  3);
    auto trades4 = book.addOrder(4, Side::Buy,  102, 12, 4);

    book.print();
    return 0;
}
