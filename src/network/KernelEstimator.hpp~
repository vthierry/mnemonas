/** Defines a discrete-time recurrent kernel transform estimator. */
class KernelEstimator {
  // Transform to estimate and related parameters
  KernelTransform& transform;
  unsigned int N, T, R;
  // Estimation algorithm variables
  double cost, upsilon, epsilon, *errors, *kappas, *betas_max, *grad, n_grad, **A, **b, *w0, *w1;
  double forward_simulation();
  double backward_sigmoid(double u, double b) const;
  void backward_tuning();
  unsigned int run_once(double weights_epsilon);
  // Wrapper to the solver::minimize routine
  double set_second_order_weights(double apsilon);
  static double solver_minimize_f(double x);
  static KernelEstimator *solver_minimize_e;
  // Algorithm behavior observation
  ExponentialDecayFit dumpCosts;
  std::string dumpName, dumpString;
public:
  /** Resets the estimator for a given kernel transform.
   * @param transform The kernel transform to estimate.
   * - The recurrent transform is fully buffered fo the estimation purpose.
   */
  KernelEstimator(KernelTransform & transform);
  virtual ~KernelEstimator();

  /** Runs the estimation.
   * @param criterion_epsilon Precision on the state values.
   * @param weights_epsilon Precision on the weight values.
   * @param maxIterations The maximal number of iteration.
   * @param dumpDirectory A string with the directory name of the estimation dump.
   * - If "stdout" printf on stdout.
   * - An empty string means no dump.
   * @param dumpHeader A JSON string with a description of estimation run parameters.
   * @return The normalized minimal cost \f${\cal L} = \sum_{nt} \rho_n(t) / (N \, T)\f$.
   */
  double run(double criterion_epsilon = 1e-6, double weights_epsilon = 1e-4, unsigned int maxIterations = 100, String dumpDirectory = "", String dumpHeader = "");

  /** Gets the cost decay fit of the last estimation run. */
  const ExponentialDecayFit& getFit() const
  {
    return dumpCosts;
  }
  /** Defines the estimator criterion.
   * @param n The input unit index.
   * @param t The value time.
   * @param value The current output value.
   * @return The related positive value to minimize
   */
  virtual double rho(unsigned int n, double t, double value) const
  {
    assume(false, "illegal-state", "in network::KernelEstimator::rho, this virtual method must be overloaded");
    return NAN;
  }
  /** Defines the estimator criterion derivative.
   * @param n The input unit index.
   * @param t The value time.
   * @param value The current output value.
   * @return The related positive value to minimize
   */
  virtual double drho(unsigned int n, double t, double value) const
  {
    assume(false, "illegal-state", "in network::KernelEstimator::rho, this virtual method must be overloaded");
    return NAN;
  }
};
