#include "OrderBook.h"
#include <iostream>

int main() {
    OrderBook book;

    auto trades = book.addOrder(1, Side::Sell, 102, 10, 1);
    auto trades2 = book.addOrder(2, Side::Sell, 101, 5,  2);
    auto trades3 = book.addOrder(3, Side::Sell, 101, 8,  3);
    auto trades4 = book.addOrder(4, Side::Buy,  102, 12, 4);

    book.print();
    return 0;
}
