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
                        //if service time is too large, resize

                        //this tells the emitter to stop
                        //without sending AF_EOS
                        this->emitter->set_remaining_jobs(true);
                    }
                }

            protected:


            public:
                af_autonomic_farm_t(af::af_emitter_t<Tin, Tin>* em) {
                    this->emitter = em;
                    remaining = new std::vector<Tin*>();
                }
        };
}

#endif