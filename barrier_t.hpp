#ifndef AF_BARRIER_HPP
#define AF_BARRIER_HPP

#include <stdlib.h>
#include <pthread.h>
#include <iostream>

namespace af {

    /*
     * Barrier interface  
     */
    struct af_barrier_t {
        virtual bool set_barrier(size_t bar_size) { return false; }
        virtual void apply_barrier(size_t) {}
    };


    /* 
     * Provides an implementation of the Barrier's
     * interface using pthread barriers.
     */
    class Barrier: af_barrier_t {
        public:
            //public constructor
            Barrier(): barrier_size(0) {}
            
            //public destructor
            ~Barrier() {
                if(barrier_size > 0)
                    pthread_barrier_destroy(&barrier); 
            }

            //set up barrier
            //Return true on initialize successfully, false otherwise
            bool set_barrier(size_t bar_size) {

                //Successful initialization
                if(bar_size > 0 && barrier_size == 0) {
                    int err = pthread_barrier_init(&barrier, NULL, bar_size);
                    if(err != 0) {
                        std::cout << "Error on pthread_barrier_init" << std::endl;
                        return false;
                    }
                    barrier_size = bar_size;
                    return true;
                }

                //Barrier is already initialized
                //no need to check for bar_size to be gt 0
                if(barrier_size == bar_size)
                    return true;

                //resize barrier
                if(bar_size > 0 && barrier_size != bar_size) {
                    int err = pthread_barrier_destroy(&barrier);
                    if(err != 0) {
                        std::cout << "Error on destroying old barrier" << std::endl;
                        return false;
                    }
                    err = pthread_barrier_init(&barrier, NULL, bar_size);
                    if(err != 0) {
                        std::cout << "Error on resizing barrier" << std::endl;
                        return false;
                    }
                    barrier_size = bar_size;
                    return true;
                }
                std::cout << "Barrier setup failed" << std::endl;
                return false;                
                
            }


            //block on barrier
            void apply_barrier() {

                pthread_barrier_wait(&barrier);

            }


        private:
            size_t barrier_size;
            pthread_barrier_t barrier;
    };

};

#endif