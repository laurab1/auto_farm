#ifndef AF_FARM_HPP
#define AF_FARM_HPP

#include <emitter_t.hpp>
#include <worker_t.hpp>

namespace af {
    template<typename Tein, typename Twin, typename Tcin, typename Tout> 
    class af_farm_t {
        protected:
            af::af_emitter_t<Tein, Twin>* emitter;
            af::af_collector_t<Tein, Twin>* collector;
            std::vector<af::af_worker_t<Twin, Tcin>*>* workers;
            std::vector<af::queue_t<Twin*>*>* w_in_queues;
            std::vector<af::queue_t<Tcin*>*>* w_out_queues;
            size_t num_workers;
            int thread_id = 0;

        public:
            af_farm_t() {}

            af_farm_t(af::af_emitter_t<Tein, Twin>* em,
                      af::af_collector_t<Tein, Twin>* col,
                      size_t nw) {
                emitter = em;
                collector = col;
                num_workers = nw;
                emitter->set_num_workers(num_workers);
                collector->set_num_workers(num_workers);
                w_in_queues = new std::vector<af::queue_t<Twin*>*>();
                w_out_queues = new std::vector<af::queue_t<Tcin*>*>();
                workers = new std::vector<af::af_worker_t<Twin, Tcin>*>();
            }

            void add_worker(af::af_worker_t<Twin, Tcin>* w) {
                workers->push_back(w);
                w->set_id(thread_id++);
                w_in_queues->push_back(w->get_queue(AF_IN_QUEUE));
                w_out_queues->push_back(w->get_queue(AF_OUT_QUEUE));
            }

            void run_farm() {
                emitter->set_queues(w_in_queues);
                collector->set_queues(w_out_queues);

                emitter->run_emitter();
                for(int i = 0; i < workers->size(); i++)
                    (workers->at(i))->run_worker();
                collector->run_collector();
            }

            void stop_farm() {
                collector->stop_collector();
                for(int i = 0; i < workers->size(); i++)
                    (workers->at(i))->stop_worker();
                emitter->stop_emitter();
            }
    };
}

#endif