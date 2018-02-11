#include "peer.h"

Peer::Peer(const string &addr, int age) {
    _addr = addr;
    _age = age;
}

Peer::Peer(const Peer &peer) {
    _addr = peer._addr;
    _age = peer._age;
}

string Peer::addr() const {
    return _addr;
}

int Peer::age() const {
    return _age;
}

void Peer::maturate() {
    _age++;
}

bool Peer::operator!=(const Peer &rhs) {
    return _addr != rhs._addr || _age != rhs._age;
}

bool Peer::operator<(const Peer &rhs) {
    return _age < rhs._age;
}

bool operator<(Peer &lhs, Peer &rhs) {
    return lhs._age < rhs._age;
}

bool operator<(const Peer &lhs, Peer &rhs) {
    return lhs._age < rhs._age;
}

ostream &operator<<(ostream &stream, const Peer &rhs) {
    stream << boost::format("<%1%:%2%>") % rhs.addr() % rhs.age();
    return stream;
}
