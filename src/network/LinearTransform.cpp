#include "main.hpp"

network::LinearTransform::LinearTransform(unsigned int N, const Input& input) : KernelTransform(N, 1, input)
{
  setOffset(0);
}
double network::LinearTransform::sat(double v) const
{
  static const double MAX_VALUE = 1e6, MIN_VALUE = -MAX_VALUE;
  return v < MIN_VALUE ? MIN_VALUE : v > MAX_VALUE ? MAX_VALUE : v;
}
network::LinearTransform& network::LinearTransform::setOffset(double value)
{
  if(!isnan(value)) {
    offset = NAN;
    for(unsigned int n = 0; n < N; n++)
      setWeight(n, 1, value);
  }
  offset = value;
  return *this;
}
bool network::LinearTransform::setWeight(unsigned int n, unsigned int d, double w)
{
  if((d == 1) && !isnan(offset))
    return false;
  else
    return KernelTransform::setWeight(n, d, w);
}
network::KernelTransform& network::LinearTransform::setWeights(const network::KernelTransform& network)
{
  assume(dynamic_cast < const LinearTransform * > (&network) != NULL, "illegal-argument", "in network::LinearTransform::setWeights wrong network type");
  unsigned int N0 = getN() < network.getN() ? getN() : network.getN();
  for(unsigned int n = 0; n < N0; n++)
    for(unsigned int d = 1; d < getKernelDimension(n); d++)
      setWeight(n, d, network.getWeight(n, d));
  return *this;
}
unsigned int network::LinearTransform::getKernelDimension(unsigned int n) const
{
  return 1 + getN() + input.getN();
}
double network::LinearTransform::getKernelValue(unsigned int n, unsigned int d, double t) const
{
  assume(n < N && d <= getKernelDimension(n), "illegal-argument", "network::LinearTransform::getKernelValue(%d, %d, t) out of bounds", n, d);
  if(d > 0) {
    if(d == 1)
      return 1;
    d -= 2;
    if(d < getN())
      return sat(get(d, t - 1));
    d -= getN();
    if(d < input.getN())
      return input.get(d, t - 1);
  }
  return 0;
}
double network::LinearTransform::getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
{
  assume(n < N && d <= getKernelDimension(n) && n_ < N, "illegal-argument", "network::LinearTransform::getKernelDerivative(%d, %d, t, %d, t_) out of bounds", n, d, n_);
  return (1 < d && n_ == d - 2 && t_ == t - 1 && d <= getN() + 1) ? 1 : 0;
}
bool network::LinearTransform::isConnected(unsigned int n, unsigned int n_) const
{
  return true;
}
