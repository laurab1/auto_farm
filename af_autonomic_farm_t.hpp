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

                void main_loop() {
                    while(execute) {
                        //check service time
                        auto etime = (this->emitter)->get_emitter_time();
                        auto ctime = (this->collector)->get_collector_time();
                        auto wtime = (this->workers.at(0))->get_worker_time();
                        wtime = wtime/(double) this->num_workers;
                        //NEED TO REWRITE THIS
                        //if(etime >= ctime) {
                        //    if(wtime >= etime) {
                        //        //compare ideal and wtime
                        //    } else {
                        //        //comare ideal and etime
                        //    }
                        //} else {
                        //        //compare ideal and ctime
                        //    }
                        //}
                        ////if service time is too large, resize

                    }
                }

                void add_auto_worker() {
                    af::af_worker_t<Tin, Tout>* w = new af::af_worker_t<Tin, Tout>();
                    w->service = this->workers->at(this->num_workers-1)->service;
                    this->workers->push_back(w);
                    this->num_workers += 1;
                    this->emitter->add_queue(w->get_queue());
                    this->emitter->set_num_workers(this->num_workers);
                    this->collector->set_num_workers(this->num_workers);
                }

                void remove_worker() {
                    this->num_workers -= 1;
                    this->emitter->set_num_workers(this->num_workers);
                    af::af_worker_t<Tin, Tout>* w = this->workers->pop_back();
                    w->kill();                 
                }

            protected:


            public:
                af_autonomic_farm_t(af::af_emitter_t<Tin, Tin>* em,
                                    af::af_collector_t<Tout, Tout>* col,
                                    size_t nw) {
                    this->emitter = em;
                    this->collector = col;
                    this->num_workers = nw;
                    this->emitter->set_num_workers(this->num_workers);
                    this->collector->set_num_workers(this->num_workers);
                    this->w_in_queues = new std::vector<af::queue_t<Tin*>*>();
                    this->w_out_queues = new std::vector<af::queue_t<Tout*>*>();
                    this->workers = new std::vector<af::af_worker_t<Tin, Tout>*>();
                    remaining = new std::vector<Tin*>();
                }
        };
}

#endif