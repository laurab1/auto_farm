#ifndef AF_EMITTER_HPP
#define AF_EMITTER_HPP

#include <definitions.hpp>

namespace af {

    // An emitter can both generate or receive a
    // stream/collection of tasks from an external thread.
    template <typename Tin, typename Tout>
        class af_emitter_t {
            private:
                template<typename A, typename B, typename C, typename D>
                    friend class af_farm_t;              
                template<typename E, typename F>
                    friend class af_autonomic_farm_t;

                // Buffers must be thread safe
                std::vector<af::queue_t<Tout*>*>* out_queues;

                std::thread* the_thread;

                std::atomic<size_t> num_workers;
                size_t next = 0;

                //std::atomic<bool> remaining_jobs;
                bool execute = true;
                bool autonomic = false; //by default, the farm is not autonomic

                std::chrono::duration<double> time;
                
                // Thread body
                void main_loop() {
                    std::cout << "Emitter running " << std::endl;
                    
                    while(execute) {
                        af::utimer tmr("emitter Ts");
                        Tout* ret = service(NULL);
                        time = tmr.get_time();
                        if(ret == AF_EOS) {
                            this->send_EOS();
                            execute = false;
                        }
                    }
                    std::cout << "em returning" << std::endl;
                    return;  
                }

                void send_EOS() {
                    for(int i = 0; i < num_workers; i++)
                        out_queues->at(i)->push((Tout*) AF_EOS);
                }

            protected:
                void run_emitter() {
                    the_thread = new std::thread(&af_emitter_t::main_loop, this);
                }

                void stop_emitter() {
                    if(the_thread->joinable())
                        the_thread->join();
                }

                // The emitter gets temporary access to a worker's in_queue
                virtual void set_queues(std::vector<af::queue_t<Tout*>*>* queues) {
                    out_queues = queues;
                }

                void set_autonomic(bool at) {
                    autonomic = at;
                }

                void set_num_workers(size_t nw) {
                    num_workers = nw;
                }

                void add_queue(af::queue_t<Tout*>* queue) {
                    out_queues->push_back(queue);
                }

                std::chrono::duration<double> get_emitter_time() {
                    return time;
                }

            public:
                // Public constructor
                af_emitter_t() {
                    //this->set_remaining_jobs(false);
                }

                // Sends out a task to the workers
                virtual void send_task(Tout* task) {
                    //std::cout << "push job e " << num_workers << " " << next << std::endl;
                    (out_queues->at(next))->push(task);
                    next += 1;
                    next = next % num_workers;
                }

                // The emitter sends tasks to the workers
                // A pure function to ease compile-time optimization
                virtual Tout* service(Tin* task) = 0;

                
        };

}

#endif