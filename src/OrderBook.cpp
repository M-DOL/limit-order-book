#include "OrderBook.h"

#include <iostream>
using namespace std;

std::vector<Trade> OrderBook::addOrder(uint64_t id, Side side,
                                        uint64_t price, uint64_t quantity,
                                        uint64_t timestamp) {
    
    Order* newOrder = new Order{id, side, price, quantity, timestamp};
    auto trades = match(newOrder);
    if(newOrder->quantity) {
        orderMap_[id] = newOrder;
        (side == Side::Buy) ? bids_[price].addOrder(newOrder) : asks_[price].addOrder(newOrder);
    }
    return trades;
}

bool OrderBook::cancelOrder(uint64_t id) {
    if(!orderMap_.count(id)) {
        return false;
    }
    Order* order = orderMap_[id];
    if(order->side == Side::Buy) {
        PriceLevel* level = &bids_[order->price];
        level->removeOrder(order);
        if(level->isEmpty()) {
            pruneEmptyLevel(Side::Buy, order->price);
        }
    }
    else {
        PriceLevel* level = &asks_[order->price];
        level->removeOrder(order);
        if(level->isEmpty()) {
            pruneEmptyLevel(Side::Sell, order->price);
        }
    }
    orderMap_.erase(id);
    delete order;
    return true;
}

bool OrderBook::modifyOrder(uint64_t id, uint64_t newQuantity) {
    if(!orderMap_.count(id)) {
        return false;
    }
    if(!newQuantity) {
        cancelOrder(id);
        return false;
    }
    Order* order = orderMap_[id];
    if(order->quantity < newQuantity) {
        if(order->side == Side::Buy) {
            bids_[order->price].removeOrder(order);
            order->quantity = newQuantity;
            bids_[order->price].addOrder(order);
        }
        else {
            asks_[order->price].removeOrder(order);
            order->quantity = newQuantity;
            asks_[order->price].addOrder(order);
        }
    }
    else {
        if(order->side == Side::Buy) {
            bids_[order->price].adjustQuantity(newQuantity - order->quantity);
            order->quantity = newQuantity;
        }
        else {
            asks_[order->price].adjustQuantity(newQuantity - order->quantity);
            order->quantity = newQuantity;
        }
    }
    return true;
}

uint64_t OrderBook::bestBid() const {
    if(bids_.empty()) {
        return 0;
    }
    return bids_.begin()->first;
}

uint64_t OrderBook::bestAsk() const {
    if(asks_.empty()) {
        return 0;
    }
    return asks_.begin()->first;
}

void OrderBook::print() const {
    cout << "ASKS:" << '\n';
    for(auto& [price, level] : asks_) {
        cout << '\t' << price << " | qty: " << level.totalQuantity() << '\n';
    }
    if(asks_.empty()) {
        cout << "\tEMPTY\n";
    }
    cout << "BIDS:" << '\n';
    for(auto& [price, level] : bids_) {
        cout << '\t' << price << " | qty: " << level.totalQuantity() << '\n';
    }
    if(bids_.empty()) {
        cout << "\tEMPTY\n";
    }
}

std::vector<Trade> OrderBook::match(Order* incoming) {
    vector<Trade> trades;
    if(incoming->side == Side::Buy) {
        while(incoming->price >= bestAsk() && incoming->quantity) {
            PriceLevel* currLevel = &asks_[bestAsk()];
            while(!currLevel->isEmpty() && incoming->quantity) {
                Order* passive = currLevel->front();
                uint64_t tradeQuantity = min(incoming->quantity, passive->quantity);
                trades.emplace_back(Trade{incoming->id, passive->id, passive->price, tradeQuantity});
                incoming->quantity -= tradeQuantity;
                passive->quantity -= tradeQuantity;
                if(!passive->quantity) {
                    orderMap_.erase(passive->id);
                    asks_[bestAsk()].removeOrder(passive);
                    delete passive;
                }
            }
            pruneEmptyLevel(Side::Sell, bestAsk());
        }
    }
    else {
        while(incoming->price <= bestBid() && incoming->quantity) {
            PriceLevel* currLevel = &bids_[bestBid()];
            while(!currLevel->isEmpty() && incoming->quantity) {
                Order* passive = currLevel->front();
                uint64_t tradeQuantity = min(incoming->quantity, passive->quantity);
                trades.emplace_back(Trade{incoming->id, passive->id, passive->price, tradeQuantity});
                incoming->quantity -= tradeQuantity;
                passive->quantity -= tradeQuantity;
                if(!passive->quantity) {
                    orderMap_.erase(passive->id);
                    bids_[bestBid()].removeOrder(passive);
                    delete passive;
                }
            }
            pruneEmptyLevel(Side::Buy, bestBid());
        }
    }
    return trades;
}

void OrderBook::pruneEmptyLevel(Side side, uint64_t price) {
    (side == Side::Buy) ? bids_.erase(price) : asks_.erase(price); 
}
