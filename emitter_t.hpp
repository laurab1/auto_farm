#ifndef AF_EMITTER_HPP
#define AF_EMITTER_HPP

#include <definitions.hpp>

namespace af {

    // An emitter can both generate or receive a
    // stream/collection of tasks from an external thread.
    template <typename Tin>
        class af_emitter_t {
            private:
                template<typename A, typename B>
                    friend class af_farm_t;              
                template<typename C, typename D>
                    friend class af_autonomic_farm_t;

                // Buffers must be thread safe
                std::vector<af::queue_t<Tin*>*>* out_queues;

                std::thread* the_thread;

                //size_t num_workers;
                size_t next = -1;

                bool freeze = false;
                bool freezed = false;
                std::mutex* mutex;
                std::condition_variable* a_condition;
                std::condition_variable* af_condition;
                bool execute;
                bool autonomic = false; //by default, the farm is not autonomic

                std::chrono::duration<double> time;
                int64_t e_time;
                int counter = 0;
                
                // Thread body
                void main_loop() {
                    
                    while(execute) {
                        af::utimer tmr("emitter Ts");
                        Tin* ret = service(NULL);
                        time = tmr.get_time();
                        e_time = tmr.count_time(time);
                        if(ret == (Tin*) AF_EOS) {
                            check = 1;
                            this->send_EOS();
                            return;
                        } else
                            this->send_task(ret);
                        
                    }
                    return;
                }

                void send_EOS() {
                    if(autonomic) {
                        std::unique_lock<std::mutex> lock(*mutex);
                        while(freeze) {
                            freezed = true;
                            a_condition->notify_one();
                            af_condition->wait(lock);
                        }
                        freezed = false;
                    }
                    for(int i = 0; i < out_queues->size(); i++)
                        out_queues->at(i)->push((Tin*) AF_EOS);
                }

            protected:
                std::atomic<int> check;
                void run_emitter() {
                    execute = true;
                    the_thread = new std::thread(&af_emitter_t::main_loop, this);
                }

                void stop_emitter() {
                    if(the_thread->joinable())
                        the_thread->join();
                }

                // The emitter gets temporary access to a worker's in_queue
                virtual void set_queues(std::vector<af::queue_t<Tin*>*>* queues) {
                    out_queues = queues;
                }

                int64_t get_emitter_time() {
                    return e_time;
                }

                void set_mutexes(std::mutex* mx,
                                 std::condition_variable* a_cond,
                                 std::condition_variable* af_cond) {
                    mutex = mx;
                    a_condition = a_cond;
                    af_condition = af_cond;
                }

                void set_autonomic() {
                    autonomic = true;
                }

            public:
                // Public constructor
                af_emitter_t() {
                    check = 0;
                }

                // Sends out a task to the workers
                virtual void send_task(Tin* task) {
                    if(autonomic) {
                        std::unique_lock<std::mutex> lock(*mutex);
                        freezed = false;
                        while(freeze) {
                            //std::cout << "emitter waits" << std::endl;
                            freezed = true;
                            a_condition->notify_one();
                            af_condition->wait(lock);
                        }
                        freezed = false;
                    }
                    next += 1;
                    if(next >= out_queues->size())
                        next = 0;
#ifdef TEST
                    counter++;
                    if(counter % 250 == 0)
                        std::cout << "Num workers " << out_queues->size() << std::endl;
#endif
                    (out_queues->at(next))->push(task);
                }

                // The emitter sends tasks to the workers
                // A pure function to ease compile-time optimization
                virtual Tin* service(Tin* task) = 0;

                
        };

}

#endif