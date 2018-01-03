#include "main.hpp"

network::SparseNonLinearTransform::SparseNonLinearTransform(unsigned int N, const Input& input) : KernelTransform(2 * N, 1, input), N(N), indexes(NULL), connected(NULL)
{
  setLeak(0);
  setConnections(0);
}
network::SparseNonLinearTransform::SparseNonLinearTransform(const SparseNonLinearTransform& transform) : KernelTransform(2 * transform.N, 1, transform.input), N(transform.N), D(transform.D), leak(transform.leak)
{
  indexes = new unsigned int[N * D];
  for(unsigned int nd = 0; nd < N * D; nd++)
    indexes[nd] = transform.indexes[nd];
  connected = new bool[N * N];
  for(unsigned int nn = 0; nn < N * N; nn++)
    connected[nn] = transform.connected[nn];
}
network::SparseNonLinearTransform& network::SparseNonLinearTransform::setLeak(double value)
{
  assume(0 <= value && value < 1, "illegal-argument", "in network::SparseNonLinearTransform we must have leak = %g in [0,1[", value);
  leak = value;
  return *this;
}
network::SparseNonLinearTransform& network::SparseNonLinearTransform::setConnections(unsigned int D, int seed)
{
  this->D = D = D == 0 ? (N <= 3 ? 1 : N <= 4 ? 2 : (int) sqrt(N)) : D;
  assume(D < N, "illegal-argument", "in network::SparseNonLinearTransform, D must be in }0, %d}", N);
  resetWeights();
  // Draws sparse connection indexes
  {
    delete[] indexes;
    indexes = new unsigned int[N * D];
    delete[] connected;
    connected = new bool[N * N];
    for(unsigned int nn = 0; nn < N * N; nn++)
      connected[nn] = false;
    random::setSeed(seed);
    unsigned int nd = 0;
    for(unsigned int n = 0; n < N; n++) {
      bool *c = random::booleans(N - 1, D);
      for(unsigned int m = 0; m < N - 1; m++)
        if(c[m]) {
          unsigned int n_ = m < n ? m : m + 1;
          indexes[nd++] = n_;
          connected[n_ + n * N] = true;
        }
      delete[] c;
      // Checks that the generation is ok
      {
        for(unsigned int d = 0; d < D; d++)
          assume(indexes[d + n * D] < N, "illegal-state", "in network::SparseNonLinearTransform, we must have index[n = %d, d = %d] in {0, %d{", n, d, N);
        assume(nd == (n + 1) * D, "illegal-state", "in network::SparseNonLinearTransform bad nd = %d <> %d", nd, (n + 1) * D);
        {
          unsigned int d = 0;
          for(unsigned int n_ = 0; n_ < N; n_++)
            if(connected[n_ + n * N])
              d++;
          assume(d == D, " illegal-state", "in network::SparseNonLinearTransform bad number of connections for n = %d, d = %d <> D = %d", n, d, D);
        }
      }
    }
#if 0
    // Dumps the connections
    {
      for(unsigned int n = 0; n < N; n++)
        for(unsigned int d = 0; d < D; d++)
          printf("%s%3d%s",
                 (d == 0 ? (n == 0 ? "{\n  connections : {\n" : "") + s_printf("    '%3d' : [ ", n) : "").c_str(),
                 indexes[d + n * D],
                 (d < D - 1 ? ", " : (" ]" + (std::string) (n < N - 1 ? ",\n" : "\n  }\n}\n"))).c_str());
      for(unsigned int n = 0, nn = 0; n < N; n++)
        for(unsigned int n_ = 0; n_ < N; n_++, nn++)
          printf("%s%1d%s",
                 (n_ == 0 ? (n == 0 ? "[\n" : "\n") + s_printf(" '%3d' : [ ", n) : "").c_str(),
                 connected[n_ + n * N],
                 (n_ < N - 1 ? "" : (" ]" + (std::string) (n < N - 1 ? "," : "\n]\n"))).c_str());
    }
#endif
  }
  return *this;
}
network::SparseNonLinearTransform::~SparseNonLinearTransform()
{
  delete[] indexes;
  delete[] connected;
}
network::KernelTransform& network::SparseNonLinearTransform::setWeights(const KernelTransform& network)
{
  assume(dynamic_cast < const SparseNonLinearTransform * > (&network) != NULL, "illegal-argument", "in network::SparseNonLinearTransform::setWeights wrong network type");
  const SparseNonLinearTransform& transform = (const SparseNonLinearTransform&) network;
  bool isomorphic = N == transform.N && D == transform.D;
  for(unsigned int nd = 0; isomorphic && nd < N * D; nd++)
    isomorphic = indexes[nd] == transform.indexes[nd];
  assume(isomorphic, "illegal-argument", "in network::SparseNonLinearTransform::setWeights cannot be implemented, since connectivity is different");
  for(unsigned int n = N; n < 2 * N; n++)
    for(unsigned int d = 1; d <= getKernelDimension(n); d++)
      setWeight(n, d, network.getWeight(n, d));
  return *this;
}
/*
unsigned int network::SparseNonLinearTransform::getKernelDimension(unsigned int n) const
{
  return n < N ? 0 : D + input.getN();
}
*/
double network::SparseNonLinearTransform::getKernelValue(unsigned int n, unsigned int d, double t) const
{
  if(n < N)
    return d == 0 ? leak *get(n, t - 1) + (get(n + N, t) > 0 ? (get(n + N, t) > SAT ? SAT : get(n + N, t)) : (get(n + N, t) == 0 ? 0.5 : 0)) : 0;
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
/*
double network::SparseNonLinearTransform::getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
{
  return n < N ? (d == 0 ? (n_ == n && t_ == t - 1 ? leak : (n_ == N + n && t_ == t && get(n_, t) > 0 && get(n_, t) <= SAT ? 1 : 0)) : 0) :
         (0 < d && d <= D && n_ == indexes[d - 1 + (n - N) * D] && t_ == t - 1 ? 1 : 0);
}
bool network::SparseNonLinearTransform::isConnected(unsigned int n, unsigned int n_) const
{
  return n < N ? n_ == n || n_ == N + n : n_ < N && connected[n_ + (n - N) * N];
}
*/
