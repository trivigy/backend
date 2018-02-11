#include "logging.h"
#include "view.h"

Peer View::random_peer() {
    if (!_peers.empty()) {
        shared_lock<shared_mutex> lock(_mutex);
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, (int) _peers.size() - 1);
        int idx = dis(gen);
        return _peers[idx];
    }
}

bool View::empty() {
    shared_lock<shared_mutex> lock(_mutex);
    return _peers.empty();
}

int View::size() {
    shared_lock<shared_mutex> lock(_mutex);
    return (int) _peers.size();
}

deque<Peer> View::snapshot() {
    shared_lock<shared_mutex> lock(_mutex);
    return _peers;
}

deque<Peer> View::select(int c, int H) {
    // TODO queue based peer sampling for purpose of fairness.
    unique_lock<shared_mutex> lock(_mutex);
    sort();
    shuffle(H);
    return head(c);
}

void View::update(int c, int H, int S, deque<Peer> &buffer) {
    unique_lock<shared_mutex> lock(_mutex);
    append(buffer);
    remove_duplicates();
    remove_old(min(H, (int) _peers.size() - c));
    remove_head(min(S, (int) _peers.size() - c));
    remove_random((int) _peers.size() - c);
    maturate();
}

void View::append(deque<Peer> &buffer) {
    copy(buffer.begin(), buffer.end(), back_inserter(_peers));
}

void View::sort() {
    stable_sort(_peers.begin(), _peers.end());
}

void View::shuffle(int num) {
    if (_peers.size() > num) {
        int diff = (int) _peers.size() - num;
        random_device rd;
        mt19937 gen(rd());
        std::shuffle(_peers.begin(), _peers.begin() + diff, gen);
    }
}

deque<Peer> View::head(int num) {
    deque<Peer> buffer;
    num = min(num, (int) _peers.size());
    copy(_peers.begin(), _peers.begin() + num, back_inserter(buffer));
    return buffer;
}

void View::remove_duplicates() {
    unordered_map<string, list<Peer>::iterator> unique;
    list<Peer> deduplicated;

    for (auto &peer : _peers) {
        auto search = unique.find(peer.addr());
        if (search != unique.end()) {
            if (search->second->age() >= peer.age()) {
                deduplicated.erase(search->second);
                unique.erase(search);
                deduplicated.emplace_back(peer);
                unique[peer.addr()] = --deduplicated.end();
            }
        } else {
            deduplicated.emplace_back(peer);
            unique[peer.addr()] = --deduplicated.end();
        }
    }

    _peers.clear();
    copy(deduplicated.begin(), deduplicated.end(), back_inserter(_peers));
}

void View::remove_old(int qty) {
    // Important point due on how std::priority_queue operates: if multple
    // peers in the view have the same age, then those that are closer to
    // the end of the list will be removed first.
    auto cmp = [](list<Peer>::iterator lhs, list<Peer>::iterator rhs) {
        return lhs->age() < rhs->age();
    };
    list<Peer> newest;
    std::priority_queue<
        list<Peer>::iterator,
        vector<list<Peer>::iterator>,
        decltype(cmp)
    > pq(cmp);

    for (auto &peer : _peers) {
        newest.emplace_back(peer);
        pq.push(--newest.end());
    }

    qty = (qty < 0) ? 0 : qty;
    while (!pq.empty() && qty > 0) {
        auto peer = pq.top();
        pq.pop();
        newest.erase(peer);
        qty--;
    }

    _peers.clear();
    copy(newest.begin(), newest.end(), back_inserter(_peers));
}

void View::remove_head(int qty) {
    qty = (qty < 0) ? 0 : qty;
    while (!_peers.empty() && qty > 0) {
        _peers.pop_front();
        qty--;
    }
}

void View::remove_random(int qty) {
    random_device rd;
    mt19937 gen(rd());
    qty = (qty < 0) ? 0 : qty;
    while (!_peers.empty() && qty > 0) {
        uniform_int_distribution<> dis(0, (int) _peers.size() - 1);
        _peers.erase(_peers.begin() + dis(gen));
        qty--;
    }
}

void View::maturate() {
    for (auto &peer : _peers) {
        peer.maturate();
    }
}