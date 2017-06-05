/** Defines a static buffered input of a calculation network. */
class BufferedInput: public Input {
  const double *values;
  std::vector < double > *vvalues;
  bool buffered, vectored;
public:
  BufferedInput(const BufferedInput &input);

  /** Resets the input with static values.
   * @param values A <tt>double[T][N]</tt> buffer with the input values, stored a T sequences of N values.
   * - The values are not copied, the buffer must thus be available during the whole object life.
   * @param N The number of input units.
   * @param T The maximal input time.
   */
  BufferedInput(const double *values, unsigned int N, unsigned int T);

  /** Resets the input with static variable time length values.
   * - Values are added in sequence using the add() method.
   * @param N The number of input units.
   */
  BufferedInput(unsigned int N);

  /** Resets the input with an input.
   * - All input values are called once in sequence during object construction.
   * @param input The input to buffer.
   * @param N0 If <tt>0 < N0</tt> considers only the 1st N0 units.
   */
  BufferedInput(const Input &input, unsigned int N0 = 0);

  /** Resets the input with random values.
   * - Data are drawn once from either a uniform or a normal distribution.
   *   - The uniform distribution stands in the  \f$[\min, \max] = [m - \sqrt{3} \, \sigma, m + \sqrt{3} \, \sigma] = m \pm 1.732 \, \sigma \f$ interval.
   * @param N The number of input units.
   * @param T The maximal input time.
   * @param mean The distribution mean.
   * @param sigma The distribution standard-deviation.
   * @param mode Either "uniform" or "normal" distribution.
   * @param seed If seed == -1 changes randomly the seed, else generates a reproducible pseudo random sequence.
   */
  BufferedInput(unsigned int N, unsigned int T, double mean = 0, double sigma = 1, String mode = "uniform", int seed = -1);

  /** Resets the input with values loaded from a file.
   * @param file The file name, a <tt>.dat</tt> extension is added to this name.
   * @param format
   *   - "binary-unit-time" : ascii integer value header, then unit's value at a given time in sequence, as 64-bits double floating point values:
   *     - <tt>N T\\nget(0, 0) get(1, 0) get(N-1, 0) get(0, 1) get(1, 1) .. get(N-1, 1)</tt>
   */
  BufferedInput(String file, String format = "binary-unit-time");

  /** Resets the input with predefined values.
   * @param name The sequence name.
   * - "sierpinski": The Sierpinski sequence, i.e. the \f$N\f$ lower bits binary decomposition (with \f$\{-1, 1\}\f$ values) of the Pascal triangle read from left to right and form top to down in sequence.
   * - "ramp": Generates ramps of period <tt>T/(n+1)</tt>, i.e., \f$x_n(t) = t \mbox{ mod } \frac{T}{n+1}\f$.
   * @param N The number of input units.
   * @param T The maximal input time.
   */
  BufferedInput(String name, unsigned int N, unsigned int T);
  ~BufferedInput();
  //
  double get(unsigned int n, double t) const
  {
    if(t < 0)
      return 0;
    if(vectored) {
      assume(n < N && t < vvalues[n].size(), "illegal-argument", "in network::BufferedInput::get, index out of range, we must have n=%d in {0, %d{ and t=%g in [0, %g[", n, N, t, vvalues[n].size());
      return vvalues[n][t];
    } else {
      assume(n < N && t < T, "illegal-argument", "in network::BufferedInput::get, index out of range, we must have n=%d in {0, %d{ and t=%g in [0, %g[", n, N, t, T);
      return values[n + N * (int) t];
    }
  }
  /** Adds a value if (and only if) this is a variable length buffer.
   * The time-length is incremented at each step.
   * @param value A <tt>double[N]</tt> array with the current time value.
   */
  void add(const double *value)
  {
    assume(vectored, "illegal-argument", "in network::BufferedInput::add, attempt to add a value on a fixed length buffer");
    for(unsigned int n = 0; n < N; n++)
      vvalues[n].push_back(value[n]);
    T++;
  }
  /** Saves the recorded values.
   * @param file The data file name, a <tt>.dat</tt> extension is added to this name.
   * @param format
   *   - "binary-unit-time" : as defined for BufferedInput::load()
   *   - "ascii-unit-time" :  each line stores unit's value at a given time, in sequence, in ascii:
   *     - <tt>get(0, 0) get(1, 0) get(N-1, 0)\\nget(0, 1) get(1, 1) .. get(N-1, 1)</tt>
   *   - "gnuplot" : each line stores a value with indexes, in ascii,
   *     - <tt>n t get(n, t)\n</tt>
   *   and a <a href="http://gnuplot.sourceforge.net">gnuplot</a> display script and image is generated, with extensions <tt>.gnuplot.sh</tt> and <tt>.png</tt>.
   *   - "gnushow" : same as the "gnuplot" option, but perfoms on the fly display.
   */
  void save(String file, String format = "binary-unit-time") const;

  /** Returns some value statistics.
   * @param what The list of unit index to consider, by default all values are taken into account.
   * @return The given statistics.
   */
  const Histogram getHistogram(String what = "") const;
};
