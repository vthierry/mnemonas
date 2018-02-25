namespace network {
  /** Defines an observable over an input.
   * - An Observable maps an Input onto a global (usually statistical) value.
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
     *  for(unsigned int t = 0; t < input->T; t++)
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
}
