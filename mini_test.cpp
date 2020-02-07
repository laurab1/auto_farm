#include <af_farm_t.hpp>
#include <unistd.h>

class emitter: public af::af_emitter_t<int, int> {
    public:
        int* service(int*) {
            for(int i=0; i<10; i++) {
                std::cout << "ciao" << std::endl;
                this->send_task(new int(0));
            }
            return (int*) af::AF_EOS;
        }

        void run() {
            this->run_emitter();
        }

        void queue(af::queue_t<int*>* q) {
            std::vector<af::queue_t<int*>*>* qu = new std::vector<af::queue_t<int*>*>();
            qu->push_back(q);
            this->set_queues(qu);
        }

        void stop() {
            this->stop_emitter();
        }
};

class worker: public af::af_worker_t<int, int> {
    public:
        int* service(int* task) {
            int* res = new int((*task)+1);
            return res;
        }

        void run() {
            this->run_worker();
        }

        af::queue_t<int*>* in_queue() {
            this->get_queue(1);
        }

        af::queue_t<int*>* out_queue() {
            this->get_queue(2);
        }

                void stop() {
            this->stop_worker();
        }
};

class collector: public af::af_collector_t<int, int> {
    public:
        int* service(int* task) {
            std::cout << "Result " << (*task) << std::endl;
        }

        void run() {
            this->run_collector();
        }

        void queue(af::queue_t<int*>* q) {
            std::vector<af::queue_t<int*>*>* qu = new std::vector<af::queue_t<int*>*>();
            qu->push_back(q);
            this->set_queues(qu);
        }

                void stop() {
            this->stop_collector();
        }
};

int main() {
    af::af_emitter_t<int, int>* emtr = new emitter();
    af::af_collector_t<int, int>* clctr = new collector();
    af::af_farm_t<int, int, int, int>* farm = new af::af_farm_t<int, int, int, int>(emtr, clctr, (size_t) 2);
    for(int i = 0; i < 2; i++)
        farm->add_worker(new worker());
    farm->run_farm();
    farm->stop_farm();
    return 0;
}