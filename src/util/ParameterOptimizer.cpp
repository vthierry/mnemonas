#include "ParameterOptimizer.hpp"

#include <cmath>
#include <cfloat>
#include "s_printf.h"
#include "now.h"

/// @cond INTERNAL
ParameterOptimizer::ParameterOptimizer(String parameters_, char timing, unsigned int window) : gamma(window == 0 ? 1 : pow(0.1, 1.0 / window)), timing(timing)
{
  assume(timing == 'c' || timing == 'r' || timing == 'i', "illegal-argument", "in ParameterOptimizer::ParameterOptimizer bad timing value '%c'", timing);
  // Defines the meta-parameter space specification
  parameters.reset(parameters_);
  D = parameters.getCount();
  assume(D > 0, "illegal-argument", "in ParameterOptimizer::ParameterOptimizer the « %s » parameter specification is empty", ((String) parameters).c_str());
  meta_parameters_sampled_values = new double *[D];
  meta_parameters_sampled_counts = new unsigned int[D];
  unsigned int d = 0;
  for(std::vector < std::string > ::const_iterator i = parameters.getNames().begin(); i != parameters.getNames().end(); i++, d++) {
    Struct parameters_value = parameters[*i];
    assume(parameters_value.getLength() > 0, "illegal-argument", "in ParameterOptimizer::ParameterOptimizer the '%s' parameter has no value", (*i).c_str());
    meta_parameters_sampled_counts[d] = parameters_value.getLength();
    meta_parameters_sampled_values[d] = new double[meta_parameters_sampled_counts[d]];
    for(unsigned int k = 0; k < meta_parameters_sampled_counts[d]; k++)
      meta_parameters_sampled_values[d][k] =
        std::isnan(Struct::toDouble(parameters_value[k])) ? k : (double) parameters_value[k];
  }
  // Defines the estimation buffers
  parameter_value = new double[D];
  parameter_index = new unsigned int[D];
  m0 = new double *[D + 1], m1 = new double *[D + 1], m2 = new double *[D + 1];
  K = D;
  for(unsigned int d = 0; d < D; d++) {
    unsigned int K_d = meta_parameters_sampled_counts[d];
    m0[d] = new double[K_d + 1], m1[d] = new double[K_d + 1], m2[d] = new double[K_d + 1];
    K = K < K_d ? K_d : K;
  }
  m0[D] = new double[1], m1[D] = new double[1], m2[D] = new double[1];
  clear();
}
void ParameterOptimizer::clear()
{
  // Defines the 1st default value as a central value
  for(unsigned int d = 0; d < D; d++)
    parameter_value[d] = meta_parameters_sampled_values[d][meta_parameters_sampled_counts[d] / 2];
  // Clears the cost and time estimation
  cost0 = NAN, time0 = 0, now(timing == 'r', true);
  for(unsigned int d = 0; d <= D; d++)
    for(unsigned int k = 0; (d < D && k <= meta_parameters_sampled_counts[d]) || k < 1; k++)
      m0[d][k] = m1[d][k] = m2[d][k] = 0;
}
/// @cond INTERNAL
ParameterOptimizer::~ParameterOptimizer()
{
  for(unsigned int d = 0; d <= D; d++) {
    delete[] m0[d];
    delete[] m1[d];
    delete[] m2[d];
  }
  delete[] m0;
  delete[] m1;
  delete[] m2;
  delete[] parameter_index;
  delete[] parameter_value;
  for(unsigned int d = 0; d < D; d++)
    delete[] meta_parameters_sampled_values[d];
  delete[] meta_parameters_sampled_values;
  delete[] meta_parameters_sampled_counts;
}
///@endcond
double ParameterOptimizer::add(double c, const double *p)
{
  if(p == NULL)
    p = parameter_value;
  else
    // Calculates the closest parameter index
    for(unsigned int d = 0; d < D; d++) {
      unsigned int k_min = -1;
      double e_min = DBL_MAX;
      for(unsigned int k = 0; k < meta_parameters_sampled_counts[d]; k++) {
        double e = fabs(meta_parameters_sampled_values[d][k] - p[d]);
        if(e < e_min)
          e_min = e, k_min = k;
      }
      parameter_index[d] = k_min;
    }
  // Calculates the delta_c value
  if(std::isnan(cost0))
    cost0 = c;
  double time_c = timing == 'i' ? time0 + 1 : now(timing == 'r', true);
  assume(time_c > time0, "illegal-state", "in ParameterOptimizer::add spurious time = %g <= previous-time = %g", time_c, time0);
  double delta_c = cost0 <= 0 ? 0 : (cost0 - c) / (cost0 * (time_c - time0));
  time0 = time_c, cost0 = c;
  // Calculates the marginal statistics
  for(unsigned int d = 0; d <= D; d++)
    for(unsigned int k = 0, n = 0; n < 1 || (d < D && n < 2); n++, k = 1 + parameter_index[d]) {
      m0[d][k] = 1 + gamma * m0[d][k];
      m1[d][k] = delta_c + gamma * m1[d][k];
      m2[d][k] = delta_c * delta_c + gamma * m2[d][k];
    }
  return delta_c;
}
const double *ParameterOptimizer::get() const
{
  // Selects the parameter to adjust

  // Selects the adjusment value

  return parameter_value;
}
std::string ParameterOptimizer::asString() const
{
  std::string s = "{\n  'parameters':" + parameters.asString("plain", 1);
  for(unsigned int d = 0; d <= D; d++)
    for(unsigned int k = 0; (d < D && k < meta_parameters_sampled_counts[d]) || k < 1; k++)
      s += s_printf("%s%g+/-%g#%.0f%s",
                    k == 0 ? (d == 0 ? "  'statistics': {\n    'marginal': [\n      [ " : "      [ ") : k == 1 ? "   " : "",
                    m0[d][k] == 0 ? 0 : m1[d][k] / m0[d][k],
                    m0[d][k] <= 1 ? 0 : sqrt((m2[d][k] - pow(m1[d][k] / m0[d][k], 2)) / (m0[d][k] - 1)),
                    m0[d][k],
                    d < D && k < meta_parameters_sampled_counts[d] - 1 ? ", " : (d < D - 1 ? " ],\n" : d < D ? " ]\n  ],\n  'global':\n" : "  ]\n  }\n}\n"));
  return s;
}
/// @endcond
