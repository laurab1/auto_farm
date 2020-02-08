#ifndef AF_DEFS_HPP
#define AF_DEFS_HPP

#include <limits.h>
#include <queue_t.hpp>
#include <vector>

namespace af {

    static void* AF_EOS =   (void*)(ULLONG_MAX);
    static void* AF_GO_ON = (void*)(ULLONG_MAX-1);
    static const int AF_IN_QUEUE = 1;
    static const int AF_OUT_QUEUE = 2;

}

#endif