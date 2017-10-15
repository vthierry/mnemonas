#include "main.hpp"

#include "util/s_regex_test.h"
#include "util/Struct_test.h"
#include "util/solver_test.h"
#include "util/random_test.h"
#include "util/ExponentialDecayFit_test.h"
#include "network/network_test.h"
#include "../tex/Experimentation.h"

void test()
{
  printf(">  Testing ... \n");
  s_regex_test();
  Struct_test();
  solver_test();
  random_test();
  ExponentialDecayFit_test();
  network_test();
  printf(">  ... test done.\n");
}
int main(int argc, const char *argv[])
{
  Struct args(argc, argv);
  printf("> Running: %s\n", ((String) args).c_str());
  if(args.get("test"))
    test();
  if(args.get("experiment")) {
    Experimentation experimentation;
    experimentation.run(args.get("experiment"));
  }
  printf("> ... run done\n");
  return EXIT_SUCCESS;
}
