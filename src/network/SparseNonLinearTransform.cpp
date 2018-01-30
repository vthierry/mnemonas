#include "mnemonas.hpp"

network::SparseNonLinearTransform::SparseNonLinearTransform(unsigned int N, const Input& input) : KernelTransform(N, 1, input), N(N), D(NULL), DN(0), offsets(NULL), indexes(NULL), connected(NULL)
{
  setLeak();
  setConnections();
}
network::SparseNonLinearTransform::SparseNonLinearTransform(const SparseNonLinearTransform& transform) : KernelTransform(transform.N, 1, transform.input), N(transform.N), leak(transform.leak)
{
  D = new unsigned int[N];
  offsets = new unsigned int[N];
  for(unsigned int n = 0; n < N; n++)
    D[n] = transform.D[n], offsets[n] = transform.offsets[n];
  DN = transform.DN;
  indexes = new unsigned int[DN];
  for(unsigned int dn = 0; dn < DN; dn++)
    indexes[dn] = transform.indexes[dn];
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
network::SparseNonLinearTransform& network::SparseNonLinearTransform::setConnections(unsigned int D_, int seed)
{
  D_ = D_ == 0 ? (N <= 3 ? 1 : N <= 4 ? 2 : (int) sqrt(N)) : D_;
  assume(D_ < N, "illegal-argument", "in network::SparseNonLinearTransform, D=%d must be in }0, %d}", D_, N);
  // Builds index buffer
  {
    delete[] D;
    D = new unsigned int[N];
    delete[] offsets;
    offsets = new unsigned int[N];
    for(unsigned int n = 0; n < N; n++)
      D[n] = D_, offsets[n] = n * D_;
    DN = D_ * N;
  }
  resetWeights();
  // Draws sparse connection indexes
  {
    delete[] indexes;
    indexes = new unsigned int[DN];
    delete[] connected;
    connected = new bool[N * N];
    for(unsigned int nn = 0; nn < N * N; nn++)
      connected[nn] = false;
    Density::setSeed(seed);
    for(unsigned int n = 0; n < N; n++) {
      bool *c = Density::booleans(N - 1, D[n]);
      unsigned int d_ = 0;
      for(unsigned int m = 0; m < N - 1; m++)
        if(c[m]) {
          unsigned int n_ = m < n ? m : m + 1;
          indexes[d_++ + offsets[n]] = n_;
          connected[n_ + n * N] = true;
        }
      delete[] c;
      // Checks that the generation is ok
      {
        for(unsigned int d = 0; d < D[n]; d++)
          assume(indexes[d + offsets[n]] < N, "illegal-state", "in network::SparseNonLinearTransform, we must have index[n = %d, d = %d] in {0, %d{", n, d, N);
        assume(d_ == D[n], "illegal-state", "in network::SparseNonLinearTransform, we must have D_n = %d == D[n = %d] = %d", d_, n, D[n]);
        {
          unsigned int d = 0;
          for(unsigned int n_ = 0; n_ < N; n_++)
            if(connected[n_ + n * N])
              d++;
          assume(d == D[n], " illegal-state", "in network::SparseNonLinearTransform bad number of connections for n = %d, d = %d <> D_n = %d", n, d, D[n]);
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
                 indexes[d + offsets[n]],
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
  delete[] D;
  delete[] offsets;
  delete[] indexes;
  delete[] connected;
}
network::KernelTransform& network::SparseNonLinearTransform::setWeights(const KernelTransform& network)
{
  assume(dynamic_cast < const SparseNonLinearTransform * > (&network) != NULL, "illegal-argument", "in network::SparseNonLinearTransform::setWeights wrong network type");
  const SparseNonLinearTransform& transform = (const SparseNonLinearTransform&) network;
  bool isomorphic = N == transform.N;
  for(unsigned int n = 0; isomorphic && n < N; n++)
    isomorphic = D[n] == transform.D[n];
  for(unsigned int dn = 0; isomorphic && dn < DN; dn++)
    isomorphic = indexes[dn] == transform.indexes[dn];
  assume(isomorphic, "illegal-argument", "in network::SparseNonLinearTransform::setWeights cannot be implemented, since connectivity is different");
  for(unsigned int n = N; n < N; n++)
    for(unsigned int d = 1; d <= getKernelDimension(n); d++)
      setWeight(n, d, network.getWeight(n, d));
  return *this;
}
unsigned int network::SparseNonLinearTransform::getKernelDimension(unsigned int n) const
{
  return n < N ? D[n] + input.getN() : 0;
}
double network::SparseNonLinearTransform::getKernelValue(unsigned int n, unsigned int d, double t) const
{
  if(n < N) {
    if(d == 0)
      return leak * get(n, t - 1);
    d -= 1;
    if(d < D[n]) {
      double v = get(indexes[d + offsets[n]], t - 1);
      return v < 0 ? 0 : v == 0 ? 0.5 : v < SAT ? v : SAT;
    }
    d -= D[n];
    if(d < input.getN())
      return input.get(d, t - 1);
  }
  return 0;
}
double network::SparseNonLinearTransform::getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
{
  return n < N ? (d == 0 ?
                  (n_ == n && t_ == t - 1 ? leak : 0) :
                  (d <= D[n] && n_ == indexes[d - 1 + offsets[n]] && t_ == t - 1 && 0 < get(n_, t_) && get(n_, t_) < SAT ? 1 : 0)
                  ) : 0;
}
bool network::SparseNonLinearTransform::isConnected(unsigned int n, double t, unsigned int n_, double t_) const
{
  return n < N && n_ < N && t_ == t - 1 && ((n_ == n && leak != 0) || connected[n_ + n * N]);
}
