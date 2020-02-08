#ifndef AF_EMITTER_HPP
#define AF_EMITTER_HPP

#include <definitions.hpp>

namespace af {

    // An emitter can both generate or receive a
    // stream/collection of tasks from an external thread.
    template <typename Tin, typename Tout>
        class af_emitter_t {
            private:
                //friend class af_farm_t;
                friend class af_autonomic_farm_t;

                // Buffer must be thread safe
                std::vector<af::queue_t<Tout*>*>* out_queues;

                std::thread* the_thread;

                size_t num_workers;
                size_t next = 0;
                
                // Thread body
                void main_loop() {
                    std::cout << "Emitter running " << std::endl;
                    bool execute = true;

                    while(execute) {
                        Tout* ret = service(NULL);
                        if(ret == AF_EOS) {
                            this->send_EOS();
                            execute = false;
                        }
                    }
                    return;  
                }

                void send_EOS() {
                    for(int i = 0; i < out_queues->size(); i++)
                        out_queues->at(i)->push((Tout*) AF_EOS);
                }

            protected:
                

            public:
                // Public constructor
                af_emitter_t() {}

                // Sends out a task to the workers
                virtual void send_task(Tout* task) {
                    (out_queues->at(next))->push(task);
                    next += 1;
                    next = next % out_queues->size();
                }

                // The emitter sends tasks to the workers
                // A pure function to ease compile-time optimization
                virtual Tout* service(Tin* task) = 0;

                // PROTECTED
                // This function is called by
                // the farm to start the emitter
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
        };

}

#endif