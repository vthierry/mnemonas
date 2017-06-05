/** Defines a recurrent network as a non-linear affine combination of input and recurrent values.
 * - Such recurrent network writes:
 * <center>\f$\begin{array}{rcl}
 *  x_n(t) &=& x_{n_1}(t) / z(t)\\
 *  z(t) &=& = \sum_n x_{n_1}(t)  \\
 *  x_{n_1}(t) &=& exp(x_{n_2}(t)) \\
 *  x_{n_2}(t) &=& \alpha_n + \sum_{n' = 0}^{N-1} W_{nn'} \, x_{n'}(t-1) + \sum_{m = 0}^{M-1} W_{nm} \, i_m(t-1) \\
 *  \end{array}\f$</center>
 * **Weight organization**:
 * |   \f$(n,d)\f$           |                      |                                         |
 * | ----------------------- | -------------------- | --------------------------------------- |
 * |  \f$(n+2\,N+1,1)\f$     |                      | Unit offset \f$\alpha_n\f$              |
 * |  \f$(n+2\,N+1,1+n')\f$  | \f$n' \in \{0,N\{\f$ | Recurrent weights \f$W_{nn'}\f$         |
 * |  \f$(n+2\,N+1,1+N+m)\f$ | \f$m  \in \{0,M\{\f$ | Input weights \f$W_{nm}\f$              |
 * with \f$n_2 = n + 2 \, N + 1\f$.
 */
class SoftMaxTransform: public KernelTransform {
private:
  unsigned int N;
  double offset;
public:
  /** Resets the transform for the given parameters.
   * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
   *  - The network has <tt>3 N</tt> nodes.
   * @param input The transform input.
   */
  SoftMaxTransform(unsigned int N, const Input &input) : KernelTransform(3 * N + 1, 1, input), N(N), offset(NAN) {}

  /** Sets a fixed common offset for this network.
   * @param value The offset value of all \f$\alpha_n\f$, NAN to unconstrain the value.
   * @return this.
   */
  SoftMaxTransform& setOffset(double value = 0)
  {
    offset = value;
    if(!isnan(value))
      for(unsigned int n = N + 1; n < 2 * N + 1; n++)
        KernelTransform::setWeight(n, 1, value);
    return *this;
  }
  bool setWeight(unsigned int n, unsigned int d, double w)
  {
    if(((2 * N < n) && (n <= 3 * N + 1) && (d == 1) && !isnan(offset)))
      return false;
    else
      return KernelTransform::setWeight(n, d, w);
  }
  unsigned int getKernelDimension(unsigned int n) const
  {
    return n <= N ? 0 : 1 + N + input.getN();
  }
  double getKernelValue(unsigned int n, unsigned int d, double t) const
  {
    if(n < N)
      return d == 0 ? get(n + N + 1, t) / get(N, t) : 0;
    else if(n == N) {
      if(d == 0) {
        double v = 0;
        for(unsigned int n = 0; n < N; n++)
          v += get(n + N + 1, t);
        return v;
      } else
        return 0;
    } else if(n < 2 * N + 1)
      return d == 0 ? exp(get(n + 2 * N + 1, t)) : 0;
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
    if(n < N)
      return d == 0 && t_ == t ? (n_ = n + N + 1 ? 1 / get(N, t) : (n_ == N ? -get(n + N + 1, t) / (get(N, t) * get(N, t)) : 0)) : 0;
    else if(n == N)
      return d == 0 && t_ == t && N + 1 <= n && n < 2 * N + 1 ? 1 : 0;
    else if(n < 2 * N + 1)
      return d == 0 && t_ == t && n_ == n + 2 * N + 1 ? exp(get(n + 2 * N + 1, t)) : 0;
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
};
