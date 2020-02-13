#ifndef AF_TIMER_HPP
#define AF_TIMER_HPP

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>

namespace af {

  class utimer {
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point stop;
    std::string message; 
    using usecs = std::chrono::microseconds;
    using msecs = std::chrono::milliseconds;

  public:

    utimer(const std::string m) : message(m) {
      start = std::chrono::system_clock::now();
    }

    std::chrono::duration<double> get_time() {
      stop = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed = stop - start;
      return elapsed;
    }

    auto count_time(std::chrono::duration<double> elpsd) {
      return std::chrono::duration_cast<std::chrono::microseconds>(elpsd).count();
    }

    ~utimer() {
      stop =
        std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed =
        stop - start;
      auto musec =
        std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

      //std::cout << message << " computed in " << musec << " usec " 
  	      //<< std::endl;

    }
  };

}

#endif