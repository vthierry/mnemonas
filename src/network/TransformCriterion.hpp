namespace network {
  /** Defines a transform critrion. */
  class TransformCriterion {
protected:
    RecurrentTransform& transform;
public:
    /** Resets the estimator for a given kernel transform.
     * @param transform The kernel transform to estimate.
     * - The recurrent transform is fully buffered fo the estimation purpose.
     */
    TransformCriterion(RecurrentTransform & transform);
    /// @cond INTERNAL
    virtual ~TransformCriterion();
    ///@endcond

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
     * double network::MyCriterion::rho(unsigned int n, double t) const {
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
private:
    double *estimates, *destimates;
public:
    /** Updates the best previous estimated value.
     * - This routine is called after backward tuning, and weight adjustment.
     */
    virtual void update();

    /** Gets the best previous estimated value.
     * - The method is to be overwritten if output values can be provided.
     * @param n The input unit index in <tt>{0,getN(){</tt>.
     * @param t The value time.
     * @return The best previous estimated value or NAN if undefined.
     */
    virtual double get(unsigned int n, double t) const;

    /** Gets the best previous estimated value number of units.
     * @return The number of units for which a best previous estimated value is defined.
     */
    virtual unsigned int getN0() const;
  };
}
