#include "main.hpp"

network::KernelTransform::KernelTransform(unsigned int N, unsigned int R, const Input& input) : RecurrentTransform(N, R, input), weights(NULL), offsets(NULL), count(0), root(NULL) {}
void network::KernelTransform::resetWeights()
{
  delete[] offsets;
  offsets = new unsigned int[N];
  count = 0;
  for(unsigned int n = 0; n < N; n++) {
    offsets[n] = count - 1;
    count += getKernelDimension(n);
  }
  delete[] weights;
  weights = new double[count];
  for(unsigned i = 0; i < count; i++)
    weights[i] = root == NULL ? 0 : root->weights[i];
}
/// @cond INTERNAL
network::KernelTransform::KernelTransform(const KernelTransform& transform) : KernelTransform(transform.N, transform.R, transform.input)
{
  root = &transform;
}
network::KernelTransform::~KernelTransform()
{
  delete[] weights;
  delete[] offsets;
}
///@endcond
unsigned int network::KernelTransform::getKernelDimension(unsigned int n) const
{
  assume(false, "illegal-state", "in network::KernelTransform::getKernelDimension, this virtual method must be overridden");
  return 0;
}
double network::KernelTransform::getKernelValue(unsigned int n, unsigned int d, double t) const
{
  assume(false, "illegal-state", "in network::KernelTransform::getKernelValue, this virtual method must be overridden");
  return NAN;
}
double network::KernelTransform::getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
{
  assume(false, "illegal-state", "in network::KernelTransform::getKernelDerivative, this virtual method must be overridden");
  return NAN;
}
/*
double network::KernelTransform::getWeight(unsigned int n, unsigned int d) const
{
  assume(weights != NULL, "illegal-state", "in network::KernelTransform::getWeights uninitialized weights");
  return n < N && 0 < d && d <= getKernelDimension(n) ? weights[offsets[n] + d] : 0;
}
*/
bool network::KernelTransform::setWeight(unsigned int n, unsigned int d, double w)
{
  assume(n < N && 0 < d && d <= getKernelDimension(n), "illegal-argument", "in network::KernelTransform::setWeight, index out of range, we must have n=%d in {0, %d{ and d=%d in {1, %d}", n, N, d, getKernelDimension(n));
  initOk();
  weights[offsets[n] + d] = w;
  return true;
}
network::KernelTransform& network::KernelTransform::setWeights(const KernelTransform& network)
{
  assume(false, "illegal-state", "in network::KernelTransform:setWeights, this virtual method must be overridden, or it has been called with wrong type");
  return *this;
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
  json = s_printf("{\n 'sizes' : %s, 'D' : [", RecurrentTransform::asString().c_str());
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
/*
double network::KernelTransform::getValueDerivative(unsigned int n, double t, unsigned int n_, double t_) const
{
  double v = getKernelDerivative(n, 0, t, n_, t_);
  for(unsigned int d = 1; d <= getKernelDimension(n); d++)
    v += getWeight(n, d) * getKernelDerivative(n, d, t, n_, t_);
  return v;
}
*/
network::KernelTransform *network::KernelTransform::newKernelTransform(String type, unsigned int N, const network::Input& input)
{
  network::KernelTransform *network = NULL;
  if(type == "LinearTransform")
    network = new network::LinearTransform(N, input);
  else if(type == "LinearNonLinearTransform")
    network = new network::LinearNonLinearTransform(N, input);
  else if(type == "SparseNonLinearTransform")
    network = new network::SparseNonLinearTransform(N, input);
  else if(type == "IntegrateAndFireTransform")
    network = new network::IntegrateAndFireTransform(N, input);
  else if(type == "SoftMaxTransform")
    network = new network::SoftMaxTransform(N, input);
  else
    assume(false, "illegal-argument", "in network::KernelTransform::newKernelTransform bad network type %s", type.c_str());
  return network;
}
