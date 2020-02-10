#include <af_autonomic_farm_t.hpp>
#include <unistd.h>
#include <utimer.hpp>

int i = 0;

class emitter: public af::af_emitter_t<int, int> {
    public:
        int* service(int*) {
            for(int i=0; i<100; i++) {
                //std::cout << "ciao" << std::endl;
                this->send_task(new int(i*10));
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
            usleep(50000);
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
            std::cout << "Result " << i++ << " " << (*task) << std::endl;
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

int main(int argc, char* argv[]) {
    size_t nw = atoi(argv[1]);

    af::af_emitter_t<int, int>* emtr = new emitter();
    af::af_collector_t<int, int>* clctr = new collector();
    //af::af_farm_t<int, int, int, int>* farm = new af::af_farm_t<int, int, int, int>(emtr, clctr, nw);

    //need to understand if this is the right way to set farm's time...
    std::chrono::duration<double> time = std::chrono::duration<double>(2.0);
    af::af_autonomic_farm_t<int, int>* farm = new af::af_autonomic_farm_t<int, int>(emtr, clctr, nw, time);
    for(int i = 0; i < nw; i++)
        farm->add_worker(new worker());

    af::utimer tmr("Completion time");
    farm->run_farm();
    farm->stop_farm();
    return 0;
}