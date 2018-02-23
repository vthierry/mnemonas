#include "mnemonas.hpp"

network::RecurrentTransform::RecurrentTransform(unsigned int N, unsigned int R, const Input& input) : Transform(N, input), values(NULL), upsilon0(0), t0(0), no_recursion(true), R(R), L(0)
{
  reset();
}
/// @cond INTERNAL
network::RecurrentTransform::~RecurrentTransform()
{
  delete[] values;
}
///@endcond
network::RecurrentTransform& network::RecurrentTransform::reset(bool buffered, double upsilon)
{
  assume(upsilon >= 0, "illegal-argument", "in network::RecurrentTransform::reset, we must have upsilon=%g >= 0", upsilon);
  upsilon0 = upsilon;
  unsigned int size = buffered ? input.T : R + 1;
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
  assume(false, "illegal-state", "in network::RecurrentTransform::getValue, this virtual method must be overridden");
  return NAN;
}
double network::RecurrentTransform::getValueDerivative(unsigned int n, double t, unsigned int n_, double t_) const
{
  assume(false, "illegal-state", "in network::RecurrentTransform::getValueDerivative, this virtual method must be overridden");
  return NAN;
}
bool network::RecurrentTransform::isConnected(unsigned int n, double t, unsigned int n_, double t_) const
{
  assume(false, "illegal-state", "in network::RecurrentTransform::isConnected, this virtual method must be overridden");
  return true;
}
double network::RecurrentTransform::get(unsigned int n, double t_) const
{
  assume(n < N, "illegal-argument", "in network::RecurrentTransform::get unit index out of range, we must have n=%d in {0, %d{", n, N);
  assume(no_recursion || t_ < t_current || (t_ == t_current && n_current < (int) n), "illegal-argument", "in network::RecurrentTransform::get non causal get(%d, %0f) while at (%d, %d)", n, t_, n_current, t_current);
  if(t_ < 0)
    return 0; // upsilon0 == 0 ? 0 : Density::gaussian(0, upsilon0);
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
  double d = 0.5 * (v_p - v_m) / epsilon;
#if 1
  if((dynamic_cast < const KernelTransform * > (this) != NULL) && (1e-2 <= fabs(d - getValueDerivative(n, t, n_, t_)))) {
    KernelTransform& transform = (KernelTransform&) *this;
    assume(false, " illegal-argument", "in network::RecurrentTransform::getValueDerivativeApproximation(n=%d < N=%d, t=%.0g, n_=%d, t_=%0.g) numerical error:", n, N, t, n_, t_);
    for(unsigned int d = 0; d <= transform.getKernelDimension(n); d++) {
      values[nit] = x_nt_ + dx;
      double v_p = transform.getKernelValue(n, d, t);
      values[nit] = x_nt_ - dx;
      double v_m = transform.getKernelValue(n, d, t);
      values[nit] = x_nt_;
      double d1 = 0.5 * (v_p - v_m) / epsilon;
      printf(">\t[d=%d getKernelDerivative=(%8.1g == %8.1g) at get(n,t) = %8.1g get(n_, t_) = %8.1g]\n",
             d, transform.getKernelDerivative(n, d, t, n_, t_), d1,
             transform.getValue(n, t), transform.getValue(n_, t_));
    }
  }
#endif
  return d;
}
Histogram network::RecurrentTransform::getLyapunovExponent(unsigned int W, unsigned int K, double d0)
{
  assume(0 < W && W < T - 1, "illegal-argument", "in network::RecurrentTransform::getLyapunovExponent we must have W=%d in }0, T-1= %d{", W, T - 1);
  assume(0 < K, "illegal-argument", "in network::RecurrentTransform::getLyapunovExponent we must have K > 0");
  assume(0 < d0, "illegal-argument", "in network::RecurrentTransform::getLyapunovExponent we must have d0 > 0");
  BufferedInput reference(*this);
  Histogram values;
  for(unsigned int k = 0; k < K; k++)
    for(unsigned int t = 0; t < T; t++) {
      if(t == T - W) {
        // Adds a random perturbation of magnitude d0
        double d[N], d2 = 0;
        for(unsigned int n = 0; n < N; n++)
          d[n] = Density::uniform(), d2 += d[n] + d[n];
        for(int n = N - 1; 0 <= n; n--)
          set(n, t, get(n, t) + d0 * d[n] / sqrt(d2));
      } else if(t == T - 1) {
        // Measures the divergence
        double d2 = 0, e1;
        for(int n = N - 1; 0 <= n; n--)
          e1 = get(n, t) - reference.get(n, t), d2 += e1 * e1;
        values.add(d2 > 0 ? log(sqrt(d2) / d0) / log(2) : 0);
      } else
        for(int n = N - 1; 0 <= n; n--)
          get(n, t);
    }
  return values;
}
std::string network::RecurrentTransform::asString() const
{
  return s_printf("{ 'N' : %d, 'R' : %d, 'L' : %d, 'M' : %d, 'T' : %.0f }", N, R, L, input.N, input.T);
}
