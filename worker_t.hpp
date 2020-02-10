#ifndef AF_WORKER_HPP
#define AF_WORKER_HPP

#include <definitions.hpp>
#include <collector_t.hpp>

// Pretty similar to emitter_t. Too much similar to emitter_t.
// Should be refactored in some way.
namespace af {
    template <typename Tin, typename Tout>
        class af_worker_t {
            private:
                template<typename A, typename B, typename C, typename D>
                    friend class af_farm_t;
                template<typename E, typename F>
                    friend class af_autonomic_farm_t;

                std::thread* the_thread;
                af::queue_t<Tin*>* in_queue;
                af::queue_t<Tout*>* out_queue;
                af::af_collector_t<Tout, Tout>* col;
                Tin* next_task;

                bool autonomic = false;
                bool cancelled = false;

                std::chrono::duration<double> time;

                void main_loop() {
                    std::cout << "Worker running " << std::endl;
                    bool execute = true;

                    while(execute) {
                        //std::cout << "pop" << std::endl;
                        af::utimer tmr("worker Ts");
                        Tin* task = this->get_next_task();
                        //std::cout << "got" << *task << std::endl;
                        if(task == (Tin*) AF_EOS) {
                            this->send_task((Tout*) AF_EOS);
                            if(autonomic && cancelled)
                                col->num_workers--;
                            //std::cout << "returning" << std::endl;
                            return;
                        }
                        Tout* ret = service(task);
                        this->send_task(ret);
                        time = tmr.get_time();
                        auto ctime = tmr.count_time(time);
                    } 
                }

                virtual void send_task(Tout* task) {
                    out_queue->push(task);
                }

                // Gets the next task in the queue
                virtual Tin* get_next_task() {
                    next_task = in_queue->pop();
                    return next_task;
                }

            protected:
                // PROTECTED
                void run_worker() {
                    the_thread = new std::thread(&af_worker_t::main_loop, this);
                }

                void stop_worker() {
                    if(the_thread->joinable())
                        the_thread->join();
                }

                // Access to the worker's queues
                af::queue_t<Tout*>* get_queue(int in_out) {
                    switch(in_out)
                    {
                    case AF_IN_QUEUE:
                        return (this->in_queue);
                    case AF_OUT_QUEUE:
                        return (this->out_queue);
                    default:
                        std::cout << "Invalid use of get_queue" << std::endl;
                        return NULL;   
                    }
                }

                void kill() {
                    in_queue->push((Tin*) AF_EOS);
                    cancelled = true;
                }

                std::chrono::duration<double> get_worker_time() {
                    return time;
                }

            public:
                af_worker_t() {
                    in_queue = new af::queue_t<Tin*>();
                    out_queue = new af::queue_t<Tout*>();
                }

                virtual Tout* service(Tin*) = 0;
                
        };
}

#endif