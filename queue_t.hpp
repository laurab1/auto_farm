#ifndef AF_QUEUE_HPP
#define AF_QUEUE_HPP

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <chrono>
#include <cstddef>
#include <math.h>
#include <string>
#include <thread>
#define WAIT_TIME 50

/*
 * The sample queue seen during the course.
 */

namespace af {

  template <typename T> 
    class queue_t {
      private:
        template <typename A, typename B>
          friend class af_worker_t;
        std::mutex              d_mutex;
        std::condition_variable d_condition;
        std::deque<T>           d_queue;

      public:

        bool u_is_empty() {
          return d_queue.empty();
        }
        
        queue_t(std::string s) { std::cout << "Created " << s << " queue " << std::endl;  }
        queue_t() {}

        void push(T const& value) {
          {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            d_queue.push_front(value);
          }
          this->d_condition.notify_one();
        }

        T pop() {
          std::unique_lock<std::mutex> lock(this->d_mutex);
          //std::cout << "blocking here" << std::endl;
          this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
          T rc(std::move(this->d_queue.back()));
          //std::cout << rc << std::endl;
          this->d_queue.pop_back();
          return rc;
        }

        T timed_pop() {
          std::unique_lock<std::mutex> lock(this->d_mutex);
          if(this->d_condition.wait_for(lock,
                                    std::chrono::microseconds(WAIT_TIME),
                                    [=]{ return !this->d_queue.empty(); })) {
            T rc(std::move(this->d_queue.back()));
            this->d_queue.pop_back();
            return rc;
          } else {
            return NULL;
          }
        }

        bool is_empty() {
          std::unique_lock<std::mutex> lock(this->d_mutex);
          return(d_queue.empty());
        }
    };

}

#endif