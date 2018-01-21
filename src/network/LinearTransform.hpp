namespace network {
/** Defines a recurrent network as an affine combination of input and recurrent values.
 * - Such recurrent network writes:
 * <center>\f$x_n(t) = \alpha_n + \sum_{n'= 0}^{N-1} W_{nn'} \, x_{n'}(t-1) + \sum_{m=0}^{M-1} W_{nm} \, i_m(t-1)\f$</center>
 * **Weight organization**:
 * |  \f$(n,d)\f$     |                      |                                         |
 * | ---------------- | -------------------- | --------------------------------------- |
 * |  \f$(n,1)\f$     |                      | Unit offset \f$\alpha_n\f$              |
 * |  \f$(n,1+n')\f$  | \f$n' \in \{0,N\{\f$ | Recurrent weights \f$W_{nn'}\f$         |
 * |  \f$(n,1+N+m)\f$ | \f$m  \in \{0,M\{\f$ | Input weights \f$W_{nm}\f$              |
 * - In order to avoid unbounded values, a saturation at \f$\pm10^{6}\f$ is introduced.
 */
  class LinearTransform: public KernelTransform {
private:
    double offset;
    double sat(double v) const;
public:
    /** Resets the transform for the given parameters.
     * @param N The number of units, defined unit indexes stand in <tt>{0, N{</tt>.
     * @param input The transform input.
     */
    LinearTransform(unsigned int N, const Input &input);

    /** Sets a fixed common offset for this network.
     * @param value The offset value of all \f$\alpha_n\f$, NAN to unconstrain the value.
     * @return this.
     */
    LinearTransform& setOffset(double value = 0);

    bool setWeight(unsigned int n, unsigned int d, double w);
    KernelTransform& setWeights(const KernelTransform& network);
    unsigned int getKernelDimension(unsigned int n) const;
    double getKernelValue(unsigned int n, unsigned int d, double t) const;
    double getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const;
    bool isConnected(unsigned int n, double t, unsigned int n_, double t_) const;
  };
}
