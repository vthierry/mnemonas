namespace network {
/** Defines a recurrent network as a fixed leak sparse rectified linear combination of input and recurrent values.
 * - Such recurrent network writes:
 * <center>\f$\begin{array}{rcl}
 *  x_n(t) &=& \gamma \, x_{n}(t-1) + \sum_{d = 0}^{D - 1} W_{nn_d} \, \zeta_{[0,\infty]}\left(x_{n_d}(t-1)\right) + \sum_{m = 0}^{M-1} W_{nm} \, i_m(t-1)
 *  \end{array}\f$</center>
 * **Weight organization**:
 * |   \f$(n,d)\f$     |                       |                                         |
 * | ----------------- | --------------------- | --------------------------------------- |
 * |  \f$(n,d)\f$      | \f$d \in \{0,D\{\f$   | Recurrent weights \f$W_{nn_d}\f$        |
 * |  \f$(n,D+m)\f$    | \f$m  \in \{0,M\{\f$  | Input weights \f$W_{nm}\f$              |
 * - In order to avoid unbounded values, since using a ReLU profile, a saturation at \f$\pm10^{6}\f$ is introduced.
 */
  class SparseNonLinearTransform: public KernelTransform {
private:
    unsigned int N, *D, DN, *offsets, *indexes;
    bool *connected;
    double leak;
    const double SAT = 10e6;
public:
    /** Resets the transform for the given parameters.
     * @param N The number of input units, defined unit indexes stand in <tt>{0, N{</tt>.
     *  - The network has <tt>2\,N</tt> nodes.
     * @param input The transform input.
     */
    SparseNonLinearTransform(unsigned int N, const Input &input);

    /** Resets the transform for the given parameters and connection.
     * @param transform The root sparse transform.
     */
    SparseNonLinearTransform(const SparseNonLinearTransform &transform);

    /** Sets a fixed common leak for this network.
     * @param value The leak value \f$\gamma\f$ for all units.
     * @return this.
     */
    SparseNonLinearTransform& setLeak(double value = 0);

    /** Randomly draws the sparse connections.
     * @param D     The number of random connections by unit. The 0 default value corresponds to \f$\max(2, \sqrt{N})\f$ connections.
     * @param seed  If seed == -1 changes randomly the seed, taking the system time. The initial seed is 0, at the beginning of the program.
     * @return this.
     */
    SparseNonLinearTransform& setConnections(unsigned int D = 0, int seed = -1);
    ~SparseNonLinearTransform();
    KernelTransform& setWeights(const KernelTransform& network);
    unsigned int getKernelDimension(unsigned int n) const;
    double getKernelValue(unsigned int n, unsigned int d, double t) const;
    double getKernelDerivative(unsigned int n, unsigned int d, double t, unsigned int n_, double t_) const;
    bool isConnected(unsigned int n, double t, unsigned int n_, double t_) const;
  };
}
