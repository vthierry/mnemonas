/** Defines a recurrent network as a non-linear affine combination of input and recurrent values.
 * - Such recurrent network writes:
 * <center>\f$\begin{array}{rcl}
 *  x_n(t) &=& x_{n_1}(t) + \zeta_{[min,max]}\left(x_{n_2}(t)\right) \\
 *  x_{n_1}(t) &=& \gamma_n \, x_{n}(t-1) \\
 *  x_{n_2}(t) &=& \alpha_n + \sum_{n' = 0}^{N-1} W_{nn'} \, x_{n'}(t-1) + \sum_{m = 0}^{M-1} W_{nm} \, i_m(t-1) \\
 *  \end{array}\f$</center>
 * **Weight organization**:
 * |   \f$(n,d)\f$         |                      |                                         |
 * | --------------------- | -------------------- | --------------------------------------- |
 * |  \f$(n+N,1)\f$        |                      | Unit leak \f$\gamma_n\f$                |
 * |  \f$(n+2\,N,1)\f$     |                      | Unit offset \f$\alpha_n\f$              |
 * |  \f$(n+2\,N,1+n')\f$  | \f$n' \in \{0,N\{\f$ | Recurrent weights \f$W_{nn'}\f$         |
 * |  \f$(n+2\,N,1+N+m)\f$ | \f$m  \in \{0,M\{\f$ | Input weights \f$W_{nm}\f$              |
 * with \f$n_1 = n + N, n_2 = n + 2\,N\f$.
 */
class LinearNonLinearTransform: public KernelTransform {
private:
  unsigned int N, N2;
  double offset, leak, a, b, c;
  // Defines a sigmoid, a = min, b = max - min, c = -4 z'(0) / b
  double zeta(double u) const
  {
    return a + b / (1 + exp(c * u));
  }
  double dzeta(double u) const
  {
    double e = exp(c * u), d = 1 + e;
    return -b * c / d * d;
  }
public:
  /** Resets the transform for the given parameters.
   * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
   *  - The network has <tt>3 N</tt> nodes.
   * @param input The transform input.
   * @param min The minimal value of the sigmoid non-linearity.
   * @param max The maximal value of the sigmoid non-linearity.
   */
  LinearNonLinearTransform(unsigned int N, const Input &input, double min = 0, double max = 1) : KernelTransform(3 * N, 1, input), N(N), N2(2 * N), offset(NAN), leak(NAN), a(min), b(max - min), c(-4 / (max - min)) {
    assume(min < max, "illegal-argument", "in network::LinearNonLinearTransform::LinearNonLinearTransform, spurious non-linearity range [%g, %g]", min, max);
  }

  /** Sets a fixed common leak for this network.
   * @param value The leak value of all \f$\gamma_n\f$, NAN to unconstrain the value.
   * @return this.
   */
  LinearNonLinearTransform& setLeak(double value = 0)
  {
    leak = value;
    if(!isnan(value))
      for(unsigned int n = N; n < N2; n++)
        KernelTransform::setWeight(n, 1, value);
    return *this;
  }
  /** Sets a fixed common offset for this network.
   * @param value The offset value of all \f$\alpha_n\f$, NAN to unconstrain the value.
   * @return this.
   */
  LinearNonLinearTransform& setOffset(double value = 0)
  {
    offset = value;
    if(!isnan(value))
      for(unsigned int n = N2; n < N2 + N; n++)
        KernelTransform::setWeight(n, 1, value);
    return *this;
  }
  bool setWeight(unsigned int n, unsigned int d, double w)
  {
    if(((N <= n) && (n < N2) && (d == 1) && !isnan(leak)) || ((N2 <= n) && (d == 1) && !isnan(offset)))
      return false;
    else if(((N <= n) && (n < N2) && (d == 1)) && (w < 0))
      return KernelTransform::setWeight(n, d, 0);
    else
      return KernelTransform::setWeight(n, d, w);
  }
  unsigned int getKernelDimension(unsigned int n) const
  {
    return n < N ? 0 : n < N2 ? 1 : 1 + N + input.getN();
  }
  double getKernelValue(unsigned int n, unsigned int d, double t) const
  {
    if(n < N)
      return d == 0 ? get(n + N, t) + zeta(get(n + N2, t)) : 0;
    else if(n < N2)
      return d == 1 ? get(n - N, t - 1) : 0;
    else {
      if(d == 0)
        return 0;
      if(d == 1)
        return 1;
      d -= 2;
      if(d < N)
        return get(d, t - 1);
      d -= N;
      if(d < input.getN())
        return input.get(d, t - 1);
      return 0;
    }
  }
  double getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
  {
    return n < N ? (d == 0 && t_ == t ? (n_ == n + N ? 1 : (n_ == n + N2 ? dzeta(get(n_, t)) : 0)) : 0) :
           n < N2 ? (d == 1 && n_ == n - N && t_ == t - 1 ? 1 : 0) :
           (1 < d && d <= N + 1 && n_ == d - 2 && t_ == t - 1 ? 1 : 0);
  }
};
