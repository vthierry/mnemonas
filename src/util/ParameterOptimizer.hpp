#ifndef PARAMETEROPTIMIZER_HPP
#define PARAMETEROPTIMIZER_HPP

#include "util/s_string.h"
#include "util/numeric.hpp"
#include "util/Struct.hpp"

/// @cond INTERNAL

/** Defines a meta-parameter optimizer of a given parameter space.
 * ### Problem position
 * Given the mimization of a criterion \f$c({\bf x})\f$ by a minimization algorithm with meta-parameters \f${\bf p}\f$,
 * this mechanism attemps to observe the minimization process in order to make proposals about the next meta-parameters.
 *
 * ### Meta-parameter space specification
 * The meta-parameter space is defined by a multi-dimensional list of sampled values, e.g.,
 * if there are one boolean parameter <tt>mode</tt> and one logarithmic numeric parameter <tt>scale</tt> we may write:<pre>
 * {
 *   'mode'  : ['true', 'false'],
 *   'scale' : [0.1, 0.2, 0.5, 1, 2, 5, 10]
 * }</pre>making explicit the potential values.
 *  - This specification is input as a JSON string.
 *  - Non numeric values are converted to their index in the value list, e.g. <tt>['true', 'undefined', 'false']</tt> is converted to <tt>[0, 1, 2]</tt>.
 *  - The mechanism uses both the continuous and discretized nature of the meta-parameter set.
 *
 * ### Using this mechanism
 * This mechanism is typicaly used in a construct of the form:<pre>
 * ParameterOptimizer parameter_optimizer(parameter_specification);
 * ../..
 * do {
 *   double* meta_parameter_values = parameter_optimizer.get();
 *   optimize_once(meta_parameter_values[0], meta_parameter_values[1], ../..
 *   parameter_optimizer.add(obtained_criterion_value());
 * } while(no_convergence);</pre>
 *
 * ### Mechanism implementation
 * - The following meta-criterion, for a cost \f$c(t)\f$ at time \f$T(t)\f$, is considered:
 * <center>\f$\delta_c(t) = \frac{c(t) - c(t-1)}{c(t-1)} \, \frac{1}{T(t) - T(t-1)}\f$</center>
 * thus the relative cost decrease speed, which reduces to relative cost decrease if only the iteration index is taken into account.
 * - Marginal Gaussian processes are considered to randomly choose the next parameter.
 */
class ParameterOptimizer: public numeric {
private:
  // Meta-parameter space specification
  Struct parameters;
  double **meta_parameters_sampled_values;
  unsigned int *meta_parameters_sampled_counts, D;
  double gamma;
  char timing;
  // Previous parameter value
  double *parameter_value, time0, cost0;
  unsigned int *parameter_index;

  // Meta-parameter values marginal statistics
  unsigned int K;
  double **m0, **m1, **m2;

  // No copy constructor
  ParameterOptimizer(const ParameterOptimizer &fit);
public:
  /** Constructs a meta-parameter optimizer for the given parameter space.
   * @param parameters The parameter space specification, see above.
   * @param timing Defines if : 'c' The cpu time, or 'r' The real time, or 'i' The iteration index is to be taken into account.
   * @param window The criterion sampling window containing 90% of the least-square average.
   */
  ParameterOptimizer(String parameters, char timing = 'i', unsigned int window = 10);
  /// @cond INTERNAL
  ~ParameterOptimizer();
  /// @endcond

  /** Clears the estimation. */
  void clear();

  /** Adds an observed value.
   * @param c The criterion observed value.
   * @param values The related meta-parameter value, by default the last meta-parameter value proposal obtained by get().
   * @return The relative cost decrease \f$\delta_c\f$.
   */
  double add(double c, const double *values = NULL);

  /** Gets the next meta-parameter value proposal. */
  const double *get() const;

  /** Returns the parameters estimation as a JSON string. */
  std::string asString() const;
};
/// @endcond

#endif
