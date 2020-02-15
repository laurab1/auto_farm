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

                size_t next = 0;

                std::chrono::duration<double> time;

                bool autonomic = false; //by default, the farm is not autonomic 

                bool execute = true;

                // Thread body
                void main_loop() {
                    std::cout << "Collector running " << std::endl;

                    while(execute) {
                        af::utimer tmr("collector Ts");
                        Tin* result = this->get_next_result();
                        Tout* ret;
                        if(result == (Tin*) AF_EOS) {
                            return; // We are done
                        } else {
                            if(result == (Tin*) AF_GO_ON)
                                continue;
                        }
                        // What the collector does with the stream
                        // of results is decided when instantiated.
                        ret = service(result);
                        time = tmr.get_time();
                    }
                }

                // Gets the next task in the queue
                Tin* get_next_result() {
                    //Tin* next_result = (in_queues->at(next))->pop();
                    //if(next_result != (Tin*) AF_EOS) {
                    //    next += 1;
                    //    next = next % num_workers;
                    //    return next_result;
                    //} else {
                    //    if(num_workers == 1)
                    //        execute = false;
                    //        std::cout << "in_queue " << in_queues->at(next)->u_is_empty() << std::endl;                            
                    //        return next_result;
                    //    std::swap(in_queues->at(next), in_queues->at(num_workers-1));
                    //    next += 1;
                    //    num_workers -= 1;
                    //    next = next % num_workers;
                    //    return (Tin*) AF_GO_ON;
                    //}
                    for(int i = 0; i < num_workers; i++) {
                        Tin* next_result = (in_queues->at(i))->pop();
                        if(next_result != (Tin*) AF_EOS) {
                            //next += 1;
                            //next = next % num_workers;
                            return next_result;
                        }
                        return (Tin*) AF_EOS;
                    }
                }

            protected:
                std::atomic<size_t> num_workers;

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
                }

                std::chrono::duration<double> get_collector_time() {
                    return time;
                }
                
            public:
                af_collector_t() {}

                virtual Tout* service(Tin*) = 0;

        };
}

#endif