#include "main.hpp"

network::SparseNonLinearTransform::SparseNonLinearTransform(unsigned int N, const Input& input) : KernelTransform(2 * N, 1, input), N(N), indexes(NULL)
{
  setLeak(0);
  setConnections(0);
}
network::SparseNonLinearTransform& network::SparseNonLinearTransform::setLeak(double value)
{
  assume(0 <= value && value < 1, "illegal-argument", "in network::SparseNonLinearTransform we must have leak = %g in [0,1[", value);
  leak = value;
  return *this;
}
network::SparseNonLinearTransform& network::SparseNonLinearTransform::setConnections(unsigned int D, int seed)
{
  assume(D <= N, "illegal-argument", "in network::SparseNonLinearTransform, D must be in }0, %d", N);
  this->D = D = D == 0 ? (N <= 4 ? 2 : (int) sqrt(N)) : D;
  resetWeights();
  // Draws sparse connection indexes
  {
    delete[] indexes;
    indexes = new unsigned int[N * D];
    random::setSeed(seed);
    unsigned int nd = 0;
    for(unsigned int n = 0; n < N; n++) {
      bool *c = random::booleans(N, D);
      unsigned int d = 0;
      for(unsigned int m = 0; m < N; m++)
        if(c[m])
          indexes[nd++] = d++;
      delete[] c;
      // Checks that the generation is ok
      {
        assume(d == D, "illegal-state", "in network::SparseNonLinearTransform at n = %d, bad d = %d <> %d", n, d, D);
        for(unsigned int d = 0; d < D; d++)
          assume(indexes[d + n * D] < N, "illegal-state", "in network::SparseNonLinearTransform, we must have index[n = %d, d = %d] in {0, %d{", n, d, N);
        assume(nd == (n + 1) * D, "illegal-state", "in network::SparseNonLinearTransform bad nd = %d <> %d", nd, N * D);
      }
    }
  }
  return *this;
}
network::SparseNonLinearTransform::~SparseNonLinearTransform()
{
  delete[] indexes;
}
network::KernelTransform& network::SparseNonLinearTransform::setWeights(KernelTransform& network)
{
  assume(false, "illegal-argument", "in network::SparseNonLinearTransform::setWeights cannot be implemented");
  return *this;
}
unsigned int network::SparseNonLinearTransform::getKernelDimension(unsigned int n) const
{
  return n < N ? 0 : D + input.getN();
}
double network::SparseNonLinearTransform::getKernelValue(unsigned int n, unsigned int d, double t) const
{
  if(n < N)
    return d == 0 ? leak *get(n, t - 1) + (get(n + N, t) > 0 ? get(n + N, t) : (get(n + N, t) == 0 ? 0.5 : 0)) : 0;
  else {
    d -= 1, n -= N;
    if(d < D)
      return get(indexes[d + n * D], t - 1);
    d -= D;
    if(d < input.getN())
      return input.get(d, t - 1);
    return 0;
  }
}
double network::SparseNonLinearTransform::getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
{
  return n < N ? (d == 0 ? (n_ == n && t_ == t - 1 ? leak : (n_ == N + n && t_ == t && get(n_, t) > 0 ? 1 : 0)) : 0) :
         (0 < d && d <= D && n_ == indexes[d - 1 + (n - N) * D] && t_ == t - 1 ? 1 : 0);
}
