namespace network {
/** Defines a discrete-time recurrent input/output kernel transform estimator. */
  class ObservableCriterion: public TransformCriterion {
public:
    /** Defines an observable over an input.
     * - By contract:
     *   - Observable must implement an implicit or explicit copy constructor.
     *   - The doValue() and doValueDerivative() methods implements a particular observable.
     */
    class Observable {
      std::string name;
      double value;
protected:
      /** This is used to implement the getValueDerivative(). */
      const Input *input;
public:
      /** Constructs an observable of the given name. */
      Observable(String name);
      /// @cond INTERNAL
      virtual ~Observable();
      ///@endcond

      /** Gets the observable name and value. */
      std::string asString() const;

      /** Resets the observable for a given input.
       * - This reset() method must be called prior to the getValue() or getValueDerivative() method.
       * @param input The input to observe in the input temporal range.
       * @return this;
       */
      Observable& reset(const Input& input);

      /** Gets the observable value expectation, given the measures. */
      double getValue() const;

      /** Gets the local observable derivative.
       * @param n The unit index.
       * @param t The time index.
       */
      double getValueDerivative(unsigned int n, double t) const;
protected:
      /** Calculates and returns the observable value expectation, given the measures.
       * - Typical implementation writes :<pre>  reset_the_calculation();
       *  for(unsigned int t = 0; t < input->getT(); t++)
       *    add_new_values(t);
       *  return finalize_the_calculation(); </pre>
       */
      virtual double doValue();

      /** Calculates and returns the observable derivative.
       * - Typical implementation uses <tt>input->get(n, t)</tt> to calculate the local observable derivative.
       * @param n The unit index.
       * @param t The time index.
       */
      virtual double doValueDerivative(unsigned int n, double t);
    };
private:
    unsigned int dimension;
    Observable **observables;
    double *lambdas, *values;
    bool reinject;
    unsigned int estimate_N0;
    double *estimates;
    double solver_project_c(const double *x, unsigned int m);
    double solver_project_d(const double *x, unsigned int m, unsigned int n);
    static double solver_project_this_c(const double *x, unsigned int m);
    static double solver_project_this_d(const double *x, unsigned int m, unsigned int n);
    static ObservableCriterion *solver_project_this;
public:
    /** Resets the criterion for a given statistics.
     * @param transform The kernel transform to estimate.
     * @param input The input to consider as a reference.
     * @param observables The observables to take into account. They must be deleted after the ObservableCriterion yuse.
     * @param lambdas The observables weighted parameters, if any.
     * @param reinject If true defined desired values constrained by the observable.
     */
    ObservableCriterion(KernelTransform & transform, const Input &input, std::vector < Observable * > observables, const double *lambdas = NULL, bool reinject = false);

    /** Resets the estimator for a given statistics.
     * @param transform The kernel transform to estimate.
     * @param observables The observables to take into account. They must be deleted after the ObservableCriterion use.
     * @param values The observables values.
     * @param lambdas The observables weighted parameters, if any.
     * @param reinject If true defined desired values constrained by the observable.
     */
    ObservableCriterion(KernelTransform & transform, std::vector < Observable * > observables, const double *values, const double *lambdas = NULL, bool reinject = false);
    /// @cond INTERNAL
    ~ObservableCriterion();
    ///@endcond
    double rho() const;
    double drho(unsigned int n, double t) const;

    virtual void update();
    virtual double get(unsigned int n, double t) const;
    virtual unsigned int getN0() const;

    /** Gets an observable expected value.
     * @param k The observable index.
     */
    double getObservableExpectedValue(unsigned int k) const;

    /** Gets a predefined observable.
     * @param name : The predefined observable name.
     * - mean : The input mean of a given channel \f$\omega_n(t) = x_n(t)\f$.
     *   - Parameter: The unit index <tt>n</tt>.
     * - icorr : The input inter-correlation between two channels \f$\omega_{n,m}(t) = x_n(t) \, x_m(t)\f$.
     *   - Parameters: The unit index <tt>n</tt> and the unit index <tt>m</tt>.
     * - acorr : The input auto-correlation of a given channel \f$\omega_{n,\tau}(t) = x_n(t) \, x_n(t - \tau)\f$.
     *   - Parameters: The unit index <tt>n</tt> and the delay <tt>tau</tt> (as unsigned int), with <tt>tau = 0</tt>, simply returns the 2nd order momentum, <tt>tau</tt> values above 16 are numericaly unrealistic.
     * @return The corresponding observable, to be deleted after use.
     */
    static Observable *getObservable(String name, ...);

    /** Gets a predefined observable array.
     * @param name : The predefined observable name.
     * - mean : The \f$N\f$ mean for each channel \f$\omega_n(t) = x_n(t), 0 \leq n < N\f$.
     *   - Parameter: The number of channels <tt>N</tt>, values above 32 are numericaly unrealistic.
     * - icorr : The \f$N + N \, (N+1) / 2\f$ mean and inter-correlation between two channels \f$\omega_n(t) = x_n(t), \omega_{N + m + n * (n+1)/2}(t) = x_n(t) \, x_m(t), 0 \leq n < N, n \leq m < N\f$.
     *   - Parameter: The number of channels <tt>N</tt>, values above 8 are numericaly unrealistic.
     * - acorr : The \f$N + N \, (\tau + 1)\f$ mean and auto-correlation for each channel \f$omega_n(t) = x_n(t), \omega_{N + n+ \tau \, N}(t) = x_n(t) \, x_n(t - \tau), 0 \leq n < \tau\f$.
     *   - Parameter: The number of channels <tt>N</tt>, and the maximal delay <tt>tau</tt> (as unsigned int), with <tt>tau = 0</tt>, simply implements 1st and 2nd order momenta, <tt>N * tau</tt> values above 32 are numericaly unrealistic..
     * @return The corresponding observables array, each observable are to be deleted with deleteObservables().
     */
    static std::vector < Observable * > getObservables(String name, ...);

    /** Deletes a predefined observable array.
     * @param observables The observable array to delete.
     */
    static void deleteObservables(std::vector < Observable * >& observables);
  };
}
