#ifndef AF_FARM_HPP
#define AF_FARM_HPP

#include <emitter_t.hpp>
#include <worker_t.hpp>

namespace af {
    template<typename Tin, typename Tout> 
    class af_farm_t {
        protected:
            af::af_emitter_t<Tin>* emitter;
            af::af_collector_t<Tout>* collector;
            std::vector<af::af_worker_t<Tin, Tout>*>* workers;
            std::vector<af::queue_t<Tin*>*>* w_in_queues;
            std::vector<af::queue_t<Tout*>*>* w_out_queues;
            size_t num_workers;
            int thread_id = 0;

        public:
            af_farm_t() {}

            af_farm_t(af::af_emitter_t<Tin>* em,
                      af::af_collector_t<Tout>* col,
                      size_t nw) {
                emitter = em;
                collector = col;
                num_workers = nw;
                emitter->set_num_workers(num_workers);
                collector->set_num_workers(num_workers);
                w_in_queues = new std::vector<af::queue_t<Tin*>*>();
                w_out_queues = new std::vector<af::queue_t<Tout*>*>();
                workers = new std::vector<af::af_worker_t<Tin, Tout>*>();
            }

            void add_worker(af::af_worker_t<Tin, Tout>* w) {
                workers->push_back(w);
                w->set_id(thread_id++);
                w_in_queues->push_back(w->get_in_queue());
                w_out_queues->push_back(w->get_out_queue());
            }

            void run_farm() {
                emitter->set_queues(w_in_queues);
                collector->set_queues(w_out_queues);

                emitter->run_emitter();
                for(int i = 0; i < num_workers; i++)
                    (workers->at(i))->run_worker();
                collector->run_collector();
            }

            void stop_farm() {
                emitter->stop_emitter();
                for(int i = 0; i < num_workers; i++)
                    (workers->at(i))->stop_worker();
                collector->stop_collector();
            }
    };
}

#endif