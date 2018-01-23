#include "mnemonas.hpp"
#include <stdarg.h>

network::ObservableCriterion::Observable::Observable(String name) : name(name), value(NAN), input(NULL) {}
/// @cond INTERNAL
network::ObservableCriterion::Observable::~Observable() {}
///@endcond
network::ObservableCriterion::Observable& network::ObservableCriterion::Observable::reset(const network::Input& input_)
{
  input = &input_;
  value = doValue();
  return *this;
}
std::string network::ObservableCriterion::Observable::asString() const
{
  return s_printf("{ 'name' = '%s', 'value' = '%g' }", name.c_str(), value);
}
double network::ObservableCriterion::Observable::getValue() const
{
  assume(input != NULL, "illegal-state", "in network::ObservableCriterion::Observable::getValueDerivative, attempt to call this method before the observable reset()");
  return value;
}
double network::ObservableCriterion::Observable::getValueDerivative(unsigned int n, double t) const
{
  assume(input != NULL, "illegal-state", "in network::ObservableCriterion::Observable::getValueDerivative, attempt to call this method before the observable reset()");
  return const_cast < Observable * > (this)->doValueDerivative(n, t);
}
double network::ObservableCriterion::Observable::doValue()
{
  assume(false, "illegal-state", "in network::ObservableCriterion::Observable::doValue, this virtual method must be overriden");
  return NAN;
}
double network::ObservableCriterion::Observable::doValueDerivative(unsigned int n, double t)
{
  assume(false, "illegal-state", "in network::ObservableCriterion::Observable::doValueDerivative, this virtual method must be overriden");
  return NAN;
}
network::ObservableCriterion::ObservableCriterion(KernelTransform& transform, std::vector < Observable * > observables_, const double *values_, const double *lambdas_, bool reinject) : network::TransformCriterion(transform), reinject(reinject), estimate_N0(0), estimates(NULL)
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
network::ObservableCriterion::ObservableCriterion(KernelTransform& transform, const Input& input, std::vector < Observable * > observables_, const double *lambdas_, bool reinject) : ObservableCriterion(transform, observables_, NULL, lambdas_, reinject)
{
  for(unsigned int k = 0; k < dimension; k++)
    values[k] = observables[k]->reset(input).getValue();
}
/// @cond INTERNAL
network::ObservableCriterion::~ObservableCriterion()
{
  delete[] observables;
  delete[] lambdas;
  delete[] values;
  delete[] estimates;
}
///@endcond
double network::ObservableCriterion::rho() const
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
double network::ObservableCriterion::drho(unsigned int n, double t) const
{
  double v = 0;
  for(unsigned int k = 0; k < dimension; k++) {
    double e = observables[k]->getValue() - values[k];
    v += lambdas[k] * (e == 0 ? 0 : e > 0 ? 1 : -1) * observables[k]->getValueDerivative(n, t);
  }
  return v;
}
double network::ObservableCriterion::getObservableExpectedValue(unsigned int k) const
{
  assume(k < dimension, "illegal-argument", "in network::ObservableCriterion::getObservableExpectedValue the index k=%d must be in {0, %d{", k, dimension);
  return values[k];
}
double network::ObservableCriterion::get(unsigned int n, double t) const
{
  return estimates == NULL ? NAN : estimates[n + transform.getN() * (int) t];
}
unsigned int network::ObservableCriterion::getN0() const
{
  return estimates == NULL ? 0 : estimate_N0;
}
void network::ObservableCriterion::update()
{
  if(reinject) {
    unsigned int N = transform.getN(), T = transform.getT();
    if(estimates == NULL)
      estimates = new double[N * T];
    // Runs one simulation to buffer the actual values
    {
      transform.reset(true);
      for(unsigned int t = 0; t < T; t++)
        for(int n = N - 1; 0 <= n; n--)
          estimates[n + t * N] = transform.get(n, t);
    }
    // Defines a projector onto the observable values
    {
      solver_project_this = this;
      solver::projsolve(dimension, N * T, solver_project_this_c, solver_project_this_d, estimates, estimates);
    }
    // Estimates N0
    {
      estimate_N0 = 0;
      for(unsigned int t = 0; t < T; t++)
        for(unsigned int n = 0; n < N; n++)
          for(unsigned int d = 0; d < dimension; d++)
            if((observables[d]->getValueDerivative(n, t) != 0) && (estimate_N0 <= n))
              estimate_N0 = n + 1;
    }
    // Updates hidden values
    TransformCriterion::update();
  }
}
double network::ObservableCriterion::solver_project_c(const double *x, unsigned int d)
{
  unsigned int N = transform.getN(), T = transform.getT();
  if(d == 0)
    for(unsigned int t = 0, nt = 0; t < T; t++)
      for(unsigned int n = 0; n < N; n++, nt++)
        transform.set(n, t, x[nt]);
  observables[d]->reset(transform);
  return observables[d]->getValue() - values[d];
}
double network::ObservableCriterion::solver_project_d(const double *x, unsigned int d, unsigned int nt)
{
  unsigned int N = transform.getN();
  return observables[d]->getValueDerivative(nt % N, nt / N);
}
double network::ObservableCriterion::solver_project_this_c(const double *x, unsigned int d)
{
  return solver_project_this->solver_project_c(x, d);
}
double network::ObservableCriterion::solver_project_this_d(const double *x, unsigned int d, unsigned int nt)
{
  return solver_project_this->solver_project_d(x, d, nt);
}
network::ObservableCriterion *network::ObservableCriterion::solver_project_this = NULL;
network::ObservableCriterion::Observable *network::ObservableCriterion::getObservable(String name, ...)
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
        assume(n0 < input->getN(), "illegal-argument", "network::ObservableCriterion::MeanObservable the unit index n0=%d must be in {0, %d{", n0, input->getN());
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
        assume(n0 < input->getN(), "illegal-argument", "network::ObservableCriterion::IcorrObservable the unit index n0=%d must be in {0, %d{", n0, input->getN());
        assume(m0 < input->getN(), "illegal-argument", "network::ObservableCriterion::IcorrObservable the unit index m0=%d must be in {0, %d{", m0, input->getN());
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
        assume(n0 < input->getN(), "illegal-argument", "network::ObservableCriterion::AcorrObservable the unit index n0=%d must be in {0, %d{", n0, input->getN());
        assume(tau < input->getT(), "illegal-argument", "network::ObservableCriterion::AcorrObservable the time shift tau=%d must be in {0, %d{", tau, input->getT());
        assume(tau <= 16, "illegal-argument", "network::ObservableCriterion::AcorrObservable the time shift tau=%d is numerically unrealistic, must be in {0, 16}", tau);
        count = sum = 0;
        for(int t = tau; t < input->getT(); t++)
          count++, sum += input->get(n0, t) * input->get(n0, t - tau);
        return count == 0 ? 0 : sum / (count - tau);
      }
      double doValueDerivative(unsigned int n, double t)
      {
        return count == 0 ? 0 : n == n0 ? ((tau <= t ? input->get(n0, t - tau) : 0) + (t + tau < input->getT() ? input->get(n0, t + tau) : 0)) / (count - tau) : 0;
      }
    };
    return new AcorrObservable(n0, tau);
  } else {
    va_end(a);
    assume(false, "illegal-argument", "in network::ObservableCriterion::getObservable undefined predefined observable name '%s", name.c_str());
    Observable o("none");
    return NULL;
  }
}
std::vector < network::ObservableCriterion::Observable * > network::ObservableCriterion::getObservables(String name, ...) {
  va_list a;
  va_start(a, name);
  if(name == "mean") {
    unsigned int N = va_arg(a, unsigned int);
    assume(0 < N && N <= 32, "illegal-argument", "network::ObservableCriterion::getObservables the number of unit N=%d is numerically unrealistic, must be in {1, 32}", N);
    va_end(a);
    std::vector < Observable * > observables;
    for(unsigned int n = 0; n < N; n++)
      observables.push_back(getObservable("mean", n));
    return observables;
  } else if(name == "icorr") {
    unsigned int N = va_arg(a, unsigned int);
    assume(0 < N && N <= 8, "illegal-argument", "network::ObservableCriterion::getObservables the number of unit N=%d is numerically unrealistic, must be in {1, 8}", N);
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
    assume(0 < N && N * tau <= 32, "illegal-argument", "network::ObservableCriterion::getObservables the number of unit N=%d and/or tau=%d is numerically unrealistic, must tau * N = %d be in {0, 32}, while 0 < N", N, tau);
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
    assume(false, "illegal-argument", "in network::ObservableCriterion::getObservables undefined predefined observable name '%s", name.c_str());
    std::vector < Observable * > observables;
    return observables;
  }
}
void network::ObservableCriterion::deleteObservables(std::vector < network::ObservableCriterion::Observable * >& observables)
{
  for(std::vector < Observable * > ::const_iterator i = observables.begin(); i != observables.end(); ++i)
    delete *i;
}
