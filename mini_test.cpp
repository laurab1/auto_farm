#include <af_autonomic_farm_t.hpp>
#include <unistd.h>
#include <utimer.hpp>

int i = 0;

class emitter: public af::af_emitter_t<int> {
    public:
        int* service(int*) {
            //usleep(1000);
            if(i>=1000)
                return (int*) af::AF_EOS;
            return (new int(i++));
        }

};

class worker: public af::af_worker_t<int, int> {
    public:
        int* service(int* task) const override {
            int* res = new int((*task)+1);
            usleep(10000);
            return res;
        }

        virtual worker* clone() const override {
            return new worker(*this);
        }

};

class collector: public af::af_collector_t<int> {
    public:
        int* service(int* task) {
            std::cout << "Result " << " " << (*task) << std::endl;
        }

};

int main(int argc, char* argv[]) {
    size_t nw = atoi(argv[1]);

    af::af_emitter_t<int>* emtr = new emitter();
    af::af_collector_t<int>* clctr = new collector();
    af::af_emitter_t<int>* emtr2 = new emitter();
    af::af_collector_t<int>* clctr2 = new collector();

    //need to understand if this is the right way to set farm's time...
    std::chrono::nanoseconds time = std::chrono::nanoseconds(100);
    af::af_farm_t<int, int>* static_farm = new af::af_farm_t<int,int>(emtr2, clctr2, nw);
    for(int j = 0; j < nw; j++)
        static_farm->add_worker(new worker());
    af::af_autonomic_farm_t<int, int>* farm = new af::af_autonomic_farm_t<int, int>(emtr, clctr, nw, time);
    for(int j = 0; j < nw; j++)
        farm->add_worker(new worker());
    af::utimer tmr("Farm time");
    static_farm->run_farm();
    static_farm->stop_farm();
    auto ttime = tmr.get_time();
    auto ctime = std::chrono::duration_cast<std::chrono::microseconds>(ttime).count();
    std::cout << ctime << std::endl;
    i = 0;
    af::utimer tmr2("Autonomic farm time");
    farm->run_auto_farm();
    farm->stop_autonomic_farm();
    auto ttime2 = tmr2.get_time();
    auto ctime2 = std::chrono::duration_cast<std::chrono::microseconds>(ttime2).count();
    std::cout << ctime2 << std::endl;

    return 0;
}