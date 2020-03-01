#ifndef AF_AUTONOMICFARM_HPP
#define AF_AUTONOMICFARM_HPP

#include <af_farm_t.hpp>

namespace af {
    template <typename Tin, typename Tout> //the only needed type here is the workers' intype
        class af_autonomic_farm_t : public af::af_farm_t<Tin,Tout> {
            private:
                bool execute = true;

                std::thread* the_thread;

                std::chrono::duration<double> ideal_time;
                // 1 if we increased size during last check, -1 otherwise
                // Needed to avoid continuous resizing
                int increase = 0;
                int id = 0;

                std::mutex* mutex;
                std::condition_variable* a_condition;
                std::condition_variable* af_condition;
                size_t em_num_workers;
                bool em_check = false;
                std::chrono::duration<double> old_time;

                void main_loop() {
                    while(true) {
                        if(this->emitter->check) {
                            std::cout << "num_workers " << this->workers->size() << std::endl;
                            em_check = true;
                            this->emitter->execute = false;
                            this->workers->at(0)->execute = false;
                            return;
                        }
                        //check service time
                        auto etime = (this->emitter)->get_emitter_time();
                        auto ctime = (this->collector)->get_collector_time();
                        auto wtime = get_workers_time();
                        wtime = wtime/(double)this->num_workers;
                        std::chrono::duration<double> actual_time;
                        if(etime >= ctime) {
                            if(wtime >= etime)
                                actual_time = wtime;
                            else
                                actual_time = etime;
                        } else
                            actual_time = ctime;
                        auto a_time = std::chrono::duration_cast<std::chrono::microseconds>(actual_time).count();
                        auto i_time = std::chrono::duration_cast<std::chrono::microseconds>(ideal_time).count();
                        //if service time is too large, resize
                        if(!em_check && i_time < a_time && increase != -1 && this->num_workers < MAX_AUTO_WORKER) {
                            this->add_auto_worker();
                            increase = 1;
                        }
                        else {
                            if(!em_check && i_time > a_time && increase != 1 && this->num_workers > MIN_AUTO_WORKER) {
                                this->remove_worker();
                                increase = -1;
                            }
                        }
                    }
                }

                void add_auto_worker() {  
                    std::unique_lock<std::mutex> lock(*mutex);
                    this->emitter->freeze = true;
                    while(!this->emitter->freezed) {
                        a_condition->wait(lock);
                    }
                    this->collector->freeze = true;
                    while(!this->collector->freezed) {
                        a_condition->wait(lock);
                    }   
                    if(em_check) {
                        this->emitter->freeze = false;
                        this->collector->freeze = false;
                        af_condition->notify_all();
                        return;
                    }
                    af::af_worker_t<Tin, Tout>& w = *this->workers->at(0);
                    af_worker_t<Tin,Tout>* w_new = w.clone();
                    this->add_worker(w_new);
                    w_new->set_id(id++);
                    w_new->set_autonomic();
                    w_new->run_worker();
                    this->num_workers += 1;
                    this->em_num_workers += 1;
                    this->emitter->set_num_workers(this->em_num_workers);
                    this->collector->set_num_workers(this->num_workers);
                    this->emitter->freeze = false;
                    this->collector->freeze = false;
                    af_condition->notify_all();
                }

                void remove_worker() {
                    std::unique_lock<std::mutex> lock(*mutex);
                    this->emitter->freeze = true;
                    while(!this->emitter->freezed) {
                        a_condition->wait(lock);
                    }
                    this->collector->freeze = true;
                    while(!this->collector->freezed) {
                        a_condition->wait(lock);
                    }
                    if(em_check) {
                        this->emitter->freeze = false;
                        this->collector->freeze = false;
                        af_condition->notify_all();
                        return;
                    }
                    this->em_num_workers -= 1;
                    this->num_workers -= 1;
                    this->emitter->set_num_workers(this->em_num_workers);
                    af::af_worker_t<Tin, Tout>* w = this->workers->back();
                    w->in_queue->push((Tin*) AF_EOS);
                    this->emitter->freeze = false;
                    this->collector->freeze = false;
                    af_condition->notify_all();            
                }

                std::chrono::duration<double> get_workers_time() {
                    std::chrono::duration<double> max = std::chrono::duration<double>(0.0);
                    std::chrono::duration<double> tmp = std::chrono::duration<double>(0.0);
                    for(int i=0; i<this->num_workers; i++) {
                        tmp = this->workers->at(i)->get_worker_time();
                        if(tmp > max)
                            max = tmp;
                    }
                    return max;
                }


            public:
                af_autonomic_farm_t(af::af_emitter_t<Tin>* em,
                                    af::af_collector_t<Tout>* col, 
                                    size_t nw,
                                    std::chrono::nanoseconds it) {
                    mutex = new std::mutex();
                    a_condition = new std::condition_variable();
                    af_condition = new std::condition_variable();
                    this->emitter = em;
                    this->collector = col;
                    this->num_workers = nw;
                    this->em_num_workers = nw;
                    this->emitter->set_num_workers(this->num_workers);
                    this->collector->set_num_workers(this->num_workers);
                    this->emitter->set_mutexes(mutex, a_condition, af_condition);
                    this->collector->set_mutexes(mutex, a_condition, af_condition);
                    this->emitter->set_autonomic();
                    this->collector->set_autonomic();
                    this->w_in_queues = new std::vector<af::queue_t<Tin*>*>();
                    this->w_out_queues = new std::vector<af::queue_t<Tout*>*>();
                    this->workers = new std::vector<af::af_worker_t<Tin, Tout>*>();
                    ideal_time = it;
                }

                void stop_autonomic_farm() {
                    this->stop_farm();
                    if(the_thread->joinable())
                        the_thread->join();
                }

                void run_auto_farm() {
                    for(int i=0; i<this->num_workers; i++) {
                        this->workers->at(i)->set_autonomic();
                        this->workers->at(i)->set_id(id++);
                    }
                    this->run_farm();
                    the_thread = new std::thread(&af::af_autonomic_farm_t<Tin, Tout>::main_loop, this);
                }
        };
}

#endif