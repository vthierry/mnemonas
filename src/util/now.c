#include "now.h"
#include "assume.h"

#include <cmath>
#include <sys/time.h>
#include <sys/resource.h>

double now(bool daytime, bool relative)
{
  if(daytime) {
    static double t0 = NAN, t1 = NAN;
    struct timeval time;
    assume(gettimeofday(&time, NULL) == 0, "illegal-state", "in now bad gettimeofday() system call");
    double t = time.tv_sec + 1e-6 * time.tv_usec;
    if(std::isnan(t0))
      t0 = t1 = t;
    double dt = relative ? t - t1 : t - t0;
    t1 = t;
    return dt;
  } else {
    static double t0 = NAN, t1 = NAN;
    struct rusage usage;
    assume(getrusage(RUSAGE_SELF, &usage) == 0, "illegal-state", "in now bad getrusage() system call");
    double t = usage.ru_utime.tv_sec + 1e-6 * usage.ru_utime.tv_usec;
    if(std::isnan(t0))
      t0 = t1 = t;
    double dt = relative ? t - t1 : t - t0;
    t1 = t;
    return dt;
  }
}
///@cond INTERNAL
static double now_static_variables_initialization = now(false) + now(true);
///@endcond
