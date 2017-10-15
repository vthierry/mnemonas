#include "main.hpp"

network::Input::Input(unsigned int N, double T) : N(N), T(T) {}
double network::Input::get(unsigned int n, double t) const
{
  assume(false, "illegal-state", "in network::Input::get, this virtual method must be overridden");
  return NAN;
}
