/** Defines a discrete-time recurrent input/output kernel transform estimator. */
class KernelSupervisedEstimator: public KernelEstimator {
  Input& values;
public:
  /** Resets the estimator for a given kernel transform.
   * @param transform The kernel transform to estimate.
   * @param values The output desired values.
   * - By contract output units correspond to the values.getN() lower indexed units and hidden units to the remainder.
   */
  KernelSupervisedEstimator(KernelTransform & transform, Input & values) : KernelEstimator(transform), values(values) {}

  virtual double rho(unsigned int n, double t, double value) const
  {
    if(n < values.getN()) {
      double e = value - values.get(n, t);
      return 0.5 * e * e;
    } else
      return 0;
  }
  virtual double drho(unsigned int n, double t, double value) const
  {
    return n < values.getN() ? value - values.get(n, t) : 0;
  }
};
