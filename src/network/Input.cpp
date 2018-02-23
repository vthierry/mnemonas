#include "mnemonas.hpp"

network::Input::Input(unsigned int N, double T, bool mutable_) : mutable_(mutable_), N(N), T(T) {}
void network::Input::setN(unsigned int N_)
{
  assume(mutable_, "illegal-state", "in network::Input::setN this parameter is not mutable");
  unsigned int* pN = const_cast < unsigned int * > (&N);
 *pN = N_;
}
void network::Input::setT(double T_)
{
  assume(mutable_, "illegal-state", "in network::Input::setTN this parameter is not mutable");
  double*  pT = const_cast < double * > (&T);
  *pT = T_;
}
double network::Input::get(unsigned int n, double t) const
{
  assume(false, "illegal-state", "in network::Input::get, this virtual method must be overridden");
  return NAN;
}
