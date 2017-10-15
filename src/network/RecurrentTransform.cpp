#include "main.hpp"

network::RecurrentTransform::RecurrentTransform(const RecurrentTransform& transform) : RecurrentTransform(transform.N, transform.R, transform.input) {}
network::RecurrentTransform::RecurrentTransform(unsigned int N, unsigned int R, const Input& input) : Transform(N, input), values(NULL), t0(0), no_recursion(true), R(R), L(0)
{
  reset();
}
network::RecurrentTransform::~RecurrentTransform()
{
  delete[] values;
}
network::RecurrentTransform& network::RecurrentTransform::reset(bool buffered)
{
  unsigned int size = buffered ? input.getT() : R + 1;
  if(size != L) {
    delete[] values;
    values = new double[N * (L = size)];
  }
  for(unsigned int nr = 0; nr < N * L; values[nr++] = NAN) ;
  t0 = 0;
  return *this;
}
double network::RecurrentTransform::getValue(unsigned int n, double t) const
{
  assume(false, "illegal-state", "in network::RecurrentTransform::get, this virtual method must be overridden");
  return NAN;
}
double network::RecurrentTransform::getValueDerivative(unsigned int n, double t, unsigned int n_, double t_) const
{
  assume(false, "illegal-state", "in network::RecurrentTransform::get, this virtual method must be overridden");
  return NAN;
}
double network::RecurrentTransform::get(unsigned int n, double t_) const
{
  assume(n < N, "illegal-argument", "in network::RecurrentTransform::get unit index out of range, we must have n=%d in {0, %d{", n, N);
  assume(no_recursion || t_ < t_current || (t_ == t_current && n_current < (int) n), "illegal-argument", "in network::RecurrentTransform::get non causal get(%d, %0f) while at (%d, %d)", n, t_, n_current, t_current);
  if(t_ < 0)
    return 0;
  unsigned int t = (unsigned int) t_, it = N * (t % L);
  if((t0 <= t + L) && (t < t0))
    return values[n + it];
  if((t == 0) && (0 < t0))
    t0 = 0;
  if(t == t0) {
    t0++;
    {
      no_recursion = false;
      t_current = t;
      for(n_current = N - 1; 0 <= n_current; n_current--) {
        values[n_current + it] = getValue(n_current, t_current);
        assume(!std::isnan(values[n_current + it]), "numerical-error", "in network::RecurrentTransform::get get(%d, %d) is NAN", n_current, t_current);
      }
      no_recursion = true;
    }
    return values[n + it];
  }
  assume(false, "illegal-argument", "in network::RecurrentTransform::get, time value out of range, we must have %d <= t=%d <= %d", t0 - L, t, t0);
  return NAN;
}
void network::RecurrentTransform::set(unsigned int n, double t_, double value)
{
  unsigned int t = (unsigned int) t_, it = N * (t % L);
  assume(t0 <= t + L && t < t0, "illegal-argument", "in network::RecurrentTransform::set, time value out of range, we must have %d <= t=%d <= %d", t0 - L, t, t0);
  values[n + it] = value;
}
double network::RecurrentTransform::getValueDerivativeApproximation(unsigned int n, double t, unsigned int n_, double t_, double epsilon) const
{
  assume(1e-12 < epsilon, "illegal-argument", "in network::RecurrentTransform::getValueDerivativeApproximation spurious value of epsilon = %g", epsilon);
  assume(n < N && n_ < N, "illegal-argument", "in network::RecurrentTransform::getValueDerivativeApproximation incoherent indexes, we must have n=%d & n_=%d < N=%d", n, n_, N);
  assume(t0 <= t + L && t < t0, "illegal-argument", "in network::RecurrentTransform::getValueDerivativeApproximation the t time value is out of range, we must have %d <= t=%.0f <= %d", t0 - L, t, t0);
  assume(t0 <= t_ + L && t_ < t0, "illegal-argument", "in network::RecurrentTransform::getValueDerivativeApproximation the t_ time value is out of range, we must have %d <= t=%.0f <= %d", t0 - L, t_, t0);
  assume(t > t_ || (t == t_ && n < n_), "illegal-argument", "in network::RecurrentTransform::getValueDerivativeApproximation non causal derivative with (n,t) = (%d,%.0f) and (n_,t_) = (%d,%.0f)", n, t, n_, t_);
  unsigned int nit = n_ + N * ((int) t_ % L);
  double x_nt_ = values[nit];
  double dx = 1e-12 < fabs(x_nt_) && fabs(x_nt_) < epsilon ? fabs(x_nt_) - 1e-12 : epsilon; // This patch attemps to avoid change of sign when using a rectification
  values[nit] = x_nt_ + dx;
  double v_p = getValue(n, t);
  values[nit] = x_nt_ - dx;
  double v_m = getValue(n, t);
  values[nit] = x_nt_;
  return 0.5 * (v_p - v_m) / epsilon;
}
