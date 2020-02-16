#ifndef AF_AUTONOMICFARM_HPP
#define AF_AUTONOMICFARM_HPP

#include <af_farm_t.hpp>

namespace af {
    template <typename Tin, typename Tout> //the only needed type here is the workers' intype
        class af_autonomic_farm_t : public af::af_farm_t<Tin,Tin,Tout,Tout> {
            private:
                std::vector<Tin*>* remaining; //a vector of remaining jobs

                bool execute = true;

                std::thread* the_thread;

                std::chrono::duration<double> ideal_time;
                // 1 if we increased size during last check, -1 otherwise
                // Needed to avoid continuous resizing
                int increase = 0;

                void main_loop() {
                    while(execute) {
                        //check service time
                        auto etime = (this->emitter)->get_emitter_time();
                        auto ctime = (this->collector)->get_collector_time();
                        auto wtime = (this->workers->at(0))->get_worker_time();
                        wtime = wtime/(double)this->num_workers;
                        std::chrono::duration<double> actual_time;
                        //NEED TO REWRITE THIS
                        if(etime >= ctime) {
                            if(wtime >= etime)
                                actual_time = wtime;
                            else
                                actual_time = etime;
                        } else
                            actual_time = ctime;
                        auto a_time = std::chrono::duration_cast<std::chrono::microseconds>(actual_time).count();
                        auto i_time = std::chrono::duration_cast<std::chrono::microseconds>(ideal_time).count();
                        //std::cout << a_time << " *** " << i_time << std::endl;
                        //if service time is too large, resize
                        if(ideal_time < actual_time && increase != -1 && this->num_workers < MAX_AUTO_WORKER) {
                            this->add_auto_worker();
                            increase = 1;
                        }
                        else {
                            if(ideal_time > actual_time && increase != 1 && this->num_workers > MIN_AUTO_WORKER) {
                                this->remove_worker();
                                increase = -1;
                            }
                        }
                    }
                    std::cout << this->num_workers << std::endl;
                    return;
                }

                void add_auto_worker() {
                    std::unique_lock<std::mutex> e_lock(this->emitter->ef_mutex);
                    this->emitter->freeze = true;
                    std::cout << this->emitter->freeze << std::endl;
                    while(!this->emitter->freezed) {
                        std::cout << "waiting" << std::endl;
                        this->emitter->ef_condition.wait(e_lock);
                    }
                    //std::unique_lock<std::mutex> c_lock(this->emitter->ef_mutex);
                    //this->collector->freeze = true;
                    //while(!this->collector->freezed) {
                    //    std::cout << "waiting on collector" << std::endl;
                    //    this->collector->cf_condition.wait(c_lock);
                    //}    
                    std::cout << "adding worker" << std::endl;
                    af::af_worker_t<Tin, Tout>* w = this->workers->at(0);
                    //w->service = &(this->workers->at(this->num_workers-1)->service);
                    this->workers->push_back(w);
                    this->num_workers += 1;
                    //this->emitter->add_queue(w->get_queue(AF_IN_QUEUE));
                    this->w_in_queues->push_back(w->get_queue(AF_IN_QUEUE));
                    this->w_out_queues->push_back(w->get_queue(AF_OUT_QUEUE));
                    this->emitter->set_num_workers(this->num_workers);
                    this->collector->set_num_workers(this->num_workers);
                    this->emitter->freeze = false;
                    this->collector->freeze = false;
                    std::cout << "notify" << std::endl;
                    this->emitter->e_condition.notify_one();
                    //this->collector->c_condition.notify_one();
                }

                void remove_worker() {
                    std::cout << "removing worker" << std::endl;
                    std::unique_lock<std::mutex> c_lock(this->collector->cf_mutex);
                    this->collector->freeze = true;
                    while(!this->collector->freezed) {
                        std::cout << "waiting on collector" << std::endl;
                        this->collector->cf_condition.wait(c_lock);
                    } 
                    this->num_workers -= 1;
                    this->emitter->set_num_workers(this->num_workers);
                    af::af_worker_t<Tin, Tout>* w = this->workers->back();
                    this->workers->pop_back();
                    w->in_queue->push((Tout*) AF_EOS);
                    this->collector->c_condition.notify_one();
                    //w->kill();               
                }

            protected:


            public:
                af_autonomic_farm_t(af::af_emitter_t<Tin, Tin>* em,
                                    af::af_collector_t<Tout, Tout>* col,
                                    size_t nw, 
                                    std::chrono::duration<double> it) {
                    this->emitter = em;
                    this->collector = col;
                    this->num_workers = nw;
                    this->emitter->set_num_workers(this->num_workers);
                    this->collector->set_num_workers(this->num_workers);
                    this->w_in_queues = new std::vector<af::queue_t<Tin*>*>();
                    this->w_out_queues = new std::vector<af::queue_t<Tout*>*>();
                    this->workers = new std::vector<af::af_worker_t<Tin, Tout>*>();
                    remaining = new std::vector<Tin*>();
                    ideal_time = it;
                }

                void stop_autonomic_farm() {
                    this->stop_farm();
                    if(the_thread->joinable())
                        the_thread->join();
                }

                void run_auto_farm() {
                    for(int i = 0; i < this->num_workers; i++)
                        this->workers->at(i)->set_autonomic();
                    this->run_farm();
                    the_thread = new std::thread(&af::af_autonomic_farm_t<Tin, Tout>::main_loop, this);
                }
        };
}

#endif