#ifndef AF_WORKER_HPP
#define AF_WORKER_HPP

#include <definitions.hpp>

// Pretty similar to emitter_t. Too much similar to emitter_t.
// Should be refactored in some way.
namespace af {
    template <typename Tin, typename Tout>
        class af_worker_t {
            private:
                template<typename A, typename B, typename C, typename D>
                    friend class af_farm_t;
                friend class af_autonomic_farm_t;
                template<typename E>
                    friend class af_controller_t;

                std::thread* the_thread;
                af::queue_t<Tin*>* in_queue;
                af::queue_t<Tout*>* out_queue;
                Tin* next_task;

                void main_loop() {
                    std::cout << "Worker running " << std::endl;
                    bool execute = true;

                    while(execute) {
                        //std::cout << "pop" << std::endl;
                        Tin* task = this->get_next_task();
                        //std::cout << "got" << *task << std::endl;
                        if(task == (Tin*) AF_EOS) {
                            this->send_task((Tout*) AF_EOS);
                            //std::cout << "returning" << std::endl;
                            return;
                        }
                        Tout* ret = service(task);
                        this->send_task(ret);
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

            public:
                af_worker_t() {
                    in_queue = new af::queue_t<Tin*>();
                    out_queue = new af::queue_t<Tout*>();
                }

                virtual Tout* service(Tin*) = 0;
                
        };
}

#endif