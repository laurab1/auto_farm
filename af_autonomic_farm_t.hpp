#ifndef AF_AUTONOMICFARM_HPP
#define AF_AUTONOMICFARM_HPP

#include <af_farm_t.hpp>

namespace af {
    template <typename Tin, typename Tout> //the only needed type here is the workers' intype
        class af_autonomic_farm_t : public af::af_farm_t<Tin,Tout> {
            private:
                std::thread* the_thread;

                int id = 0;
                int inc_count = 0; //update at each iteration
                int dec_count = 0; //if they grows, resize

                std::mutex* mutex;
                std::condition_variable* a_condition;
                std::condition_variable* af_condition;
                int64_t i_time;

                void main_loop() {
                    std::cout << "ideal time is " << i_time << std::endl;
                    while(true) {
                        if(this->emitter->check) {
                            std::cout << "AF num_workers " << this->workers->size() << std::endl;
                            this->emitter->execute = false;
                            return;
                        }
                        //check service time
                        int64_t etime = (this->emitter)->get_emitter_time();
                        int64_t ctime = (this->collector)->get_collector_time();
                        int64_t wtime = get_workers_time();
                        wtime = wtime/(int64_t)this->workers->size();
                        
                        int64_t actual_time = 0;
                        if(etime >= ctime) {
                            if(wtime >= etime)
                                actual_time = wtime;
                            else
                                actual_time = etime;
                        } else {
                            if(wtime >= ctime)
                                actual_time = wtime;
                            else
                                actual_time = ctime;
                        }
                        if(abs(i_time - actual_time) < DELTA)
                            continue;
                        //if service time is too large, resize
                        if(i_time < actual_time)
                            inc_count++;
                        else {
                            if(i_time > actual_time)
                                dec_count++;
                        }
                        if(inc_count >= GRAIN && this->w_in_queues->size() < af::MAX_AUTO_WORKER) {
                            this->add_auto_worker();
                            inc_count = 0;
                            dec_count = 0;
                            continue;
                        }
                        if(dec_count >= GRAIN && this->w_in_queues->size() > af::MIN_AUTO_WORKER) {
                            this->remove_worker();
                            dec_count = 0;
                            inc_count = 0;
                            continue;
                        }
                    }
                }

                void add_auto_worker() {
                    std::cout << "AF INC" << std::endl;
                    std::unique_lock<std::mutex> lock(*mutex);
                    this->emitter->freeze = true;
                    while(!this->emitter->check && !this->emitter->freezed) {
                        a_condition->wait(lock);
                    }
                    this->collector->freeze = true;
                    while(!this->collector->freezed) {
                        a_condition->wait(lock);
                    }
                    if(this->emitter->check) {
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
                    this->emitter->freeze = false;
                    this->collector->freeze = false;
                    af_condition->notify_all();
                }

                void remove_worker() {
                    std::cout << "AF DEC" << std::endl;
                    std::unique_lock<std::mutex> lock(*mutex);
                    this->emitter->freeze = true;
                    while(!this->emitter->check && !this->emitter->freezed) {
                        a_condition->wait(lock);
                    }
                    this->collector->freeze = true;
                    while(!this->collector->freezed) {
                        a_condition->wait(lock);
                    }
                    if(this->emitter->check) {
                        this->emitter->freeze = false;
                        this->collector->freeze = false;
                        af_condition->notify_all();
                        return;
                    }
                    af::af_worker_t<Tin, Tout>* w = this->workers->back();
                    this->workers->pop_back();
                    this->w_in_queues->pop_back();
                    w->in_queue->push((Tin*) AF_EOS);
                    this->emitter->freeze = false;
                    this->collector->freeze = false;
                    af_condition->notify_all();            
                }

                int64_t get_workers_time() {
                    int64_t tmp = 0;
                    for(int i=1; i<this->workers->size(); i++) {
                        tmp += this->workers->at(i)->get_worker_time();
                    }
                    return tmp/this->workers->size();
                }


            public:
                af_autonomic_farm_t(af::af_emitter_t<Tin>* em,
                                    af::af_collector_t<Tout>* col, 
                                    size_t nw,
                                    std::chrono::microseconds it) {
                    mutex = new std::mutex();
                    a_condition = new std::condition_variable();
                    af_condition = new std::condition_variable();
                    this->emitter = em;
                    this->collector = col;
                    this->num_workers = nw;
                    this->emitter->set_mutexes(mutex, a_condition, af_condition);
                    this->collector->set_mutexes(mutex, a_condition, af_condition);
                    this->emitter->set_autonomic();
                    this->collector->set_autonomic();
                    this->w_in_queues = new std::vector<af::queue_t<Tin*>*>();
                    this->w_out_queues = new std::vector<af::queue_t<Tout*>*>();
                    this->workers = new std::vector<af::af_worker_t<Tin, Tout>*>();
                    i_time = std::chrono::duration_cast<std::chrono::microseconds>(it).count();
                }

                void stop_autonomic_farm() {
                    this->stop_farm();
                    if(the_thread->joinable())
                        the_thread->join();
                }

                void run_auto_farm() {
                    for(int i=0; i<this->workers->size(); i++) {
                        this->workers->at(i)->set_autonomic();
                        this->workers->at(i)->set_id(id++);
                    }
                    this->run_farm();
                    the_thread = new std::thread(&af::af_autonomic_farm_t<Tin, Tout>::main_loop, this);
                }
        };
}

#endif
