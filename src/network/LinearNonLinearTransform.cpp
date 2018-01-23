#include "mnemonas.hpp"

double network::LinearNonLinearTransform::zeta(double u) const
{
  static const double MAX_VALUE = 1e6;
  if(b == INFINITY)
    return a + (u <= 0 ? 0 : u < MAX_VALUE ? u : MAX_VALUE);
  double e = exp(c * u);
  return e >= 1e24 ? a : a + b / (1 + e);
}
double network::LinearNonLinearTransform::dzeta(double u) const
{
  if(b == INFINITY)
    return u > 0 ? 1 : u == 0 ? 0.5 : 0;
  double e = exp(c * u), d = 1 + e;
  return e >= 1e24 ? 0 : -b * c * e / (d * d);
}
network::LinearNonLinearTransform::LinearNonLinearTransform(unsigned int N, const Input& input, double min, double max) : network::KernelTransform(3 * N, 1, input), N(N), N2(2 * N), a(min)
{
  assume(min < max, "illegal-argument", "in network::LinearNonLinearTransform::LinearNonLinearTransform, spurious non-linearity range [%g, %g]", min, max);
  b = max == INFINITY ? INFINITY : max - min, c = max == INFINITY ? 0 : -4 / (max - min);
  setLeak(0);
  setOffset(0);
}
network::LinearNonLinearTransform& network::LinearNonLinearTransform::setLeak(double value)
{
  if(!isnan(value)) {
    leak = NAN;
    for(unsigned int n = N; n < N2; n++)
      setWeight(n, 1, value);
  }
  leak = value;
  return *this;
}
network::LinearNonLinearTransform& network::LinearNonLinearTransform::setOffset(double value)
{
  if(!isnan(value)) {
    offset = NAN;
    for(unsigned int n = N2; n < N2 + N; n++)
      setWeight(n, 1, value);
  }
  offset = value;
  return *this;
}
bool network::LinearNonLinearTransform::setWeight(unsigned int n, unsigned int d, double w)
{
  if(((N <= n) && (n < N2) && (d == 1) && !isnan(leak)) ||
     ((N2 <= n) && (d == 1) && !isnan(offset)))
    return false;
  else if((N <= n) && (n < N2) && (d == 1))
    return KernelTransform::setWeight(n, d, w < 0 ? 0 : 0.999 < w ? 0.999 : w);
  else
    return KernelTransform::setWeight(n, d, w);
}
network::KernelTransform& network::LinearNonLinearTransform::setWeights(const network::KernelTransform& network)
{
  assume(dynamic_cast < const LinearNonLinearTransform * > (&network) != NULL, "illegal-argument", "in network::LinearNonLinearTransform::setWeights wrong network type");
  unsigned int N0 = getN() < network.getN() ? getN() : network.getN();
  for(unsigned int n = N0; n < N0 / 3; n++)
    for(unsigned int k = 0, k0 = 0, k1 = 0; k < 3; k++, k0 += getN() / 3, k1 += network.getN() / 3)
      for(unsigned int d = 1; d < getKernelDimension(n + k0); d++)
        setWeight(n + k0, d, network.getWeight(n + k1, d));
  return *this;
}
unsigned int network::LinearNonLinearTransform::getKernelDimension(unsigned int n) const
{
  return n < N ? 0 : n < N2 ? 1 : 1 + N + input.getN();
}
double network::LinearNonLinearTransform::getKernelValue(unsigned int n, unsigned int d, double t) const
{
  if(n < N)
    return d == 0 ? get(n + N, t) + zeta(get(n + N2, t)) : 0;
  else if(n < N2)
    return d == 1 ? get(n - N, t - 1) : 0;
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
double network::LinearNonLinearTransform::getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
{
  return n < N ? (d == 0 && t_ == t ?
                  (n_ == n + N ? 1 :
                   (n_ == n + N2 ? dzeta(get(n_, t)) : 0)) : 0) :
         n < N2 ? (d == 1 && n_ == n - N && t_ == t - 1 ? 1 : 0) :
         (1 < d && d <= N + 1 && n_ == d - 2 && t_ == t - 1 ? 1 : 0);
}
bool network::LinearNonLinearTransform::isConnected(unsigned int n, double t, unsigned int n_, double t_) const
{
  return n < N ? (t_ == t && (n_ == N + n || n_ == n + N2)) : n < N2 ? (t_ == t - 1 && n_ == n - N) : (t_ == t - 1 && n_ < N);
}
