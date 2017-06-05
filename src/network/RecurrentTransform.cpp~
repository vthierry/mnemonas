#include "main.hpp"

network::RecurrentTransform::RecurrentTransform(const RecurrentTransform& transform) : RecurrentTransform(transform.N, transform.R, transform.input) {}
network::RecurrentTransform::RecurrentTransform(unsigned int N, unsigned int R, const Input& input) : Transform(N, input), values(NULL), t0(0), R(R), L(0)
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
double network::RecurrentTransform::get(unsigned int n, double t_) const
{
  if(t_ < 0)
    return 0;
  unsigned int t = (unsigned int) t_;
  if((t0 <= t + L) && (t < t0))
    return values[n + N * (t % L)];
  if((t == 0) && (0 < t0))
    t0 = 0;
  if(t == t0) {
    unsigned int it = N * (t % L);
    t0++;
    for(int n_ = N - 1; 0 <= n_; n_--)
      values[n_ + it] = getValue(n_, t);
    return values[n + it];
  }
  assume(false, "illegal-argument", "in network::RecurrentTransform::get, uncalculable value, we must have %d <= t=%d <= %d", t0 - L, t, t0);
  return NAN;
}
