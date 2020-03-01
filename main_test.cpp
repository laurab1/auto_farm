#include <af_autonomic_farm_t.hpp>
#include <unistd.h>
#include <utimer.hpp>
#define SMALL 9000
#define MEDIUM 45000
#define LARGE 120000

int i = 0;
int num_tasks = SMALL;

class emitter: public af::af_emitter_t<std::vector<std::string>> {
    public:
        std::vector<std::string>* service(std::vector<std::string>*) {
            if(i >= num_tasks)
                return (std::vector<std::string>*) af::AF_EOS;
            i++;
            std::vector<std::string>* v = new std::vector<std::string>();
            std::string st1 = "ciao";
            if(i < num_tasks/3) {
                for(int j=0; j<16; j++)
                    v->push_back(st1);
                return v;
            }
            if(i >= num_tasks/3 && i < (num_tasks/3)*2) {
                for(int j=0; j<2; j++)
                    v->push_back(st1);
                return v;
            }
            for(int j=0; j<8; j++)
                v->push_back(st1);
            return v;
        }

};

class worker: public af::af_worker_t<std::vector<std::string>, std::string> {
    public:
        std::string* service(std::vector<std::string>* task) const override {
            std::string* res = new std::string();
            for(int i=0; i<task->size(); i++)
                *res = *res + task->at(i);
            return res;
        }

        virtual worker* clone() const override {
            return new worker(*this);
        }

};

class collector: public af::af_collector_t<std::string> {
    public:
        std::string* service(std::string* task) {
            std::cout << "Result " << " " << (*task) << std::endl;
        }

};

std::string usage() {
    return "Use ./main_test [nw] [set_tasks (1 for 9000, 2 for 45000, 3 for 120000)]";
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cout << usage() << std::endl;
    }
    size_t nw = atoi(argv[1]);
    int choice = atoi(argv[2]);

    switch(choice) {
        case 1: break;
        case 2: {
            num_tasks = MEDIUM;
            break;
        }
        case 3: {
            num_tasks = LARGE;
        }
        default: break;
    }

    af::af_emitter_t<std::vector<std::string>>* emtr = new emitter();
    af::af_collector_t<std::string>* clctr = new collector();

    //need to understand if this is the right way to set farm's time...
    std::chrono::nanoseconds time = std::chrono::nanoseconds(12000);
    af::af_autonomic_farm_t<std::vector<std::string>, std::string>* farm = new af::af_autonomic_farm_t<std::vector<std::string>, std::string>(emtr, clctr, nw, time);
    for(int j = 0; j < nw; j++)
        farm->add_worker(new worker());
    af::utimer tmr2("Autonomic farm time");
    farm->run_auto_farm();
    farm->stop_autonomic_farm();
    auto ttime2 = tmr2.get_time();
    auto ctime2 = std::chrono::duration_cast<std::chrono::microseconds>(ttime2).count();
    std::cout << ctime2 << std::endl;

    return 0;
}