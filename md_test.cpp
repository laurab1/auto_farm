#include <af_autonomic_farm_t.hpp>
#include <unistd.h>
#include <utimer.hpp>

#include <chrono>

void active_wait(int usecs) {
  auto start = std::chrono::high_resolution_clock::now();
  auto end   = false;
  while(!end) {
    auto elapsed =
      std::chrono::high_resolution_clock::now() - start;
    auto usec =
      std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    if(usec>usecs)
      end = true;
  }
  return;
}

int i = 0;

const bool debug = true;

class emitter: public af::af_emitter_t<int> {
private:
  long usec;
  long taskno;
  long emit;
public:

  emitter(long taskno, long usec):taskno(taskno),usec(usec) {
    emit = 0;
  }
  emitter(long taskno):taskno(taskno),usec(100) {
    emit = 0;
  }
  emitter() : taskno(1000),usec(100) {
    emit = 0;
  }
  
  int* service(int*) {
    active_wait(usec);
    if(emit>=taskno)
      return (int*) af::AF_EOS;
    if(debug)
      std::cout << "Emitter going to emit " << emit << std::endl; 
    return (new int(emit++));
  }

};

class worker: public af::af_worker_t<int, int> {
private:
  long usec;
  int wn; 
public:
  worker(int wn, long usec):wn(wn),usec(usec) {}
  worker(int wn):wn(wn),usec(100)  {}
  worker():wn(-1),usec(100) {}
  
  int* service(int* task) const override {
    if(debug)
      std::cout << "Worker" << wn << " got task " << *task << std::endl;
    active_wait(usec);
    (*task)++;
    return task;
  }
  
  virtual worker* clone() const override {
    if(debug)
      std::cout << "Going to clone worker W" << wn  << std::endl;
    return new worker(usec);
  }
  
};

class collector: public af::af_collector_t<int> {
private:
  long usec;
  
public:

  collector(long usec):usec(usec) {}
  collector() : usec(100)  {}
  
  int* service(int* task) {
    if(debug)
      std::cout << "Collector got result " << " " << (*task) << std::endl;
    return(task);
  }
  
};

int main(int argc, char* argv[]) {

  if(argc!=7) {
    std::cout << "Usage is: " << argv[0]
	      << " usec_e usec_w usec_c usec_farm m nw"
	      << std::endl;
    return(-1);
  }
  long usec_e = atoi(argv[1]);
  long usec_w = atoi(argv[2]);
  long usec_c = atoi(argv[3]);
  long usec_f = atoi(argv[4]);
  
  long m    = atoi(argv[5]);
  size_t nw   = atoi(argv[6]);

  af::af_emitter_t<int>* emtr = new emitter(m, usec_e);
  af::af_collector_t<int>* clctr = new collector(usec_c);
  
  //need to understand if this is the right way to set farm's time...
  std::chrono::microseconds time =
    std::chrono::microseconds(usec_f);
  
  af::af_autonomic_farm_t<int, int>* farm =
    new af::af_autonomic_farm_t<int, int>(emtr, clctr, nw, time);
  for(int j = 0; j < nw; j++)
    farm->add_worker(new worker(j+1,usec_w));
  
  af::utimer tmr("Autonomic farm time");
  farm->run_auto_farm();
  farm->stop_autonomic_farm();
  auto ttime = tmr.get_time();
  auto ctime =
    std::chrono::duration_cast<std::chrono::microseconds>(ttime).count();
  std::cout << "AF Time spent in af_farm with "
	    << nw << " workers : "
	    << ctime << std::endl;
  
  return 0;
}
