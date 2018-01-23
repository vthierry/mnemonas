///@cond INTERNAL

#include "mnemonas.hpp"

#include "util/s_regex_test.htt"
#include "util/Struct_test.htt"
#include "util/solver_test.htt"
#include "util/random_test.htt"
#include "util/CurveFit_test.htt"
#include "util/ParameterOptimizer_test.htt"
#include "network/network_test.htt"
#include "../tex/BackwardTuning/Experimentation.h"
#include "../tex/ReservoirAdjustment/Experimentation.h"

void test()
{
  printf(">  Testing ... \n");
  s_regex_test();
  Struct_test();
  solver_test();
  random_test();
  CurveFit_test();
  ParameterOptimizer_test();
  network_test();
  printf(">  ... test done.\n");
}
int main(int argc, const char *argv[])
{
  Struct args(argc, argv);
  if(args.get("test"))
    test();
  if(args.get("experiment1")) {
    Experimentation1 experimentation;
    experimentation.run(args.get("experiment1"));
  }
  if(args.get("experiment2")) {
    Experimentation2 experimentation;
    experimentation.run(args.get("experiment2"));
  }
  printf("> ... run done\n");
  return EXIT_SUCCESS;
}
///@endcond
