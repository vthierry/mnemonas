#include "main.hpp"

double network::IntegrateAndFireTransform::zeta(double u) const
{
  double e = exp(-4 * u);
  return e >= 1e24 ? 0 : 1 / (1 + e);
}
double network::IntegrateAndFireTransform::dzeta(double u) const
{
  double e = exp(-4 * u), d = 1 + e;
  return e >= 1e24 ? 0 : 4 * e / (d * d);
}
network::IntegrateAndFireTransform::IntegrateAndFireTransform(unsigned int N, const Input& input) : KernelTransform(2 * N, 1, input), N(N), leak(NAN), offset(NAN)
{
  setOffset(0).setLeak(0).setSharpness(1);
}
network::IntegrateAndFireTransform& network::IntegrateAndFireTransform::setSharpness(double value)
{
  if(!isnan(value)) {
    sharpness = NAN;
    for(unsigned int n = N; n < N * 2; n++)
      setWeight(n, 1, value);
  }
  sharpness = value;
  return *this;
}
network::IntegrateAndFireTransform& network::IntegrateAndFireTransform::setLeak(double value)
{
  if(!isnan(value)) {
    leak = NAN;
    for(unsigned int n = 0; n < N; n++)
      setWeight(n, 1, value);
  }
  leak = value;
  return *this;
}
network::IntegrateAndFireTransform& network::IntegrateAndFireTransform::setOffset(double value)
{
  if(!isnan(value)) {
    offset = NAN;
    for(unsigned int n = 0; n < N; n++)
      setWeight(n, 2, value);
  }
  offset = value;
  return *this;
}
bool network::IntegrateAndFireTransform::setWeight(unsigned int n, unsigned int d, double w)
{
  if(((N <= n) && (d == 1) && !isnan(sharpness)) ||
     ((n < N) && (((d == 1) && !isnan(leak)) ||
                  ((d == 2) && !isnan(offset)))))
    return false;
  else if((N <= n) && (d == 1))
    return KernelTransform::setWeight(n, d, w < 0 ? 0 : 100 < w ? 100 : w);
  else if((N < n) && (d == 1))
    return KernelTransform::setWeight(n, d, w < 0 ? 0 : 0.999 < w ? 0.999 : w);
  else
    return KernelTransform::setWeight(n, d, w);
}
network::KernelTransform& network::IntegrateAndFireTransform::setWeights(const network::KernelTransform& network)
{
  assume(dynamic_cast < const IntegrateAndFireTransform * > (&network) != NULL, "illegal-argument", "in network::IntegrateAndFireTransform::setWeights wrong network type");
  unsigned int N0 = getN() < network.getN() ? getN() : network.getN();
  for(unsigned int n = 0; n < N0 / 2; n++)
    for(unsigned int k = 0, k0 = 0, k1 = 0; k < 2; k++, k0 += getN() / 2, k1 += network.getN() / 2)
      for(unsigned int d = 1; d < getKernelDimension(n + k0); d++)
        setWeight(n + k0, d, network.getWeight(n + k1, d));
  return *this;
}
unsigned int network::IntegrateAndFireTransform::getKernelDimension(unsigned int n) const
{
  return n < N ? 2 + N + input.getN() : 1;
}
double network::IntegrateAndFireTransform::getKernelValue(unsigned int n, unsigned int d, double t) const
{
  if(n < N) {
    if(d == 0)
      return 0;
    if(d == 1)
      return 0.5 * (1 - zeta(get(n + N, t))) * get(n, t - 1);
    if(d == 2)
      return 1;
    d -= 3;
    if(d < N)
      return zeta(get(d + N, t));
    d -= N;
    if(d < input.getN())
      return input.get(d, t - 1);
    return 0;
  } else
    return d == 1 ? (get(n - N, t - 1) - 0.5) : 0;
}
double network::IntegrateAndFireTransform::getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
{
  return n < N ?
         (d == 1 ?
          (n_ == n && t_ == t - 1 ? 0.5 * (1 - zeta(get(n + N, t))) :
           n_ == n + N && t_ == t ? -0.5 * dzeta(get(n_, t)) * get(n, t - 1) : 0) :
          3 <= d && d < N + 3 && n_ == N + d - 3 && t_ == t ? dzeta(get(n_, t)) : 0) :
         d == 1 && n_ == n - N && t_ == t - 1 ? 1 : 0;
}
bool network::IntegrateAndFireTransform::isConnected(unsigned int n, double t, unsigned int n_, double t_) const
{
  return n < N ? ((leak != 0 && t_ == t - 1 && n_ == n) || (t_ == t && N <= n_)) : (t_ == t - 1 && n_ == n - N);
}
