namespace network {
  /** Defines a discrete-time recurrent kernel transform experimental estimator.
   * - This estimator experiments several possible criterion decrement as discussed in the corresponding <a href="https://hal.inria.fr/hal-01610735v1">publication</a>.
   */
  class KernelExperimentalEstimator: public KernelEstimator {
private:
    // Estimation algorithm variables
    static const unsigned int DMAX = 16;  // Maximal kernel dimension for 2nd order methods
    static const unsigned int K_GRADS = 8; // 1st order methods variants
    static const unsigned int K_SYLIN = 3; // 2nd order method variants
    double cost, cost0, upsilon, **grads, *k_grads, *n_grads, ***A, ***b, *w1;
private:
    // Algorithm steps
    void backward_tuning();
    void run_init(double weights_epsilon);
    bool run_once(double weights_epsilon);
    // Wrapper to the solver::minimize routine
    double set_second_order_weights(double apsilon);
    static double solver_minimize_f(double x);
    static KernelExperimentalEstimator *solver_minimize_e;
    unsigned int solver_minimize_k;
    // Algorithm behavior observation
    std::string dumpName, dumpString;
public:
    /** Resets the estimator for a given kernel transform.
     * @param transform The kernel transform to estimate.
     * - The recurrent transform is fully buffered fo the estimation purpose.
     * @param criterion The criterion to use for the estimation.
     */
    KernelExperimentalEstimator(KernelTransform & transform, TransformCriterion & criterion);
    /// @cond INTERNAL
    virtual ~KernelExperimentalEstimator();
    ///@endcond

    /** Runs the estimation.
     * @param criterion_epsilon Relative precision on the state values, with respect to the initial cost.
     * - The iteration stops if the criterion decrease is lower than this value.
     * @param weights_epsilon Precision on the weight values.
     * @param maxIterations The maximal number of iteration.
     * @param dumpDirectory A string with the directory name of the estimation dump.
     * - If "stdout" printf on stdout.
     * - An empty string means no dump.
     * @param dumpHeader A JSON string with a description of estimation run parameters.
     * @return The normalized minimal cost \f${\cal L} = \sum_{nt} \rho_n(t) / (N \, T)\f$.
     */
    double run(double criterion_epsilon = 1e-6, double weights_epsilon = 1e-4, unsigned int maxIterations = 100, String dumpDirectory = "", String dumpHeader = "{}");
  };
}
