///@cond INTERNAL

#include "mnemonas.hpp"

#include "util/s_regex_test.h"
#include "util/Struct_test.h"
#include "util/solver_test.h"
#include "util/Density_test.h"
#include "util/CurveFit_test.h"
#include "util/ParameterOptimizer_test.h"
#include "network/network_test.h"
#include "../tex/BackwardTuning/Experimentation.h"
#include "../tex/ReservoirAdjustment/Experimentation.h"

void test()
{
  printf(">  Testing ... \n");
  s_regex_test();
  Struct_test();
  solver_test();
  Density_test();
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
