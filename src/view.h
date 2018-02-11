#ifndef SYNC_VIEW_H
#define SYNC_VIEW_H

#include "peer.h"

#include <iostream>
#include <deque>
#include <utility>
#include <random>
#include <string>
#include <iterator>
#include <algorithm>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <list>
#include <unordered_map>
#include <queue>

using namespace std;

class View {
public:
    Peer random_peer();

    bool empty();

    int size();

    deque<Peer> snapshot();

    deque<Peer> select(int c, int H);

    void update(int c, int H, int S, deque<Peer> &buffer);

protected:
    void append(deque<Peer> &buffer);

    void sort();

    void shuffle(int num);

    deque<Peer> head(int num);

    void remove_duplicates();

    void remove_old(int qty);

    void remove_head(int qty);

    void remove_random(int qty);

    void maturate();

private:
    mutable shared_mutex _mutex;
    deque<Peer> _peers;
};

#endif //SYNC_VIEW_H