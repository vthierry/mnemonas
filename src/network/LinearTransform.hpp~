/** Defines a recurrent network as an affine combination of input and recurrent values.
 * - Such recurrent network writes:
 * <center>\f$x_n(t) = \sum_{n'= 0}^{N-1} W_{nn'} \, x_{n'}(t-1) + \sum_{m=0}^{M-1} W_{nm} \, i_m(t-1)\f$</center>
 * **Weight organization**:
 * |  \f$(n,d)\f$   |                      |                                         |
 * | -------------- | -------------------- | --------------------------------------- |
 * |  \f$(n,n')\f$  | \f$n' \in \{0,N\{\f$ | Recurrent weights \f$W_{nn'}\f$         |
 * |  \f$(n,N+m)\f$ | \f$m  \in \{0,M\{\f$ | Input weights \f$W_{nm}\f$              |
 */
class LinearTransform: public KernelTransform {
private:
  double offset;
public:
  /** Resets the transform for the given parameters.
   * @param N The number of units, defined unit indexes stand in <tt>{0, N{</tt>.
   * @param input The transform input.
   */
  LinearTransform(unsigned int N, const Input &input) : KernelTransform(N, 1, input) {}

  unsigned int getKernelDimension(unsigned int n) const
  {
    return getN() + input.getN();
  }
  double getKernelValue(unsigned int n, unsigned int d, double t) const
  {
    assume(n < N && d <= getKernelDimension(n), "illegal-argument", "network::LinearTransform::getKernelValue(%d, %d, t) out of bounds", n, d);
    if(d > 0) {
      d -= 1;
      if(d < getN())
        return get(d, t - 1);
      d -= getN();
      if(d < input.getN())
        return input.get(d, t - 1);
    }
    return 0;
  }
  double getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const
  {
    assume(n < N && d <= getKernelDimension(n) && n_ < N, "illegal-argument", "network::LinearTransform::getKernelDerivative(%d, %d, t, %d, t_) out of bounds", n, d, n_);
    return (0 < d && n_ == d - 1 && t_ == t - 1 && d <= getN()) ? 1 : 0;
  }
};
