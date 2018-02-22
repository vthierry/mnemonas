namespace network {
/** Defines a static buffered input of a calculation network.
 *
 * \nosubgrouping
 */
  class BufferedInput: public Input {
    const double *values;
    std::vector < double > *vvalues;
    bool buffered, vectored;
    static double mirror(const Input& input, unsigned int n, unsigned int t)
    {
      return input.get(n, t);
    }
public:
    /** @name Buffer construction by copy */
    // @{

    /** Resets the input from another input with a predefined function.
     * - All input values are called once in sequence during object construction.
     * @param input The input to buffer.
     * @param N0 If <tt>0 < N0</tt> considers only N0 units.
     * @param n0 If <tt>0 < n0</tt> considers only units starting at the n0 index.
     * @param T0 If <tt>0 < T0</tt> considers only T0 time-steps.
     * @param t0 If <tt>0 < t0</tt> considers only time-steps starting at the t0 index.
     */
    BufferedInput(const Input &input, unsigned int N0 = 0, unsigned int n0 = 0, unsigned int T0 = 0, unsigned int t0 = 0);
    ///@cond INTERNAL
    BufferedInput(const BufferedInput &input);
    ///@endcond

    /** Resets the input with static values.
     * @param values A <tt>double[T][N]</tt> buffer with the input values, stored a T sequences of N values.
     * - The values are not copied, the buffer must thus be available during the whole object life.
     * @param N The number of input units.
     * @param T The maximal input time.
     */
    BufferedInput(const double *values, unsigned int N, unsigned int T);

    /** Resets the input with values loaded from a file.
     * @param file The file name, an extension is added to this name.
     * @param format
     *   - "binary-unit-time" : ascii integer value header, then unit's value at a given time in sequence, as 64-bits double floating point values:
     *     - <tt>N T\\nget(0, 0) get(1, 0) get(N-1, 0) get(0, 1) get(1, 1) .. get(N-1, 1)</tt>
     *     - The <tt>.dat</tt> extension is added to the file-name.
     *   - "csv" : each line stores unit's value at a given time, in sequence, in ascii:
     *     - <tt>get(0, 0),get(1, 0), .. get(N-1, 0)\\nget(0, 1),get(1, 1), .. get(N-1, 1)</tt>
     *     - The <tt>.csv</tt> extension is added to the file-name.
     */
    BufferedInput(String file, String format = "binary-unit-time");

    // @}
    /** @name Buffer construction by calculation */
    // @{

    /** Resets the input with predefined values.
     * @param name The sequence name.
     * - "normal: A 0 mean and 1 standard-deviation pseudo-random normal value sequence.
     *   - Parameter: A <tt>seed::unsigned int</tt> specifying if the sequence is reproducible (using same seed value at each call) or not.
     * - "sierpinski": The Sierpinski sequence, i.e. the \f$N\f$ lower bits binary decomposition (with \f$\{0, 1\}\f$ values) of the Pascal triangle read from left to right and form top to down in sequence.
     *   - Parameter: None.
     * - "zerone": Generates a random binary sequence of a given period.
     *   - Parameter: The binary sequence period.
     * - "ramp": Generates ramps of period <tt>T/(n+1)</tt>, i.e., \f$x_n(t) = t \mbox{ mod } \frac{T}{n+1}\f$.
     *   - Parameter: None.
     * - "mean" : A reproducible pseudo-random normal value sequence, with a given mean.
     *   - Parameter: A <tt>mean::double</tt> value. The standard-deviation is <tt>mean / 10</tt>.
     * - "icorr": A reproducible pseudo-random normal value sequence, with inter-correlation between each pair of unit.
     *   - Parameter: A <tt>icorr::double \f$\in[-1,1]\f$</tt> inter-correlation value.
     * - "acorr": A reproducible pseudo-random normal value sequence, with auto-correlation for each unit.
     *   - Parameter: A <tt>acorr::double \f$\in[-1/2,1/2]\f$</tt> auto-correlation value.
     * @param N The number of input units.
     * @param T The maximal input time.
     */
    BufferedInput(String name, unsigned int N, unsigned int T, ...);

    /** Resets the input applying a predefined transformation on another input.
     * @param input The input to buffer.
     * @param name The tranformation name.
     * - "affine: Performs an affine transform <tt>output(n, t) = offset + gain * input(n, t)</tt>.
     *   - Parameters: The <tt>offset::double</tt> and the <tt>gain::double</tt>
     * - "noise": Adds some noise to the input drawn from a normal distribution.
     *   - Parameters: The probability to add noise to a given value <tt>noiseProbability::double</tt> and the standard deviation of the normal added noise <tt>noiseStandardDeviation::double</tt>
     */
    BufferedInput(const Input &input, String name, ...);

    ///@cond INTERNAL
    virtual ~BufferedInput();
    ///@endcond

    // @}
    /** @name Variable length buffer interface */
    // @{

    /** Resets the input with static variable time length values.
     * - Values are added in sequence using the add() method.
     * @param N The number of input units.
     */
    BufferedInput(unsigned int N);

    /** Adds a value if (and only if) this is a variable time length buffer.
     * The time-length is incremented at each step.
     * @param value A <tt>double[N]</tt> array with the current time value.
     */
    void add(const double *value);

    // @}
    /** @name Buffered input read */
    // @{

    double get(unsigned int n, double t) const;

    /** Saves the recorded values.
     * @param file The data file name, an extension is added to this name: <tt>.csv</tt> if using the "csv" format, <tt>.dat</tt> otherwise.
     *   - "stdout" : printf the result, in "ascii-unit-time"
     * @param format
     *   - "binary-unit-time" : as defined for BufferedInput::load()
     *   - "csv" : as defined for BufferedInput::load()
     *   - "ascii-unit-time" :  each line stores unit's value at a given time, in sequence, in ascii:
     *     - <tt>get(0, 0) get(1, 0) .. get(N-1, 0)\\nget(0, 1) get(1, 1) .. get(N-1, 1)</tt>
     *   - "gnuplot" : each line stores a value with indexes, in ascii,
     *     - <tt>n t get(n, t)\n</tt>
     *   - "gnuimg" : stores the data as a float image, each line stores the <tt>n</tt>-th unt time sequence
     *     - <tt>get(n, 0) get(n, 1) .. get(n, T-1)</tt>
     * - For "gnuplot" and "gnuimg" a <a href="http://gnuplot.sourceforge.net">gnuplot</a> display script and image is generated, with extensions <tt>.gnuplot.sh</tt> and <tt>.png</tt>.
     * @param show When considering the "gnuplot" or "gnuimg" format, if true, performs on the fly display.
     */
    void save(String file, String format = "binary-unit-time", bool show = false) const;

    /** Returns some value statistics.
     * @param what The list of unit index to consider, by default all values are taken into account.
     * @return The given statistics.
     */
    const Histogram getHistogram(String what = "") const;

    // @}
  };
}
