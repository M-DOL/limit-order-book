#include "OrderBook.h"

#include <iostream>
using namespace std;

template <Side S>
constexpr Side OppV = (S == Side::Buy) ? Side::Sell : Side::Buy;

// ── Compile-time side helpers ────────────────────────────────────────────────

template <Side S>
auto& OrderBook::sideBook() {
    if constexpr (S == Side::Buy) return bids_;
    else                          return asks_;
}

template <Side S>
uint64_t OrderBook::bestPrice() const {
    if constexpr (S == Side::Buy) return bestBid();
    else                          return bestAsk();
}

template <Side S>
bool OrderBook::crosses(uint64_t ip, uint64_t bp) const {
    if constexpr (S == Side::Buy) return ip >= bp;
    else                          return ip <= bp;
}

// ── cancelSide / matchSide ───────────────────────────────────────────────────

template <Side S>
void OrderBook::cancelSide(PriceLevel::Iter it) {
    uint64_t price = it->price;
    PriceLevel& level = sideBook<S>()[price];
    level.removeOrder(it);
    if (level.isEmpty())
        pruneEmptyLevel(S, price);
}

template <Side S>
std::vector<Trade> OrderBook::matchSide(Order& incoming) {
    std::vector<Trade> trades;
    while (crosses<S>(incoming.price, bestPrice<OppV<S>>()) && incoming.quantity) {
        uint64_t best     = bestPrice<OppV<S>>();
        PriceLevel& level = sideBook<OppV<S>>()[best];
        while (!level.isEmpty() && incoming.quantity) {
            Order& passive   = level.front();
            uint64_t qty     = min(incoming.quantity, passive.quantity);
            trades.emplace_back(Trade{incoming.id, passive.id, passive.price, qty});
            incoming.quantity -= qty;
            passive.quantity  -= qty;
            if (!passive.quantity) {
                auto listIt = orderMap_[passive.id];
                orderMap_.erase(passive.id);  // remove index entry
                level.removeOrder(listIt);    // O(1); destroys Order, passive dangling
            }
        }
        pruneEmptyLevel(OppV<S>, best);
    }
    return trades;
}

// ── Public interface ─────────────────────────────────────────────────────────

std::vector<Trade> OrderBook::addOrder(uint64_t id, Side side,
                                        uint64_t price, uint64_t quantity,
                                        uint64_t timestamp) {
    Order incoming{id, side, price, quantity, timestamp};
    auto trades = match(incoming);
    if (incoming.quantity) {
        PriceLevel::Iter it = (side == Side::Buy)
            ? bids_[price].addOrder(std::move(incoming))
            : asks_[price].addOrder(std::move(incoming));
        orderMap_[id] = it;
    }
    return trades;
}

bool OrderBook::cancelOrder(uint64_t id) {
    auto mapIt = orderMap_.find(id);
    if (mapIt == orderMap_.end()) return false;
    PriceLevel::Iter listIt = mapIt->second;
    Side side = listIt->side;
    orderMap_.erase(mapIt);
    (side == Side::Buy) ? cancelSide<Side::Buy>(listIt)
                        : cancelSide<Side::Sell>(listIt);
    return true;
}

bool OrderBook::modifyOrder(uint64_t id, uint64_t newQuantity) {
    auto mapIt = orderMap_.find(id);
    if (mapIt == orderMap_.end()) return false;
    if (!newQuantity) {
        cancelOrder(id);
        return false;
    }
    PriceLevel::Iter listIt = mapIt->second;
    Order& order = *listIt;
    if (order.quantity < newQuantity) {
        // Increase: re-queue at back (loses priority)
        uint64_t price = order.price;
        if (order.side == Side::Buy) {
            Order o = bids_[price].removeOrder(listIt);
            o.quantity = newQuantity;
            mapIt->second = bids_[price].addOrder(std::move(o));
        } else {
            Order o = asks_[price].removeOrder(listIt);
            o.quantity = newQuantity;
            mapIt->second = asks_[price].addOrder(std::move(o));
        }
    } else {
        // Decrease: update in place (preserves queue position)
        int64_t delta = (int64_t)newQuantity - (int64_t)order.quantity;
        order.quantity = newQuantity;
        if (order.side == Side::Buy)
            bids_[order.price].adjustQuantity(delta);
        else
            asks_[order.price].adjustQuantity(delta);
    }
    return true;
}

uint64_t OrderBook::bestBid() const {
    return bids_.empty() ? 0 : bids_.begin()->first;
}

uint64_t OrderBook::bestAsk() const {
    return asks_.empty() ? 0 : asks_.begin()->first;
}

void OrderBook::print() const {
    cout << "ASKS:\n";
    for (auto& [price, level] : asks_)
        cout << '\t' << price << " | qty: " << level.totalQuantity() << '\n';
    if (asks_.empty()) cout << "\tEMPTY\n";
    cout << "BIDS:\n";
    for (auto& [price, level] : bids_)
        cout << '\t' << price << " | qty: " << level.totalQuantity() << '\n';
    if (bids_.empty()) cout << "\tEMPTY\n";
}

std::vector<Trade> OrderBook::match(Order& incoming) {
    return (incoming.side == Side::Buy) ? matchSide<Side::Buy>(incoming)
                                        : matchSide<Side::Sell>(incoming);
}

void OrderBook::pruneEmptyLevel(Side side, uint64_t price) {
    (side == Side::Buy) ? bids_.erase(price) : asks_.erase(price);
}
