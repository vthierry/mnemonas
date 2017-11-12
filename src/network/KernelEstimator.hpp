namespace network {
/** Defines a discrete-time recurrent kernel transform estimator. */
  class KernelEstimator {
protected:
    KernelTransform& transform;
private:
    // Transform to estimate and related parameters
    unsigned int N, T, R;
    // Estimation algorithm variables
    static const unsigned int DMAX = 16;  // Maximal kernel dimension for 2nd order methods
    static const unsigned int K_GRADS = 8; // 1st order methods variants
    static const unsigned int K_SYLIN = 3; // 2nd order method variants
    double cost0, cost, upsilon, *errors, **grads, *k_grads, *n_grads, ***A, ***b, *w1;
    // Previous estimation management
protected:
    double *values0, *w0;
private:
    void save_previous_estimation();
    // Algorithm steps
    void backward_tuning();
    void run_init(double weights_epsilon);
    bool run_once(double weights_epsilon);
    // Wrapper to the solver::minimize routine
    double set_second_order_weights(double apsilon);
    static double solver_minimize_f(double x);
    static KernelEstimator *solver_minimize_e;
    unsigned int solver_minimize_k;
    // Algorithm behavior observation
    ExponentialDecayFit dumpCosts;
    std::string dumpName, dumpString;
public:
    /** Resets the estimator for a given kernel transform.
     * @param transform The kernel transform to estimate.
     * - The recurrent transform is fully buffered fo the estimation purpose.
     */
    KernelEstimator(KernelTransform & transform);
    /// @cond INTERNAL
    virtual ~KernelEstimator();
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

    /** Gets the cost decay fit of the last estimation run. */
    const ExponentialDecayFit& getFit() const
    {
      return dumpCosts;
    }
    /** Defines the estimator criterion.
     * - The method is to be overwritten to implement a global estimator.
     * - The default implementation simply returns \f$\frac{\sum_{nt} \rho_{nt}(x_n(t))}{N\,T}\f$, running:<pre>
     * double r = 0;
     * transform.reset(true);
     * for(unsigned int t = 0; t < transform.getT(); t++)
     *   for(int n =  transform.getN() - 1; 0 <= n; n--)
     *      r += rho(n, t, transform.get(n, t));
     * return r / (transform.getN() *  transform.getT());</pre>
     * @return The criterion value to mnimize.
     */
    virtual double rho() const;

    /** Defines the estimator criterion.
     * - The method is to be overwritten to implement a local estimator.
     * - A typical implementation is of the form:<pre>
     * double network::KernelSupervisedEstimator::rho(unsigned int n, double t) const {
     *  double value = transform.get(n, t);
     *  ../..</pre>
     * @param n The input unit index.
     * @param t The value time.
     * @return The related positive value to minimize.
     */
    virtual double rho(unsigned int n, double t) const;

    /** Defines the estimator criterion derivative.
     * - The method is to be overwritten to implement a given estimator.
     * @param n The input unit index.
     * @param t The value time.
     * @return The value derivative \f$\frac{\partial \rho}{\partial x_n(t)}({\bf x})\f$.
     */
    virtual double drho(unsigned int n, double t) const;

    /** Updates the best previous estimated value.
     * - This routine is called after a forward simulation and a backward tuning, and before 1st and 2nd order weight adjustment.
     */
    virtual void update() {}

    /** Gets the best previous estimated value.
     * - The method is to be overwritten if output values can be provided.
     * @param n The input unit index.
     * @param t The value time.
     */
    virtual double get(unsigned int n, double t) const;
  };
}
