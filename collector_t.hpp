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

                // Thread body
                void main_loop() {
                    std::cout << "Collector running " << std::endl;

                    while(true) {
                        //std::cout << "coll pops" << std::endl;
                        af::utimer tmr("worker Ts");
                        Tin* result = this->get_next_result();
                        Tout* ret;
                        //std::cout << *result << std::endl;
                        if(result == (Tin*) AF_EOS)
                            return; // We are done

                        // What the collector does with the stream
                        // of results is decided when instantiated.
                        ret = service(result);
                        time = tmr.get_time();
                        std::cout << "col time " << ctime << std::endl;
                    }
                }

                // Gets the next task in the queue
                virtual Tin* get_next_result() {
                    for(int i = next; i<num_workers; i++) {
                        Tin* next_result = (in_queues->at(next))->pop();
                        next += 1;
                        //std::cout << "queues size " << in_queues->size() << std::endl;
                        next = next % num_workers;
                        if(next_result != (Tin*) AF_EOS)
                            return next_result;
                    }
                    return (Tin*) AF_EOS;
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