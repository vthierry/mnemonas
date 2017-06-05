/** Defines a recurrent network as an approximate integrate and fire transform.
 * - Such recurrent network writes:
 * <center>\f$\begin{array}{rcl}
 *  x_n(t) &=& \gamma_n \, \left[(1-\zeta_{[0,1]}(x_{n_1}(t))) \, x_n(t-1)\right] \\
 *  &+& \sum_{n' = 0}^{N} W_{nn'} \, \zeta_{[0,1]}\left(x_{n'_1}(t-1)\right) +
 *     \sum_{m = 0}^{N} W_{nm} \, i_m(t-1) \\
 *  x_{n_1}(t) &=& \frac{1}{\epsilon} \, (x_n(t-1) - 1) \\
 *  \end{array}\f$</center>
 * **Weight organization**:
 * |   \f$(n,d)\f$    |                      |                                         |
 * | -----------------| -------------------- | --------------------------------------- |
 * |  \f$(n,1)\f$     |                      | Unit leak \f$\gamma_n\f$                |
 * |  \f$(n,1+n')\f$  | \f$n' \in \{0,N\{\f$ | Recurrent weights \f$W_{nn'}\f$         |
 * |  \f$(n,1+N+m)\f$ | \f$m  \in \{0,M\{\f$ | Input weights \f$W_{nm}\f$              |
 * |  \f$(n+N,1)\f$   |                      | Unit sharpness \f$\frac{1}{\epsilon}\f$ |
 * with \f$n_1 = n + N\f$.
 */
class IntegrateAndFireTransform: public KernelTransform {
private:
  unsigned int N;
  double sharpness, leak;
  double zeta(unsigned int n, double u) const
  {
    if(getWeight(N + n, 1) == 0)
      return u < 0 ? 0 : u > 0 ? 1 : 0.5;
    return 1 / (1 + exp(-4 * u));
  }
  double dzeta(unsigned int n, double u) const
  {
    if(getWeight(N + n, 1) == 0)
      return 0;
    double e = exp(-4 * u), d = 1 + e;
    return 4 / d * d;
  }
public:
  /** Resets the transform for the given parameters.
   * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
   *  - The network has <tt>3 N</tt> nodes.
   * @param input The transform input.
   */
  IntegrateAndFireTransform(unsigned int N, const Input &input) : KernelTransform(2 * N, 1, input), N(N), leak(NAN) {
    setSharpness(1);
    sharpness = NAN;
  }

  /** Sets a fixed common leak for this network.
   * @param value The leak value of all \f$\gamma_n\f$, NAN to unconstrain the value, 0 to impose a Heaviside profile.
   * @return this.
   */
  IntegrateAndFireTransform& setLeak(double value = 0)
  {
    leak = value;
    if(!isnan(value))
      for(unsigned int n = 0; n < 1; n++)
        KernelTransform::setWeight(n, 1, value);
    return *this;
  }
  /** Sets a fixed common sharpness for this network.
   * @param value The sharpness  value of all \f$\alpha_n\f$, NAN to unconstrain the value.
   * @return this.
   */
  IntegrateAndFireTransform& setSharpness(double value = 1)
  {
    sharpness = value;
    if(!isnan(value))
      for(unsigned int n = N; n < N * 2; n++)
        KernelTransform::setWeight(n, 1, value);
    return *this;
  }
  bool setWeight(unsigned int n, unsigned int d, double w)
  {
    if(((N <= n) && (d == 1) && !isnan(leak)) || ((N < n) && (d == 1) && !isnan(sharpness)))
      return false;
    else if((((N <= n) && (d == 1)) || ((N < n) && (d == 1))) && (w < 0))
      return KernelTransform::setWeight(n, d, 0);
    else
      return KernelTransform::setWeight(n, d, w);
  }
  unsigned int getKernelDimension(unsigned int n) const
  {
    return n < N ? 1 + N + input.getN() : 1;
  }
  double getKernelValue(unsigned int n, unsigned int d, double t) const
  {
    if(n < N) {
      if(d == 0)
        return 0;
      if(d == 1)
        return (1 - zeta(n, get(n + N, t))) * get(n, t - 1);
      d -= 2;
      if(d < N)
        return zeta(n, get(d + N, t - 1));
      d -= N;
      if(d < input.getN())
        return input.get(d, t - 1);
      return 0;
    } else
      return d == 1 ? get(n - N, t - 1) - 1 : 0;
  }
  double getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
  {
    return n < N ? (d == 1 ?
                    (n_ == n + N && t_ == t ? -dzeta(n, get(n_, t)) :
                     (n_ = n && t_ == t - 1 ? 1 : 0)) :
                    (1 < d && d <= N + 1 && n_ == d - 2 && t_ == t - 1 ? dzeta(n, get(d + N, t_)) :
                     0)) :
           d == 1 && n_ == n - N && t_ == t - 1 ? 1 : 0;
  }
};
