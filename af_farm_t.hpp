#ifndef AF_FARM_HPP
#define AF_FARM_HPP

#include <emitter_t.hpp>
#include <worker_t.hpp>
#include <collector_t.hpp>

namespace af {
    template<typename Tein, typename Twin, typename Tcin, typename Tout> 
    class af_farm_t {
        private:
            af::af_emitter_t<Tein, Twin>* emitter;
            af::af_collector_t<Tein, Twin>* collector;
            std::vector<af::af_worker_t<Twin, Tcin>*>* workers;
            std::vector<af::queue_t<Twin*>*>* w_in_queues;
            std::vector<af::queue_t<Tcin*>*>* w_out_queues;
            size_t num_workers;

        protected:
            

        public:
            af_farm_t(af::af_emitter_t<Tein, Twin>* em,
                      af::af_collector_t<Tein, Twin>* col,
                      size_t nw) {
                emitter = em;
                collector = col;
                w_in_queues = new std::vector<af::queue_t<Twin*>*>();
                w_out_queues = new std::vector<af::queue_t<Tcin*>*>();
                workers = new std::vector<af::af_worker_t<Twin, Tcin>*>();
            }

            void add_worker(af::af_worker_t<Twin, Tcin>* w) {
                workers->push_back(w);
                num_workers += 1;
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