#ifndef AF_COLLECTOR_HPP
#define AF_COLLECTOR_HPP

#include <definitions.hpp>

namespace af {
    template <typename Tin, typename Tout>
        class af_collector_t {
            private:
                template<typename A, typename B, typename C, typename D>
                    friend class af_farm_t;
                template<typename E, typename F>
                    friend class af_autonomic_farm_t;
                template<typename G, typename H>
                    friend class af_worker_t;

                std::thread* the_thread;

                std::vector<af::queue_t<Tin*>*>* in_queues;

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
                    //std::cout << "Collector running " << std::endl;

                    while(execute) {
                        af::utimer tmr("collector Ts"); 
                        Tin* result = this->get_next_result();
                        Tout* ret;
                        if(result == (Tin*) AF_EOS) {
                            std::cout << "received eos" << std::endl;
                            check = 1;
                            a_condition->notify_all();
                            return;
                        }
                        if(result == (Tin*) AF_GO_ON)
                            continue;
                        if(result != (Tin*) AF_EOS)
                            ret = service(result);
                        time = tmr.get_time();
                    }
                }

                // Gets the next task in the queue
                Tin* get_next_result() {
                    if(autonomic) {
                        std::unique_lock<std::mutex> lock(*mutex);
                        freezed = false;
                        while(freeze) {
                            //std::cout << "collector waits " << std::endl;
                            freezed = true;
                            a_condition->notify_one();
                            af_condition->wait(lock);
                        }
                        freezed = false;
                    }
                    next += 1;
                    if(next == left)
                        next = 0;
                    Tin* next_result = (in_queues->at(next))->pop();
                    if(next_result == (Tin*) AF_EOS) {
                        if(left == 1) {
                            return next_result;
                        }
                        std::swap(in_queues->at(next), in_queues->at(left-1));
                        left -= 1;
                        next = 0;
                        return (Tin*) AF_GO_ON;
                    }
                    return next_result;
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

                // The collector gets temporary access to a worker's out_queue
                virtual void set_queues(std::vector<af::queue_t<Tin*>*>* queues) {
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

                virtual Tout* service(Tin*) = 0;

        };
}

#endif