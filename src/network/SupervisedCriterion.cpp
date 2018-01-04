#include "main.hpp"

network::SupervisedCriterion::SupervisedCriterion(network::KernelTransform& transform, Input& values, char criterion, double nu, char reinject) : network::TransformCriterion(transform), values(values), criterion(criterion), nu(nu), reinject(reinject), estimates(NULL), destimates(NULL)
{
  assume(0 <= nu, "illegal-argument", "in network::SupervisedCriterion::SupervisedCriterion the nu=%g must be non-negative criterion %c", nu, criterion);
  assume(reinject == 'n' || reinject == 'o' || reinject == 'b', "illegal-argument", "in network::SupervisedCriterion::SupervisedCriterion the reinject=%c parameter must be in {'t', 'o', 'n'}", reinject);
}
/// @cond INTERNAL
network::SupervisedCriterion::~SupervisedCriterion()
{
  delete[] estimates;
  delete[] destimates;
}
///@endcond
double network::SupervisedCriterion::rho(unsigned int n, double t) const
{
  double value = transform.get(n, t);
  if(n < values.getN())
    switch(criterion) {
    case '2':
    {
      double e = value - values.get(n, t);
      return 0.5 * e * e;
    }
    case '1':
    {
      double e = value - values.get(n, t), e0 = get(n, t) - values.get(n, t), d = nu + fabs(e0);
      return d < 1e-6 ? fabs(e) : 0.5 * e * e / d;
    }
    case '0':
    {
      double e = value - values.get(n, t), e0 = get(n, t) - values.get(n, t), d = nu + e0 * e0;
      return d < 1e-6 ? (fabs(e) < 1e-12 ? 0 : 1) : 0.5 * e * e / d;
    }
    case 'a':
    {
      double e = fabs(value - values.get(n, t));
      return e < nu ? 0.5 * e * e / nu : e - nu / 2;
    }
    case 'b':
    {
      if(nu < 1e-6)
        return value == values.get(n, t) ? 0 : 1;
      else {
        double e = (value - values.get(n, t)) / nu, e2 = e * e;
        return e2 < 1 ? (e2 * (e2 - 3) + 3) * e2 : 1;
      }
    }
    case 'h':
    {
      if(nu == 0)
        return (values.get(n, t) - 0.5) * (value - 0.5) < 0 ? 1 : 0;
      else {
        double e = nu - (values.get(n, t) < 0.5 ? -1 : 1) * (value - 0.5);
        return e > 0 ? 0.5 * e * e : 0;
      }
    }
    }
  return 0;
}
double network::SupervisedCriterion::drho(unsigned int n, double t) const
{
  double value = transform.get(n, t);
  if(n < values.getN())
    switch(criterion) {
    case '2':
      return value - values.get(n, t);
    case '1':
    {
      double e = value - values.get(n, t), e0 = get(n, t) - values.get(n, t), d = nu + fabs(e0);
      return e / d;
    }
    case '0':
    {
      double e = value - values.get(n, t), e0 = get(n, t) - values.get(n, t), d = nu + e0 * e0;
      return e / d;
    }
    case 'a':
    {
      double e = fabs(value - values.get(n, t));
      return e < nu ? e / nu : value > values.get(n, t) ? 1 : value < values.get(n, t) ? -1 : 0;
    }
    case 'b':
    {
      if(nu < 1e-12)
        return 0;
      else {
        double e = abs((value - values.get(n, t)) / nu), ep = e + 1, en = e - 1;
        return e < 1 ? 6 * e * ep * ep * en * en / nu : 0;
      }
    }
    case 'h':
    {
      if(nu == 0)
        return 0;
      else {
        double s = -(values.get(n, t) < 0.5 ? -1 : 1), e = nu + s * (value - 0.5);
        return e > 0 ? s * e : 0;
      }
    }
    }
  return 0;
}
double network::SupervisedCriterion::get(unsigned int n, double t) const
{
  if(reinject != 'n') {
    if(reinject == 'b' && estimates != NULL)
      return estimates[n + transform.getN() * (int) t];
    if(n < values.getN())
      return values.get(n, t);
  }
  return NAN;
}
unsigned int network::SupervisedCriterion::getN0() const
{
  return reinject == 'b' && estimates != NULL ? transform.getN(): reinject == 'o' ? values.getN() : 0;
}
void network::SupervisedCriterion::update()
{
  if(reinject) {
    if(estimates == NULL) {
      estimates = new double[transform.getN() * ((int) transform.getT())];
      destimates = new double[transform.getN() * ((int) transform.getT())];
    }
    unsigned int N = transform.getN(), N0 = values.getN(), T = transform.getT(), R = transform.getR();
    transform.reset(true);
    double r = 1;
    static const unsigned int K = 20;
    for(unsigned int k = 0; k < K && 1e-6 < r; k++) {
      for(unsigned int t = 0; t < T; t++)
        for(int n = N - 1; 0 <= n; n--) {
          double v = transform.get(n, t);
          estimates[n + t * N] = n < (int) N0 ? values.get(n, t) : v;
        }
      if(k > 0)
        for(int t_ = T - 1; 0 <= t_; t_--)
          for(unsigned int n_ = 0, nt_ = N * t_; n_ < N; n_++, nt_++)
            if(N0 <= n_)
              for(int t = t_; t_ <= t + (int) R && t < (int) T; t++)
                for(unsigned int n = 0; n < (t == t_ ? n_ : N); n++)
                  estimates[n_ + t_ * N] += transform.getValueDerivative(n, t, n_, t_) * destimates[n + t * N];
      r = 0;
      for(unsigned int t = 0; t < T; t++)
        for(unsigned int n = 0; n < N; n++) {
          double e = destimates[n + t * N] = estimates[n + t * N] - transform.get(n, t);
          r += e * e;
        }
      r = sqrt(r / (N * T));
      for(unsigned int t = 0; t < T; t++)
        for(unsigned int n = 0; n < N; n++)
          transform.set(n, t, estimates[n + t * N]);
      // - printf("network::SupervisedCriterion::update() r[%d] = %g\n", k, r); // @todo
    }
  }
}
