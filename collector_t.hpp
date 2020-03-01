#ifndef AF_COLLECTOR_HPP
#define AF_COLLECTOR_HPP

#include <definitions.hpp>

namespace af {
    template <typename Tout>
        class af_collector_t {
            private:
                template<typename A, typename B>
                    friend class af_farm_t;
                template<typename C, typename D>
                    friend class af_autonomic_farm_t;
                template<typename E, typename F>
                    friend class af_worker_t;

                std::thread* the_thread;

                std::vector<af::queue_t<Tout*>*>* in_queues;

                size_t next = -1;
                size_t num_workers;

                bool freeze = false;
                bool freezed = false;
                std::mutex* mutex;
                std::condition_variable* a_condition;
                std::condition_variable* af_condition;

                std::chrono::duration<double> time;

                bool autonomic = false; //by default, the farm is not autonomic 
                size_t left;

                bool execute = true;
                

                // Thread body
                void main_loop() {

                    while(execute) {
                        af::utimer tmr("collector Ts"); 
                        Tout* result = this->get_next_result();
                        Tout* ret;
                        if(result == (Tout*) AF_EOS) {
                            check = 1;
                            if(autonomic)
                                a_condition->notify_all();
                            std::cout << num_workers << std::endl;
                            return;
                        }
                        if(result == (Tout*) AF_GO_ON)
                            continue;
                        if(result != (Tout*) AF_EOS)
                            ret = service(result);
                        time = tmr.get_time();
                    }
                }

                // Gets the next task in the queue
                Tout* get_next_result() {
                    if(autonomic) {
                        std::unique_lock<std::mutex> lock(*mutex);
                        freezed = false;
                        while(freeze) {
                            freezed = true;
                            a_condition->notify_one();
                            af_condition->wait(lock);
                        }
                        freezed = false;
                        next += 1;
                        if(next == left)
                            next = 0;
                        Tout* next_result = (in_queues->at(next))->timed_pop();
                        if(next_result == (Tout*) AF_EOS) {
                            if(left == 1) {
                                return next_result;
                            }
                            std::swap(in_queues->at(next), in_queues->at(left-1));
                            left -= 1;
                            next = 0;
                            return (Tout*) AF_GO_ON;
                        }
                        if(next_result == NULL) {
                            return (Tout*) AF_GO_ON;
                        }
                        return next_result;
                    }
                    next += 1;
                    if(next == num_workers)
                        next = 0;
                    return in_queues->at(next)->pop();              
                }

            protected:
                std::atomic<int> check;

                void set_mutexes(std::mutex* mx,
                                 std::condition_variable* a_cond,
                                 std::condition_variable* af_cond) {
                    mutex = mx;
                    a_condition = a_cond;
                    af_condition = af_cond;
                }

                void run_collector() {
                    the_thread = new std::thread(&af_collector_t::main_loop, this);
                }

                void stop_collector() {
                    if(the_thread->joinable())
                        the_thread->join();
                }

                // The collector gets temporary access to the workers' queues
                virtual void set_queues(std::vector<af::queue_t<Tout*>*>* queues) {
                    in_queues = queues;
                }

                void set_num_workers(size_t nw) {
                    num_workers = nw;
                    left = nw;
                }

                std::chrono::duration<double> get_collector_time() {
                    return time;
                }

                void set_autonomic() {
                    autonomic = true;
                }
                
            public:
                af_collector_t() {
                    check = 0;
                }

                virtual Tout* service(Tout*) = 0;

        };
}

#endif