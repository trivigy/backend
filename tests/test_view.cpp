#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define BOOST_TEST_MODULE test_view

#include <boost/test/unit_test.hpp>

#include "peer.h"
#include "view.h"

class F : public View {
};

BOOST_FIXTURE_TEST_CASE(remove_duplicates, F) { // NOLINT
    deque<Peer> buffer{
        Peer(string("a"), 0),
        Peer(string("a"), 3),
        Peer(string("b"), 1),
        Peer(string("c"), 5),
        Peer(string("d"), 6),
        Peer(string("a"), 1),
        Peer(string("d"), 3),
        Peer(string("f"), 7),
        Peer(string("g"), 8),
        Peer(string("h"), 9)
    };
    append(buffer);
    remove_duplicates();
    deque<Peer> snap = snapshot();

    deque<Peer> expected{
        Peer(string("a"), 0),
        Peer(string("b"), 1),
        Peer(string("c"), 5),
        Peer(string("d"), 3),
        Peer(string("f"), 7),
        Peer(string("g"), 8),
        Peer(string("h"), 9)
    };

    BOOST_CHECK_EQUAL_COLLECTIONS(
        snap.begin(), snap.end(),
        expected.begin(), expected.end()
    );

}

BOOST_FIXTURE_TEST_CASE(remove_old, F) { // NOLINT
    deque<Peer> buffer{
        Peer(string("a"), 0),
        Peer(string("a"), 3),
        Peer(string("b"), 1),
        Peer(string("c"), 6),
        Peer(string("d"), 6),
        Peer(string("a"), 1),
        Peer(string("d"), 3),
        Peer(string("f"), 7),
        Peer(string("g"), 4),
        Peer(string("h"), 9)
    };
    append(buffer);
    remove_old(3);
    deque<Peer> snap = snapshot();

    deque<Peer> expected{
        Peer(string("a"), 0),
        Peer(string("a"), 3),
        Peer(string("b"), 1),
        Peer(string("d"), 6),
        Peer(string("a"), 1),
        Peer(string("d"), 3),
        Peer(string("g"), 4)
    };

    BOOST_CHECK_EQUAL_COLLECTIONS(
        snap.begin(), snap.end(),
        expected.begin(), expected.end()
    );
}

BOOST_FIXTURE_TEST_CASE(remove_head, F) { // NOLINT
    deque<Peer> buffer{
        Peer(string("a"), 0),
        Peer(string("a"), 3),
        Peer(string("b"), 1),
        Peer(string("c"), 6),
        Peer(string("d"), 6),
        Peer(string("a"), 1),
        Peer(string("d"), 3),
        Peer(string("f"), 7),
        Peer(string("g"), 4),
        Peer(string("h"), 9)
    };
    append(buffer);
    remove_head(5);
    deque<Peer> snap = snapshot();

    deque<Peer> expected{
        Peer(string("a"), 1),
        Peer(string("d"), 3),
        Peer(string("f"), 7),
        Peer(string("g"), 4),
        Peer(string("h"), 9)
    };

    BOOST_CHECK_EQUAL_COLLECTIONS(
        snap.begin(), snap.end(),
        expected.begin(), expected.end()
    );
}

BOOST_FIXTURE_TEST_CASE(remove_random, F) { // NOLINT
    deque<Peer> buffer{
        Peer(string("a"), 0),
        Peer(string("a"), 3),
        Peer(string("b"), 1),
        Peer(string("c"), 6),
        Peer(string("d"), 6),
        Peer(string("a"), 1),
        Peer(string("d"), 3),
        Peer(string("f"), 7),
        Peer(string("g"), 4),
        Peer(string("h"), 9)
    };
    append(buffer);
    remove_random(5);
    deque<Peer> snap = snapshot();

    BOOST_CHECK_EQUAL(snap.size(), size());
}

BOOST_FIXTURE_TEST_CASE(remove_update, F) { // NOLINT
    deque<Peer> buffer{
        Peer(string("a"), 0),
        Peer(string("a"), 3),
        Peer(string("b"), 1),
        Peer(string("c"), 5),
        Peer(string("d"), 6),
        Peer(string("a"), 1),
        Peer(string("d"), 3),
        Peer(string("f"), 7),
        Peer(string("g"), 8),
        Peer(string("h"), 9)
    };
    append(buffer);
    update(30, 15, 0, buffer);
    deque<Peer> snap = snapshot();

    deque<Peer> expected{
        Peer(string("a"), 1),
        Peer(string("b"), 2),
        Peer(string("c"), 6),
        Peer(string("d"), 4),
        Peer(string("f"), 8),
        Peer(string("g"), 9),
        Peer(string("h"), 10)
    };

    BOOST_CHECK_EQUAL_COLLECTIONS(
        snap.begin(), snap.end(),
        expected.begin(), expected.end()
    );

}

#pragma clang diagnostic pop