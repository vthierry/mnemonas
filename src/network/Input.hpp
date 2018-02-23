namespace network {
/** Defines the input of a calculation network. */
  class Input {
    bool mutable_;
public:
    /** Gets the number of input units. */
    const unsigned int N; 
    /** Gets the maximal time range. */
    const double T;
public:
    /** Resets the input dimensions.
     * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
     * @param T The maximal time range, defined times stand in <tt>[0, T[</tt>.
     * @param mutable_ If true N and T can be modified.
     */
    Input(unsigned int N, double T, bool mutable_ = true);
    /// @cond INTERNAL
    virtual ~Input() {}
    ///@endcond
    /** Sets the number of input units.
     * @param N The number of input units.
     */
    virtual void setN(unsigned int N);
    /** Sets the maximal time range. 
     * @param T The maximal time range
     */
    virtual void setT(double T);
    /** Returns the input value.
     * - The method is to be overwritten to implement a given input.
     * @param n The input unit index.
     * @param t The value time.
     * @return The input value if defined, else raises a fatal error.
     */
    virtual double get(unsigned int n, double t) const;
  };
}
