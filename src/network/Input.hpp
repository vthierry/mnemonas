/** Defines the input of a calculation network. */
class Input {
protected:
  unsigned int N;
  double T;
public:
  /** Resets the input dimensions.
   * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
   * @param T The maximal time range, defined times stand in <tt>[0, T[</tt>.
   */
  Input(unsigned int N, double T) : N(N), T(T) {}
  /** Gets the number of input units. */
  unsigned int getN() const
  {
    return N;
  }
  /** Gets the maximal time range. */
  double getT() const
  {
    return T;
  }
  /** Returns the input value.
   * - The method is to be overwritten to implement a given input.
   * @param n The input unit index.
   * @param t The value time.
   * @return The input value if defined, else raises a fatal error.
   */
  virtual double get(unsigned int n, double t) const
  {
    assume(false, "illegal-state", "in network::Input::get, this virtual method must be overloaded");
    return NAN;
  }
};
