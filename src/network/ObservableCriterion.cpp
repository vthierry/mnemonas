#include "main.hpp"
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
  if (reinject) {
    unsigned int N = transform.getN(), T = transform.getT();
    if(estimates == NULL)
      estimates = new double[N * T];
    double *lambda = new double[dimension], *b = new double [dimension], *A = new double [(dimension * (dimension + 1))/2], *x0 = new double [N * T], *x1 = new double [N * T];
    // Runs one simulation to buffer the actual values
    {
      transform.reset(true);
      for(unsigned int t = 0; t < T; t++) 
	for(int n = N - 1; 0 <= n; n--) 
	  x1[n + t * N] = x0[n + t * N] = transform.get(n, t);
    }
    estimate_N0 = 0;
    double r1 = 1e10, r0 = r1 + 1;
    static const unsigned int K = 20; 
    for(unsigned int k = 0; k < K; k++) {
      r0 = r1, r1 = 0;
      // Recomputes the projection error
      {
	for(unsigned int d = 0, dd_ = 0; d < dimension; d++) {
	  observables[d]->reset(transform);
	  for(unsigned int d_ = 0; d_ <= d; d_++, dd_++)
	    A[dd_] = 0;
	  b[d] = values[d] - observables[d]->getValue();
	  r1 += fabs(b[d]);
	  printf("\tO[%d] = %9.2g %9.2g\n", d, values[d], observables[d]->getValue());
	}
      }
      // Updates if improvement
      {
	if(r1 >= r0 - 1e-6)
	  break;
	for(unsigned int nt = 0; nt < N * T; nt++)
	  estimates[nt] = x1[nt];
      }
      // Solves the lambda linear system
      {
	for(unsigned int t = 0; t < T; t++) 
	  for(int n = N - 1; 0 <= n; n--) {
	    for(unsigned int d = 0, dd_ = 0; d < dimension; d++)
	      for(unsigned int d_ = 0; d_ <= d; d_++, dd_++)
		A[dd_] += observables[d]->getValueDerivative(n, t) * observables[d_]->getValueDerivative(n, t);
	  }
	solver::linsolve(dimension, dimension, A, true, b, lambda);
	printf(" A:\n %s b:\n %s lambda:\n %s", solver::asString(A, dimension, dimension, true).c_str(), solver::asString(b, dimension).c_str(), solver::asString(lambda, dimension).c_str());
      }
      // Updates the estimates
      {
	estimate_N0 = 0;
	for(unsigned int t = 0, nt = 0; t < T; t++) 
	  for(unsigned int n = 0; n < N; n++, nt++) {
	    for(unsigned int d = 0; d < dimension; d++) {
	      double do_nt = observables[d]->getValueDerivative(n, t);
	      if (do_nt != 0 && estimate_N0 < n) 
		estimate_N0 = n + 1;
	      x1[nt] = x0[nt] + do_nt * lambda[d];
	      transform.set(n, t, x1[nt]);
	    }
	  }
	printf("network::ObservableCriterion::update() r[%d] = %g N0=%d\n", k, r1, estimate_N0);
      }
    }
    delete[] x1;
    delete[] x0;
    delete[] A;
    delete[] b;
    delete[] lambda;
  }
}
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
