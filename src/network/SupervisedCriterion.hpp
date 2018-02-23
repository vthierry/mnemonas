namespace network {
  /** Defines a discrete-time recurrent input/output suprvised transform criterion. */
  class SupervisedCriterion: public TransformCriterion {
    Input& values;
    int criterion;
    double nu;
    char reinject;
public:
    /** Resets the criterion for a given transform.
     * @param transform The transform to estimate.
     * @param values The output desired values.
     * - By contract output units correspond to the values.N lower indexed units and hidden units to the remainder indexes.
     * @param criterion The criterion index:
     * - <tt>2</tt> The L2 criterion \f$\rho_{nt}(x_n(t)) = (x_n(t)-\bar{o}_n(t))^2/2\f$.
     * - <tt>1</tt> The reweighted L1 criterion \f$\rho_{nt}(x_n(t)) = (x_n(t)-\bar{o}_n(t))^2/(2 \, (\nu + |\hat{x}_n(t)-\bar{o}_n(t)|))\f$.
     * - <tt>0</tt> The reweighted L0 criterion \f$\rho_{nt}(x_n(t)) = (x_n(t)-\bar{o}_n(t))^2/(2 \, (\nu + (\hat{x}_n(t)-\bar{o}_n(t))^2))\f$.
     * - <tt>a</tt> A L1 related unbounded criterion \f$\rho_{nt}(x_n(t)) = \nu \, \rho((x_n(t)-\bar{o}_n(t))/\nu), \rho(x) \stackrel{\rm def}{=} \frac{x^2}{2} \, H(1 - |x|) + \left(|x| - \frac{1}{2}\right) \, H(|x| - 1)\f$.
     *    - If \f$\nu == 0\f$ simply considers the absolute value of the error.
     * - <tt>b</tt> A L0 related biweight bounded criterion \f$\rho_{nt}(x_n(t)) = \rho((x_n(t)-\bar{o}_n(t))/\nu), \rho(x) \stackrel{\rm def}{=} (x^6 - 3 \, x^4 + 3 \, x^2) \, H(1 - |x|) + H(|x| - 1)\f$.
     *    - If \f$\nu == 0\f$ simply counts the number of non-zero values.
     * - <tt>h</tt> The L2 boolean criterion \f$\rho_{nt}(x_n(t)) =  \rho(\nu - sg(\bar{o}_n(t) - 1/2) \, (x_n(t) - 1/2)), \;\;\; \rho(x) \stackrel{\rm def}{=} H(x) \, x^2/2\f$.
     *    - If \f$\nu == 0\f$ simply counts the number of wrong boolean values (not used as criterion but a posteriori precision).
     * @param nu The criterion margin parameter.
     * @param reinject Defined how desired values are defined:
     * - 'n' : output values are not reinjected as desired values.
     * - 'o : output values are reinjected as desired values, other values are not used.
     * - 'b' : output values are reinjected and backpropagated on hidden values in order to match the output values.
     */
    SupervisedCriterion(KernelTransform & transform, Input & values, char criterion = '2', double nu = 1e-3, char reinject = 'b');
    /// @cond INTERNAL
    virtual ~SupervisedCriterion() {}
    ///@endcond
    virtual double rho(unsigned int n, double t) const;
    virtual double drho(unsigned int n, double t) const;
    virtual double get(unsigned int n, double t) const;
    virtual unsigned int getN0() const;
    virtual void update();
  };
}
