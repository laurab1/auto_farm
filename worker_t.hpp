#ifndef AF_WORKER_HPP
#define AF_WORKER_HPP

#include <definitions.hpp>
#include <collector_t.hpp>

namespace af {
    template <typename Tin, typename Tout>
        class af_worker_t {
            private:
                template<typename A, typename B>
                    friend class af_farm_t;
                template<typename C, typename D>
                    friend class af_autonomic_farm_t;

                std::thread* the_thread;
                af::af_collector_t<Tout>* col;
                Tin* next_task;
                int id;
                bool autonomic = false;

                std::chrono::duration<double> time;
                int64_t w_time;

                void main_loop() {

                    while(true) {
                        
                        Tin* task = this->get_next_task();
                        if(task == (Tin*) AF_EOS) {
                            this->send_task((Tout*) AF_EOS);
                            //time = tmr.get_time();
                            //w_time = tmr.count_time(time);
                            return;
                        }
                        af::utimer tmr("worker Ts");
                        Tout* ret = service(task);
                        this->send_task(ret);
                        time = tmr.get_time();
                        w_time = tmr.count_time(time);
                    } 
                    return;
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
                bool execute = true;
                af::queue_t<Tin*>* in_queue;
                af::queue_t<Tout*>* out_queue;

                void run_worker() {
                    the_thread = new std::thread(&af_worker_t::main_loop, this);
                }

                void stop_worker() {
                    if(the_thread->joinable())
                        the_thread->join();
                }

                // Access to the worker's queues
                af::queue_t<Tin*>* get_in_queue() {
                    return (this->in_queue);
                }

                af::queue_t<Tout*>* get_out_queue() {
                    return (this->out_queue);
                }

                void set_autonomic() {
                    autonomic = true;
                }

                int64_t get_worker_time() {
                    return w_time;
                }

                void set_id(int i) {
                    id = i;
                }

            public:
                virtual af_worker_t<Tin, Tout>* clone() const = 0;

                af_worker_t() {
                    in_queue = new af::queue_t<Tin*>();
                    out_queue = new af::queue_t<Tout*>();
                }

                virtual Tout* service(Tin*) const = 0;

                virtual ~af_worker_t() = default;
                
        };
}

#endif