#include <vector>

namespace network {
  /** Defines basic mechanisms for kernel weights backward tuning estimation. */
  class KernelEstimator {
protected:
    /** The tranform to estimate. */
    KernelTransform& transform;
    /** The transform related parameters. */
    unsigned int N, T, R;
    /** The criterion to apply. */
    TransformCriterion& criterion;
    /** The cost and errors traces. */
    CurveFit costs, output_errors, backward_errors, ratio_negligible, ratio_saturated, simulation_time, tuning_time, iteration_time;
    // Estimation buffers
    double cost, *weights0, *values0, *errors;
    // Backward guard parameters
    double drho_mean;
    // Used to detect the 1st iteration
    bool once;
    // Connection buffer
    class connection {
public:
      unsigned int n, r;
      connection(unsigned int n, unsigned int r);
      bool operator == (const connection &c) const;
    };
    std::vector < connection > *connections;
protected:
    /** Number of negligible or saturated values during a backward tuning, over the N T values. */
    unsigned int k_negligible, k_saturated;
public:
    /** Resets the estimator for a given kernel transform.
     * @param transform The kernel transform to estimate.
     * - The recurrent transform is fully buffered fo the estimation purpose.
     * @param criterion The criterion to use for the estimation.
     */
    KernelEstimator(KernelTransform & transform, TransformCriterion & criterion);
    /// @cond INTERNAL
    virtual ~KernelEstimator();
    ///@endcond

    /** Runs one step of estimation.
     * - It saves previous weights and values, runs a simulation getting the cost, estimates the backward-tuning errors.
     * @return The last estimated cost.
     */
    double run();
    /** Gets the last estimated cost. */
    double getCost() const
    {
      return cost;
    }
    /** Gets the last estimated state value.
     * - If the criterion is able to estimate the best estimated value, it is returned, otherwise the last simulation value is used.
     * @param n The input unit index.
     * @param t The value time.
     */
    double getValue(unsigned int n, double t) const;

    /** Gets the last estimated backward error.
     * @param n The input unit index.
     * @param t The value time.
     */
    double getError(unsigned int n, double t) const
    {
      return errors[n + (int) t * N];
    }
    /** Gets the last estimated weights. */
    const double *getWeights() const
    {
      return weights0;
    }
private:
    unsigned int c_f, c_N0;
    double cost0, cost1, *w0, *w1;
    double solver_minimize_f(double x);
    static double solver_minimize_e_f(double x);
    static KernelEstimator *solver_minimize_e;
public:
    /** Updates the weights given a readout estimation.
     * - The readout is adjusted on the N0 desired values defined by the criterion.
     * @param N0 Defines the number of units to take into account, 0 means using <tt>criterion.getN0()</tt>.
     * @return The obtained criterion cost.
     */
    double updateReadout(unsigned int N0 = 0);

    /** Gets some fits of the estimation runs.
     * @param what :
     * - "cost" : returns the cost decay fit.
     * - "output-error" : returns the output error decay fit.
     * - "backward-error" : returns the backward error decay fit.
     * - "ratio-negligible" : returns the ratio of negligible backward errors.
     * - "ratio-saturated" : returns the ratio of saturated backward errors.
     * - "simulation-time" : returns the simulation cpu time.
     * - "tuning-time" : returns the backward tuning cpu time.
     * - "iteration-time" : returns the iteration duration cpu time.
     */
    const CurveFit& getFit(String what = "cost") const;
  };
}
