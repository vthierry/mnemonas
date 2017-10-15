#include "main.hpp"

double network::SoftMaxTransform::zexp(double u) const
{
  double e = exp(u);
  static const double MAX_VALUE = 1e10;
  return e == HUGE_VAL ? MAX_VALUE : e;
}
network::SoftMaxTransform::SoftMaxTransform(unsigned int N, const Input& input) : network::KernelTransform(3 * N + 1, 1, input), N(N), N1(N + 1), N2(2 * N), N3(3 * N)
{
  setOffset(0);
}
network::SoftMaxTransform& network::SoftMaxTransform::setOffset(double value)
{
  if(!isnan(value)) {
    offset = NAN;
    for(unsigned int n = N2 + 1; n <= 3 * N; n++)
      setWeight(n, 1, value);
  }
  offset = value;
  return *this;
}
bool network::SoftMaxTransform::setWeight(unsigned int n, unsigned int d, double w)
{
  if((N2 < n) && (n <= N3) && (d == 1) && !isnan(offset))
    return false;
  else
    return KernelTransform::setWeight(n, d, w);
}
network::KernelTransform& network::SoftMaxTransform::setWeights(KernelTransform& network)
{
  assume(dynamic_cast < SoftMaxTransform * > (&network) != NULL, "illegal-argument", "in network::SoftMaxTransform::setWeights wrong network type");
  unsigned int N0 = getN() < network.getN() ? getN() : network.getN();
  for(unsigned int n = 0; n < N0 / 3; n++) {
    unsigned k0 = 1 + 2 * getN() / 3, k1 = 1 + 2 * network.getN() / 3;
    for(unsigned int d = 1; d < getKernelDimension(n + k0); d++)
      setWeight(n + k0, d, network.getWeight(n + k1, d));
  }
  return *this;
}
unsigned int network::SoftMaxTransform::getKernelDimension(unsigned int n) const
{
  return n <= N2 ? 0 : N1 + input.getN();
}
double network::SoftMaxTransform::getKernelValue(unsigned int n, unsigned int d, double t) const
{
  if(n < N)
    return d == 0 ? get(n + N1, t) / get(N, t) : 0;
  else if(n == N) {
    if(d == 0) {
      double v = 0;
      for(unsigned int n_1 = N1; n_1 <= N2; n_1++)
        v += get(n_1, t);
      return v;
    } else
      return 0;
  } else if(n <= N2)
    return d == 0 ? zexp(get(n + N, t)) : 0;
  else {
    if(d == 0)
      return 0;
    if(d == 1)
      return 1;
    d -= 2;
    if(d < N)
      return get(d, t - 1);
    d -= N;
    if(d < input.getN())
      return input.get(d, t - 1);
    return 0;
  }
}
double network::SoftMaxTransform::getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
{
  if(n < N)
    return d == 0 && t_ == t ?
           (n_ == N ? -get(n + N1, t) / (get(N, t) * get(N, t)) :
            (n_ == n + N1 ? 1.0 / get(N, t) : 0)) : 0;
  else if(n == N)
    return d == 0 && t_ == t && N < n_ && n_ <= 2 * N ? 1 : 0;
  else if(n <= N2)
    return d == 0 && t_ == t && n_ == n + N ? get(n, t) : 0;
  else
    return 1 < d && d <= N + 1 && n_ == d - 2 && t_ == t - 1 ? 1 : 0;
}
