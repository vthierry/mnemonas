#include "main.hpp"

#include "inc/s_regex_test.h"
#include "inc/Struct_test.h"
#include "inc/solver_test.h"
#include "inc/random_test.h"
#include "network/network_test.h"

void test()
{
  printf("testing ...\n");
  s_regex_test();
  Struct_test();
  solver_test();
  random_test();
  network_test();
  printf(" ... done.\n");
}
#include "../tex/run/reverse_engineering.h"
#include "../tex/run/sequence_generation.h"

int main(int argc, const char *argv[])
{
#ifdef TOTO
  printf(">>> TOTI\n");
#endif
  Struct args(argc, argv);
  args.set("test");
  {
    if(args.get("test"))
      test();
    if(args.get("reverse-engineering"))
      reverse_engineering();
  }
  return EXIT_SUCCESS;
}
