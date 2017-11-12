namespace network {
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
 * |  \f$(n+N,1)\f$        |                      | Unit leak \f$\gamma_n \in [0, 0.999]\f$ |
 * |  \f$(n+2\,N,1)\f$     |                      | Unit offset \f$\alpha_n\f$              |
 * |  \f$(n+2\,N,2+n')\f$  | \f$n' \in \{0,N\{\f$ | Recurrent weights \f$W_{nn'}\f$         |
 * |  \f$(n+2\,N,2+N+m)\f$ | \f$m  \in \{0,M\{\f$ | Input weights \f$W_{nm}\f$              |
 * with \f$n_1 = N - n, n_2 = 2\,N  - n\f$.
 * - In order to avoid unbounded values, when using a ReLU profile, a saturation at \f$\pm10^{6}\f$ is introduced.
 */
  class LinearNonLinearTransform: public KernelTransform {
private:
    unsigned int N, N2;
    double offset, leak, a, b, c;
    // Defines a sigmoid, a = min, b = max - min, c = -4 z'(0) / b
    double zeta(double u) const;
    double dzeta(double u) const;
public:
    /** Resets the transform for the given parameters.
     * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
     *  - The network has <tt>3\,N</tt> nodes.
     * @param input The transform input.
     * @param min The minimal value of the sigmoid non-linearity.
     * @param max The maximal value of the sigmoid non-linearity. If <tt>max == INFINITY</tt> defines a ReLU, i.e. a rectification, i.e. \f$\zeta_{[min,\infty]}\left(u\right) = min + u \, H(u)\f$.
     */
    LinearNonLinearTransform(unsigned int N, const Input &input, double min = 0, double max = 1);

    /** Sets a fixed common leak for this network.
     * @param value The leak value of all \f$\gamma_n\f$, NAN to unconstrain the value.
     * @return this.
     */
    LinearNonLinearTransform& setLeak(double value = 0);

    /** Sets a fixed common offset for this network.
     * @param value The offset value of all \f$\alpha_n\f$, NAN to unconstrain the value.
     * @return this.
     */
    LinearNonLinearTransform& setOffset(double value = 0);
    bool setWeight(unsigned int n, unsigned int d, double w);
    KernelTransform& setWeights(KernelTransform& network);
    unsigned int getKernelDimension(unsigned int n) const;
    double getKernelValue(unsigned int n, unsigned int d, double t) const;
    double getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const;
  };
}
