#ifndef SYNCAIDE_PEER_H
#define SYNCAIDE_PEER_H

#include <boost/format.hpp>
#include <string>
#include <utility>
#include <iostream>

using namespace std;

class Peer {
public:
    Peer(const string &addr, int age);

    Peer(const Peer &peer);

    string addr() const;

    int age() const;

    void maturate();

    bool operator!=(const Peer &rhs);

    bool operator<(const Peer &rhs);

    friend bool operator<(Peer &lhs, Peer &rhs);

    friend bool operator<(const Peer &lhs, Peer &rhs);

    friend ostream &operator<<(ostream &stream, const Peer &rhs);

private:
    string _addr;
    int _age;
};

#endif //SYNCAIDE_PEER_H