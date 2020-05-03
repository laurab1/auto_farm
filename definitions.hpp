#ifndef AF_DEFS_HPP
#define AF_DEFS_HPP

#include <limits.h>
#include <queue_t.hpp>
#include <vector>
#include <atomic>
#include <utimer.hpp>

namespace af {

    static void* AF_EOS =   (void*)(ULLONG_MAX);
    static void* AF_GO_ON = (void*)(ULLONG_MAX-1);
    static const int AF_IN_QUEUE = 1;
    static const int AF_OUT_QUEUE = 2;
    static const int64_t DELTA = 500;
    static const int GRAIN = 25;
    static const unsigned int MIN_AUTO_WORKER = 1;
}

#endif