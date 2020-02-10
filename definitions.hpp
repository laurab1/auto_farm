#ifndef AF_DEFS_HPP
#define AF_DEFS_HPP

#include <limits.h>
#include <queue_t.hpp>
#include <vector>
#include <atomic>
#include <utimer.hpp>

namespace af {

    static void* AF_EOS =   (void*)(ULLONG_MAX);
    static const int AF_IN_QUEUE = 1;
    static const int AF_OUT_QUEUE = 2;

}

#endif