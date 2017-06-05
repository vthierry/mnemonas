#include "main.hpp"
#include "inc/s_printf.h"

network::KernelTransform::KernelTransform(const KernelTransform& transform) : KernelTransform(transform.N, transform.R, transform.input)
{
  root = &transform;
}
network::KernelTransform::KernelTransform(unsigned int N, unsigned int R, const Input& input) : RecurrentTransform(N, R, input), weights(NULL), offsets(NULL), count(0), root(NULL) {}
void network::KernelTransform::init()
{
  offsets = new unsigned int[N];
  count = 0;
  for(unsigned int n = 0; n < N; n++) {
    offsets[n] = count - 1;
    count += getKernelDimension(n);
  }
  weights = new double[count];
  for(unsigned i = 0; i < count; i++)
    weights[i] = root == NULL ? 0 : root->weights[i];
}
network::KernelTransform::~KernelTransform()
{
  delete[] weights;
  delete[] offsets;
}
network::KernelTransform& network::KernelTransform::setWeightsRandom(double mean, double sigma, bool add, String mode, int seed)
{
  random::setSeed(seed);
  for(unsigned int n = 0; n < N; n++)
    for(unsigned int d = 1; d <= getKernelDimension(n); d++)
      setWeight(n, d,
                (mode == "normal" ?
                 random::gaussian(mean, sigma) :
                 random::uniform(mean - sqrt(3) * sigma, mean + sqrt(3) * sigma)) +
                (add ? getWeight(n, d) : 0));
  return *this;
}
std::string network::KernelTransform::asString() const
{
  std::string json;
  json = s_printf("{\n 'N' : %d,\n 'D' : [", N);
  for(unsigned int n = 0; n < N; n++)
    json += s_printf("%d%s", getKernelDimension(n), n < N - 1 ? ", " : "],\n 'W' : [\n");
  for(unsigned int n = 0; n < N; n++)
    for(unsigned int d = 1; d <= getKernelDimension(n); d++)
      json += s_printf("%s%6.1g%s", d == 1 ? "   [" : "", getWeight(n, d), d < getKernelDimension(n) ? ", " : n < N - 1 ? "],\n" : "]\n ]\n}");
  return json;
}
double network::KernelTransform::getValue(unsigned int n, double t) const
{
  double v = getKernelValue(n, 0, t);
  for(unsigned int d = 1; d <= getKernelDimension(n); d++)
    v += getWeight(n, d) * getKernelValue(n, d, t);
  return v;
}
double network::KernelTransform::getValueDerivative(unsigned int n, double t, unsigned int n_, double t_) const
{
  double v = getKernelDerivative(n, 0, t, n_, t_);
  for(unsigned int d = 1; d <= getKernelDimension(n); d++)
    v += getWeight(n, d) * getKernelDerivative(n, d, t, n_, t_);
  return v;
}
