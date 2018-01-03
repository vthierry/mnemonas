namespace network {
  /** Defines a discrete-time recurrent kernel transform distributed estimator.
   * - This estimator implements a fully distributed stochastic 2nd order adjusment.
   */
  class KernelDistributedEstimator: public KernelEstimator {
private:
    // Estimation algorithm variables
    double *A, *b, *w0, *w1;

    /** Algorithm step. */
    void run_once(unsigned int batch_duration, unsigned int batch_count, bool with_update);
    // Algorithmic trace
    CurveFit line_search_values, line_search_counts;
    // Wrapper to the solver::minimize routine
    double solver_minimize_f(double x);
    unsigned int n_f, D_f, c_f;
    static double solver_minimize_e_f(double x);
    static KernelDistributedEstimator *solver_minimize_e;
public:
    /** Resets the estimator for a given kernel transform.
     * @param transform The kernel transform to estimate.
     * - The recurrent transform is fully buffered fo the estimation purpose.
     * @param criterion The criterion to use for the estimation.
     */
    KernelDistributedEstimator(KernelTransform & transform, TransformCriterion & criterion);
    /// @cond INTERNAL
    virtual ~KernelDistributedEstimator();
    ///@endcond

    /** Runs the estimation.
     * @param criterion_epsilon Relative precision on the state values, with respect to the initial cost.
     * - The iteration stops if the criterion decrease is lower than this value.
     * @param maxIterations The maximal number of iteration.
     * @param batch_duration Size of the temporal duration of a batch, 0 means considering the kernel dimension <tt>D</tt>.
     * @param batch_count Number of batchs run between two backward tuning, value > 0.
     * @param with_update Whether the hidden units desired value are predicted or not before estimation.
     * @return The normalized minimal cost \f${\cal L} = \sum_{nt} \rho_n(t) / (N \, T)\f$.
     */
    double run(double criterion_epsilon = 1e-6, unsigned int maxIterations = 100, unsigned int batch_duration = 0, unsigned int batch_count = 1, bool with_update = false);

    /** Gets some fits of the estimation runs.
     * @param what :
     * - "cost" : returns the cost decay fit.
     * - "output-error" : returns the output error decay fit.
     * - "backward-error" : returns the backward error decay fit.
     * - "line-search-value" : returns the line search value (0: no improvment, 1: no adjustment).
     * - "line-search-count" : returns the line search number of interation.
     */
    const CurveFit& getFit(String what = "cost") const;
  };
}
