namespace network {
/** Defines a recurrent network as a non-linear affine combination of input and recurrent values.
 * - Such recurrent network writes:
 * <center>\f$\begin{array}{rcl}
 *  x_n(t)     &=& x_{n_1}(t) / x_{z}(t)\\
 *  x_{z}(t)   &=& \sum_n x_{n_1}(t)  \\
 *  x_{n_1}(t) &=& exp(x_{n_2}(t)) \\
 *  x_{n_2}(t) &=& \alpha_n + \sum_{n' = 0}^{N-1} W_{nn'} \, x_{n'}(t-1) + \sum_{m = 0}^{M-1} W_{nm} \, i_m(t-1) \\
 *  \end{array}\f$</center>
 * **Weight organization**:
 * |   \f$(n,d)\f$           |                      |                                         |
 * | ----------------------- | -------------------- | --------------------------------------- |
 * |  \f$(n+2\,N+1,1)\f$     |                      | Unit offset \f$\alpha_n\f$              |
 * |  \f$(n+2\,N+1,2+n')\f$  | \f$n' \in \{0,N\{\f$ | Recurrent weights \f$W_{nn'}\f$         |
 * |  \f$(n+2\,N+1,2+N+m)\f$ | \f$m  \in \{0,M\{\f$ | Input weights \f$W_{nm}\f$              |
 * with \f$z = N, n_1 = N + 1  - n, n_2 = 2 \, N + 1 - n\f$.
 * - In order to avoid unbounded values, a saturation at \f$10^{10}\f$ is introduced, for the exponential.
 */
  class SoftMaxTransform: public KernelTransform {
private:
    unsigned int N, N1, N2, N3;
    double offset;
    // Here defined a ``safe saturated´´ exp()
    double zexp(double u) const;
public:
    /** Resets the transform for the given parameters.
     * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
     *  - The network has <tt>3\,N + 1</tt> nodes.
     * @param input The transform input.
     */
    SoftMaxTransform(unsigned int N, const Input &input);

    /** Sets a fixed common offset for this network.
     * @param value The offset value of all \f$\alpha_n\f$, NAN to unconstrain the value.
     * @return this.
     */
    SoftMaxTransform& setOffset(double value = 0);
    bool setWeight(unsigned int n, unsigned int d, double w);
    KernelTransform& setWeights(const KernelTransform& network);
    unsigned int getKernelDimension(unsigned int n) const;
    double getKernelValue(unsigned int n, unsigned int d, double t) const;
    double getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const;
    bool isConnected(unsigned int n, unsigned int n_) const;
  };
}
