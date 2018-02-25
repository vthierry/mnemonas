#include "mnemonas.hpp"

network::Observable::Observable(String name) : name(name), value(NAN), input(NULL) {}
/// @cond INTERNAL
network::Observable::~Observable() {}
///@endcond
network::Observable& network::Observable::reset(const network::Input& input_)
{
  input = &input_;
  value = doValue();
  return *this;
}
std::string network::Observable::asString() const
{
  return s_printf("{ 'name' = '%s', 'value' = '%g' }", name.c_str(), value);
}
double network::Observable::getValue() const
{
  assume(input != NULL, "illegal-state", "in network::Observable::getValueDerivative, attempt to call this method before the observable reset()");
  return value;
}
double network::Observable::getValueDerivative(unsigned int n, double t) const
{
  assume(input != NULL, "illegal-state", "in network::Observable::getValueDerivative, attempt to call this method before the observable reset()");
  return const_cast < Observable * > (this)->doValueDerivative(n, t);
}
double network::Observable::doValue()
{
  assume(false, "illegal-state", "in network::Observable::doValue, this virtual method must be overriden");
  return NAN;
}
double network::Observable::doValueDerivative(unsigned int n, double t)
{
  assume(false, "illegal-state", "in network::Observable::doValueDerivative, this virtual method must be overriden");
  return NAN;
}
