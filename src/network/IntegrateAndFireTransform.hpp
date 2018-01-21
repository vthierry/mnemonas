namespace network {
/** Defines a recurrent network as an approximate integrate and fire transform.
 * - Such recurrent network writes:
 * <center>\f$\begin{array}{rcl}
 *  x_n(t) &=& \gamma_n \, \left(1 - \zeta_{[0,1]}\left(x_{n_1}(t)\right)\right) \, x_n(t-1) + \sum_{n' = 0}^{N} W_{nn'} \, \zeta_{[0,1]}\left(x_{n'_1}(t)\right) + \sum_{m = 0}^{N} W_{nm} \, i_m(t-1), \\
 *  x_{n_1}(t) &=& \frac{1}{\epsilon} \, (x_n(t-1) - 1/2) \\
 *  \end{array}\f$</center>
 * **Weight organization**:
 * |   \f$(n,d)\f$    |                      |                                         |
 * | -----------------| -------------------- | --------------------------------------- |
 * |  \f$(n,1)\f$     |                      | Unit leak \f$\gamma_n \in [0, 0.999]\f$ |
 * |  \f$(n,2)\f$     |                      | Unit offset \f$\alpha_n\f$              |
 * |  \f$(n,2+n')\f$  | \f$n' \in \{0,N\{\f$ | Recurrent weights \f$W_{nn'}\f$         |
 * |  \f$(n,2+N+m)\f$ | \f$m  \in \{0,M\{\f$ | Input weights \f$W_{nm}\f$              |
 * |  \f$(n+N,1)\f$   |                      | Unit sharpness \f$\frac{1}{\epsilon} \in [0, 100]\f$ |
 * with \f$n_1 = N - n \f$.
 * - In order to guaranty the numerical stability, the sharpness is bounded.
 */
  class IntegrateAndFireTransform: public KernelTransform {
private:
    unsigned int N;
    double sharpness, leak, offset;
    double zeta(double u) const;
    double dzeta(double u) const;
public:
    /** Resets the transform for the given parameters.
     * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
     *  - The network has <tt>2\,N</tt> nodes.
     * @param input The transform input.
     */
    IntegrateAndFireTransform(unsigned int N, const Input &input);

    /** Sets a fixed common sharpness for this network.
     * @param value The sharpness  value of all \f$\alpha_n\f$, NAN to unconstrain the value, 0 to impose a Heaviside profile.
     * @return this.
     */
    IntegrateAndFireTransform& setSharpness(double value = 1);

    /** Sets a fixed common leak for this network.
     * @param value The leak value of all \f$\gamma_n\f$, NAN to unconstrain the value.
     * @return this.
     */
    IntegrateAndFireTransform& setLeak(double value = 0);

    /** Sets a fixed common offset for this network.
     * @param value The offset value of all \f$\alpha_n\f$, NAN to unconstrain the value.
     * @return this.
     */
    IntegrateAndFireTransform& setOffset(double value = 0);
    bool setWeight(unsigned int n, unsigned int d, double w);
    KernelTransform& setWeights(const KernelTransform& network);
    unsigned int getKernelDimension(unsigned int n) const;
    double getKernelValue(unsigned int n, unsigned int d, double t) const;
    double getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const;
    bool isConnected(unsigned int n, double t, unsigned int n_, double t_) const;
  };
}
