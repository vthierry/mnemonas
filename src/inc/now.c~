#include "now.h"

#include "assume.h"
#include <cmath>
#include <sys/time.h>
#include <sys/resource.h>

double now(bool daytime)
{
  if(daytime) {
    static double t0 = NAN;
    struct timeval time;
    assume(gettimeofday(&time, NULL) == 0, "illegal-state", "in now bad gettimeofday() system call");
    double t = time.tv_sec + 1e-6 * time.tv_usec;
    if(std::isnan(t0))
      t0 = t;
    return t - t0;
  } else {
    static double t0 = NAN;
    struct rusage usage;
    assume(getrusage(RUSAGE_SELF, &usage) == 0, "illegal-state", "in now bad getrusage() system call");
    double t = usage.ru_utime.tv_sec + 1e-6 * usage.ru_utime.tv_usec;
    if(std::isnan(t0))
      t0 = t;
    return t - t0;
  }
}
static double just_to_init_the_now_variables = now(false) + now(true);
