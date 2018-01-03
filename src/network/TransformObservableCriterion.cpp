#include "main.hpp"
#include <stdarg.h>

network::TransformObservableCriterion::Observable::Observable(String name) : name(name), value(NAN), input(NULL) {}
/// @cond INTERNAL
network::TransformObservableCriterion::Observable::~Observable() {}
///@endcond
network::TransformObservableCriterion::Observable& network::TransformObservableCriterion::Observable::reset(const network::Input& input_)
{
  input = &input_;
  value = doValue();
  return *this;
}
std::string network::TransformObservableCriterion::Observable::asString() const
{
  return s_printf("{ 'name' = '%s', 'value' = '%g' }", name.c_str(), value);
}
double network::TransformObservableCriterion::Observable::getValue() const
{
  assume(input != NULL, "illegal-state", "in network::TransformObservableCriterion::Observable::getValueDerivative, attempt to call this method before the observable reset()");
  return value;
}
double network::TransformObservableCriterion::Observable::getValueDerivative(unsigned int n, double t) const
{
  assume(input != NULL, "illegal-state", "in network::TransformObservableCriterion::Observable::getValueDerivative, attempt to call this method before the observable reset()");
  return const_cast < Observable * > (this)->doValueDerivative(n, t);
}
double network::TransformObservableCriterion::Observable::doValue()
{
  assume(false, "illegal-state", "in network::TransformObservableCriterion::Observable::doValue, this virtual method must be overriden");
  return NAN;
}
double network::TransformObservableCriterion::Observable::doValueDerivative(unsigned int n, double t)
{
  assume(false, "illegal-state", "in network::TransformObservableCriterion::Observable::doValueDerivative, this virtual method must be overriden");
  return NAN;
}
network::TransformObservableCriterion::TransformObservableCriterion(KernelTransform& transform, std::vector < Observable * > observables_, const double *values_, const double *lambdas_) : network::TransformCriterion(transform)
{
  // Stores the observables as an array
  {
    dimension = observables_.size();
    observables = new Observable *[dimension];
    for(unsigned int k = 0; k < dimension; k++)
      observables[k] = observables_[k];
  }
  // Stores lambdas or default values
  {
    double u = transform.getT() > 0 ? 1.0 / transform.getT() : 1;
    lambdas = new double[dimension];
    for(unsigned int k = 0; k < dimension; k++)
      lambdas[k] = lambdas_ == NULL ? u : fabs(lambdas_[k]);
  }
  // Defines the desired observable expectations
  {
    values = new double[dimension];
    for(unsigned int k = 0; k < dimension; k++)
      values[k] = values_ == NULL ? 0 : values_[k];
  }
}
network::TransformObservableCriterion::TransformObservableCriterion(KernelTransform& transform, const Input& input, std::vector < Observable * > observables_, const double *lambdas_) : TransformObservableCriterion(transform, observables_, NULL, lambdas_)
{
  for(unsigned int k = 0; k < dimension; k++)
    values[k] = observables[k]->reset(input).getValue();
}
/// @cond INTERNAL
network::TransformObservableCriterion::~TransformObservableCriterion()
{
  delete[] observables;
  delete[] lambdas;
  delete[] values;
}
///@endcond
double network::TransformObservableCriterion::rho() const
{
  transform.reset(true);
  // -printf("W : %s\n", transform.asString().c_str());
  for(unsigned int t = 0; t < transform.getT(); t++)
    for(int n = transform.getN() - 1; 0 <= n; n--)
      transform.get(n, t);
  double v = 0;
  for(unsigned int k = 0; k < dimension; k++)
    observables[k]->reset(transform);
  for(unsigned int k = 0; k < dimension; k++) {
    double e = observables[k]->getValue() - values[k];
    v += lambdas[k] * fabs(e);
    // - printf("%s|~o = %g - _o = %g|%s", k == 0 ? "rho() = " : " + ", observables[k]->getValue(), values[k], k < dimension - 1 ? "" : s_printf(" = %g\n", v).c_str());
  }
  return v;
}
double network::TransformObservableCriterion::drho(unsigned int n, double t) const
{
  double v = 0;
  for(unsigned int k = 0; k < dimension; k++) {
    double e = observables[k]->getValue() - values[k];
    v += lambdas[k] * (e == 0 ? 0 : e > 0 ? 1 : -1) * observables[k]->getValueDerivative(n, t);
  }
  return v;
}
double network::TransformObservableCriterion::getObservableExpectedValue(unsigned int k) const
{
  assume(k < dimension, "illegal-argument", "in network::TransformObservableCriterion::getObservableExpectedValue the index k=%d must be in {0, %d{", k, dimension);
  return values[k];
}
network::TransformObservableCriterion::Observable *network::TransformObservableCriterion::getObservable(String name, ...)
{
  va_list a;
  va_start(a, name);
  if(name == "mean") {
    unsigned int n0 = va_arg(a, unsigned int);
    va_end(a);
    class MeanObservable: public Observable {
      unsigned int n0;
      double count, sum;
public:
      MeanObservable(unsigned int n0) : Observable(s_printf("MeanObservable(%d)", n0)), n0(n0), count(0), sum(0) {}
protected:
      double doValue()
      {
        assume(n0 < input->getN(), "illegal-argument", "network::TransformObservableCriterion::MeanObservable the unit index n0=%d must be in {0, %d{", n0, input->getN());
        count = sum = 0;
        for(unsigned int t = 0; t < input->getT(); t++)
          count++, sum += input->get(n0, t);
        return count == 0 ? 0 : sum / count;
      }
      double doValueDerivative(unsigned int n, double t)
      {
        return count == 0 ? 0 : n == n0 ? 1.0 / count : 0;
      }
    };
    return new MeanObservable(n0);
  } else if(name == "icorr") {
    unsigned int n0 = va_arg(a, unsigned int);
    unsigned int m0 = va_arg(a, unsigned int);
    va_end(a);
    class IcorrObservable: public Observable {
      unsigned int n0, m0;
      double count, sum;
public:
      IcorrObservable(unsigned int n0, unsigned int m0) : Observable(s_printf("IcorrObservable(%d, %d)", n0, m0)), n0(n0), m0(m0), count(0), sum(0) {}
protected:
      double doValue()
      {
        assume(n0 < input->getN(), "illegal-argument", "network::TransformObservableCriterion::IcorrObservable the unit index n0=%d must be in {0, %d{", n0, input->getN());
        assume(m0 < input->getN(), "illegal-argument", "network::TransformObservableCriterion::IcorrObservable the unit index m0=%d must be in {0, %d{", m0, input->getN());
        count = sum = 0;
        for(unsigned int t = 0; t < input->getT(); t++)
          count++, sum += input->get(n0, t) * input->get(m0, t);
        return count == 0 ? 0 : sum / count;
      }
      double doValueDerivative(unsigned int n, double t)
      {
        return count == 0 ? 0 : ((n == n0 ? input->get(m0, t) : 0) + (n == m0 ? input->get(n0, t) : 0)) / count;
      }
    };
    return new IcorrObservable(n0, m0);
  } else if(name == "acorr") {
    unsigned int n0 = va_arg(a, unsigned int);
    unsigned int tau = va_arg(a, unsigned int);
    va_end(a);
    class AcorrObservable: public Observable {
      unsigned int n0;
      unsigned int tau;
      double count, sum;
public:
      AcorrObservable(unsigned int n0, unsigned int tau) : Observable(s_printf("AcorrObservable(%d, tau=%d)", n0, tau)), n0(n0), tau(tau), count(0), sum(0) {}
protected:
      double doValue()
      {
        assume(n0 < input->getN(), "illegal-argument", "network::TransformObservableCriterion::AcorrObservable the unit index n0=%d must be in {0, %d{", n0, input->getN());
        assume(tau < input->getT(), "illegal-argument", "network::TransformObservableCriterion::AcorrObservable the time shift tau=%d must be in {0, %d{", tau, input->getT());
        assume(tau <= 16, "illegal-argument", "network::TransformObservableCriterion::AcorrObservable the time shift tau=%d is numerically unrealistic, must be in {0, 16}", tau);
        count = sum = 0;
        for(int t = tau; t < input->getT(); t++)
          count++, sum += input->get(n0, t) * input->get(n0, t - tau);
        return count == 0 ? 0 : sum / count;
      }
      double doValueDerivative(unsigned int n, double t)
      {
        return count == 0 ? 0 : n == n0 ? ((tau <= t ? input->get(n0, t - tau) : 0) + (t + tau < input->getT() ? input->get(n0, t + tau) : 0)) / count : 0;
      }
    };
    return new AcorrObservable(n0, tau);
  } else {
    va_end(a);
    assume(false, "illegal-argument", "in network::TransformObservableCriterion::getObservable undefined predefined observable name '%s", name.c_str());
    Observable o("none");
    return NULL;
  }
}
std::vector < network::TransformObservableCriterion::Observable * > network::TransformObservableCriterion::getObservables(String name, ...) {
  va_list a;
  va_start(a, name);
  if(name == "mean") {
    unsigned int N = va_arg(a, unsigned int);
    assume(0 < N && N <= 32, "illegal-argument", "network::TransformObservableCriterion::getObservables the number of unit N=%d is numerically unrealistic, must be in {1, 32}", N);
    va_end(a);
    std::vector < Observable * > observables;
    for(unsigned int n = 0; n < N; n++)
      observables.push_back(getObservable("mean", n));
    return observables;
  } else if(name == "icorr") {
    unsigned int N = va_arg(a, unsigned int);
    assume(0 < N && N <= 8, "illegal-argument", "network::TransformObservableCriterion::getObservables the number of unit N=%d is numerically unrealistic, must be in {1, 8}", N);
    va_end(a);
    std::vector < Observable * > observables;
    for(unsigned int n = 0; n < N; n++)
      observables.push_back(getObservable("mean", n));
    for(unsigned int n = 0; n < N; n++)
      for(unsigned int m = n; m < N; m++)
        observables.push_back(getObservable("icorr", n, m));
    return observables;
  } else if(name == "acorr") {
    unsigned int N = va_arg(a, unsigned int);
    unsigned int tau = va_arg(a, unsigned int);
    assume(0 < N && N * tau <= 32, "illegal-argument", "network::TransformObservableCriterion::getObservables the number of unit N=%d and/or tau=%d is numerically unrealistic, must tau * N = %d be in {0, 32}, while 0 < N", N, tau);
    va_end(a);
    std::vector < Observable * > observables;
    for(unsigned int n = 0; n < N; n++)
      observables.push_back(getObservable("mean", n));
    for(unsigned int t = 0; t <= tau; t++)
      for(unsigned int n = 0; n < N; n++)
        observables.push_back(getObservable("acorr", n, t));
    return observables;
  } else {
    va_end(a);
    assume(false, "illegal-argument", "in network::TransformObservableCriterion::getObservables undefined predefined observable name '%s", name.c_str());
    std::vector < Observable * > observables;
    return observables;
  }
}
void network::TransformObservableCriterion::deleteObservables(std::vector < network::TransformObservableCriterion::Observable * >& observables)
{
  for(std::vector < Observable * > ::const_iterator i = observables.begin(); i != observables.end(); ++i)
    delete *i;
}
