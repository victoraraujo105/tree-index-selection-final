#ifndef Stats_HPP
#define Stats_HPP

#include <cstdio>

struct Stats {
    size_t pags, ios, tuples;
    Stats(size_t pags, size_t ios, size_t tuples): pags(pags), ios(ios), tuples(tuples) {}
};

#endif