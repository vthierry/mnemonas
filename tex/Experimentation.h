// Generates the experimental results of the paper
class Experimentation {
  std::string header;
  network::Input *input, *output;
  network::KernelTransform *transform;
  ExponentialDecayFit fit;
  unsigned int seed;
  bool verbose_but_nosave;
public:
  Experimentation() : input(NULL), output(NULL), transform(NULL), seed(0), verbose_but_nosave(false) {}
  ~Experimentation() {
    delete input;
    delete output;
    delete transform;
  }

  /** Checks if the file already exists.
   * @param filename The file name.
   */
  bool file_tobedone(String filename)
  {
    return access(filename.c_str(), F_OK) == -1;
  }
  /** Returns a new kernel transform adapated to the given test.
   * @param test The test to perform: "reverse_engineering", "sequence_generation".
   * @param type The nodes type: "LinearTransform", "LinearNonLinearTransform", "SparseNonLinearTransform", "SoftMaxTransform", "IntegrateAndFireTransform"
   * @param N The number of state units.
   * @param input The transform input.
   * @param root If true generates a reverse_engineering root network.
   */
  network::KernelTransform *newKernelTransform(String test, String type, unsigned int N, const network::Input& input, bool root = false)
  {
    network::KernelTransform *network = network::KernelTransform::newKernelTransform(type, N, input);
    if(test == "reverse_engineering") {
      if(type == "LinearNonLinearTransform")
        ((network::LinearNonLinearTransform *) network)->setLeak(root ? 0 : NAN);
      else if(type == "SparseNonLinearTransform")
        ((network::SparseNonLinearTransform *) network)->setConnections(0, 0);
      else if(type == "IntegrateAndFireTransform")
        ((network::IntegrateAndFireTransform *) network)->setSharpness(root ? 100 : NAN);
    } else if(test.substr(0, ((String) "sequence_generation").size()) == "sequence_generation") {
      if(type == "LinearTransform")
        ((network::LinearTransform *) network)->setOffset(NAN);
      else if(type == "LinearNonLinearTransform")
        ((network::LinearNonLinearTransform *) network)->setOffset(NAN).setLeak(NAN);
      else if(type == "IntegrateAndFireTransform")
        ((network::IntegrateAndFireTransform *) network)->setOffset(NAN).setSharpness(NAN);
    }
    network->setWeightsRandom(0, 0.5 / sqrt(N), false, "normal", root ? 0 : 1);
    return network;
  }
  /** Encapsulates an exponential decay fit latex result table construction. */
  class ResultTable {
    std::string param, param2, what, name, l0, l1, l2, l3, l4, l5, l6, s;
    unsigned int count;
    bool with_decay_and_bias;
public:
    /** Constructs the result table
     * @param param The variable parameter name, e.g., "Number of units".
     * @param param2 A 2nd optional variable parameter name, e.g., "Number of units".
     * @param what The table title parameter name, e.g., "Node type".
     * @param name The table title parameter name, e.g., "LinearTransform".
     * @param with_decay_and_bias Whether exponential fit decay and bias is to be shown or not.
     */
    ResultTable(String param, String param2, String what, String name, bool with_decay_and_bias = true) : param(param), param2(param2), what(what), name(name), l0(""), l1(""), l2(""), l3(""), l4(""), l5(""), l6(""), count(0), with_decay_and_bias(with_decay_and_bias) {}

    /** Adds a value.
     * @param v The variable parameter corresponding value.
     * @param fit The related exponential decy fit.
     */
    void add(String v, const ExponentialDecayFit& fit)
    {
      count++;
      l0 += "c";
      l1 += s_printf(" & %s", v.c_str());
      l2 += s_printf(" & %.0f", fit.getCount());
      l3 += s_printf(" & %.1e", fit.getMinimalValue());
      l4 += s_printf(" & %.0f", fit.getDecay());
      l5 += s_printf(" & %.1e", fit.getBias());
    }
    void add(double v, const ExponentialDecayFit& fit)
    {
      add(s_printf("%g", v), fit);
    }
    /** Adds two values.
     * @param v1 The 1st variable parameter corresponding value.
     * @param v2 The 2nd variable parameter corresponding value.
     * @param fit The related exponential decy fit.
     */
    void add2(double v1, double v2, const ExponentialDecayFit& fit)
    {
      add(s_printf("%g", v1), fit);
      l6 += s_printf(" & %g", v2);
    }
    /** Adds an empty value.
     * @param v The variable parameter corresponding value.
     */
    void add(double v)
    {
      count++;
      l0 += "c", l1 += s_printf(" & %g", v), l2 += " & T.B.D.", l3 += " &  T.B.D.", l4 += " &  T.B.D.", l5 += " &  T.B.D.";
      l6 += " &  T.B.D.";
    }
    /** Saves the table in a latex file.
     * @param filename The latex file name.
     */
    void save(String filename) const
    {
      s_save(filename,
             s_printf("\\begin{tabular}{l|" + l0 + "|}\n" +
                      what + " & \\multicolumn{%d}{c}{" + name + "} \\\\\n" +
                      "\\hline\n" +
                      param + l1 + " \\\\\n" +
                      (param2 != "" ? param2 + l6 + " \\\\\n" : "") +
                      "Number of Iterations" + l2 + " \\\\\n" +
                      "Minimal criterion value" + l3 + " \\\\\n" +
                      (with_decay_and_bias ?
                       "Exponential decay time" + l4 + " \\\\\n" +
                       "Final bias interpolation" + l5 + " \\\\\n" : "") +
                      "\\hline\n\\end{tabular}\n", count));
    }
  };

  /** Generates a reverse engineering input/output sequence.
   * @param type The nodes type.
   * @param N The number of state units.
   * @param seed Specifies if the pseudo-random sequence is reproducible (using same value at each call) or not.
   */
  void set_reverse_engineering_inoutput(String type, unsigned int N, unsigned int seed = 0)
  {
    unsigned int T = (N * (N + 1)) * 2;
    delete input;
    input = new network::BufferedInput("normal", 1, T, seed);
    network::KernelTransform *root = newKernelTransform("ReverseEngineering", type, N, *input, true);
    delete output;
    output = new network::BufferedInput(*root, 1);
    delete root;
    header = s_printf(" {\n   'estimation' : 'reverse-engineering',\n   'network-type' : '" + type + "',\n   'network-size' : %d,\n   'sample-length' : %d,\n   'input' : " + ((network::BufferedInput) *input).getHistogram().asString("min max mean stdev", true) + ",\n   'output' : " + ((network::BufferedInput) *output).getHistogram().asString("min max mean stdev", true) + "\n }", N, T);
  }
  /** Generates a noisy LinearNonLinearTransform reverse engineering input/output sequence.
   * @param N The number of state units.
   * @param noiseProbability The probability to add noise to a given value.
   * @param noiseStandardDeviation The standard deviation of the normal added noise.
   */
  void set_noisy_inoutput(unsigned int N, double noiseProbability, double noiseStandardDeviation)
  {
    unsigned int T = (N * (N + 1)) * 2;
    delete input;
    input = new network::BufferedInput("normal", 1, T, true);
    network::KernelTransform *root = newKernelTransform("ReverseEngineering", "LinearNonLinearTransform", N, *input, true);
    delete output;
    network::BufferedInput output0(*root, 1);
    output = new network::BufferedInput(output0, "noise", noiseProbability, noiseStandardDeviation);
    delete root;
    header = s_printf(" {\n   'estimation' : 'noisy-reverse-engineering',\n   'network-type' : 'LinearNonLinearTransform',\n   'network-size' : %d,\n   'sample-length' : %d,\n   'noiseProbability' : %g ,\n   'noiseStandardDeviation' : %g ,\n   'input' : " + ((network::BufferedInput) *input).getHistogram().asString("min max mean stdev", true) + ",\n   'output' : " + ((network::BufferedInput) *output).getHistogram().asString("min max mean stdev", true) + "\n }", N, T, noiseProbability, noiseStandardDeviation);
  }
  /** Generates an output sequence, without input.
   * @param name The related sequence name.
   * @param type The related nodes type.
   * @param N The number of state units.
   * @param T The number of samples.
   */
  void set_sequence_inoutput(String name, String type, unsigned int N, unsigned int T)
  {
    delete input;
    input = new network::Input(0, T);
    delete output;
    output = new network::BufferedInput(name, 1, name == "zerone" ? 5 *T : T, T);
    header = s_printf(" {\n   'estimation' : 'sequence-reproduction',\n   'network-type' : '" + type + "',\n   'network-size' : %d,\n   'sample-length' : %d,\n   'output' : " + ((network::BufferedInput) *output).getHistogram().asString("min max mean stdev", true) + "\n }", N, T);
  }
  /** Performs a supervised estimation, if the related file does not exist.
   * @param test The test to perform: "reverse_engineering", "sequence_generation".
   * @param type The nodes type: "LinearTransform", "LinearNonLinearTransform", "SparseNonLinearTransform", "SoftMaxTransform", "IntegrateAndFireTransform"
   * @param criterion The used criterion : '2', '1', '0', 'a', 'b', 'h'.
   * @param N The number of state units.
   * @param mode The input generation mode : "reuse" of previous estimation weights, "reproducible", or not.
   * @param criterion_epsilon The precision on the state values.
   * @param maxIterations The maximal number of iteration.
   * @param reinject If true, uses the desired value in the 2nd order criterion estimation.
   */
  void do_supervised_estimation(String test, String type, char criterion, unsigned int N, String mode = "reproducible", double criterion_epsilon = 1e-6, unsigned int maxIterations = 100, bool reinject = true)
  {
    std::string filename = test == "" ? "" : s_printf("tex/results/" + test + "_" + type + "_N=%d", N);
    printf("> doing %s\n", filename == "" ? "run" : filename.c_str());
    {
      network::KernelTransform *transform0 = newKernelTransform(test, type, N, *input, false);
      transform0->setWeightsRandom(-0.1, 0.1 / sqrt(N), false, "normal", mode == "reproducible" || mode == "reuse" ? 0 : ++seed);
      if((mode == "reuse") && (transform != NULL))
        transform0->setWeights(*transform);
      delete transform;
      transform = transform0;
    }
    network::KernelSupervisedEstimator estimator(*transform, *output, criterion, 1e-1, reinject);
    estimator.run(criterion_epsilon, 1e-4, maxIterations, verbose_but_nosave ? "stdout" : filename, header);
    fit = estimator.getFit();
  }
  /** Resets before weights reuse. */
  void initReuse()
  {
    delete transform;
    transform = NULL;
  }
  /** Runs the {reverse-engineering} experiments.
   * @param type The related node type.
   */
  void run_reverse_engineering(String type = "")
  {
    const char *types[] = { "LinearNonLinearTransform", "IntegrateAndFireTransform", "LinearTransform", "SparseNonLinearTransform", "SoftMaxTransform" };
    if(type == "")
      for(unsigned int k = 0; k < 5; k++)
        run_reverse_engineering(types[k]);
    else {
      std::string filename = s_printf("tex/results/reverse_engineering_" + type + ".tex");
      if(file_tobedone(filename)) {
        ResultTable result("Number of units", "", "Node type", type);
        unsigned int N0 = 16, N2 = 16, N1 = 128;  // @todo
        for(unsigned int N = N0; N <= N1; N *= 2) {
          set_reverse_engineering_inoutput(type, N);
          do_supervised_estimation("reverse_engineering", type, '2', N, "reproducible", N < N2 ? 1e-6 : 1e-4, 100, true); // @todo
          result.add(N, fit);
          result.save(filename);
        }
      }
    }
  }
  /** Runs the {reverse-engineering-variability} experiment. */
  void run_reverse_engineering_variability()
  {
    std::string filename = s_printf("tex/results/reverse_engineering_variability.tex");
    if(file_tobedone(filename)) {
      ResultTable result("Sample index", "", "Node type", "LinearNonLinearTransform", false);
      for(unsigned int h = 1; h <= 7; h++) {
        set_reverse_engineering_inoutput("LinearNonLinearTransform", 8, ++seed);
        do_supervised_estimation("", "LinearNonLinearTransform", '2', 8, "random", 1e-12);
        result.add(h, fit);
      }
      result.save(filename);
    }
  }
  /** Runs the {robust-criterion} experiment.
   * @param what The experiment name : "noise" or "outiers" or both.
   */
  void run_robust_criterion(String what = "")
  {
    const char *criteria[] = { "2", "1", "0", "a", "b" };
    const unsigned int N = 4;
    if((what == "") || (what == "noise")) {
      std::string filename = s_printf("tex/results/robust_criterion_with_noise.tex");
      if(file_tobedone(filename)) {
        ResultTable result("Criterion", "", "", "Robustness to noise", false);
        const double noiseProbability = 1, noiseStandardDeviation = 0.02;
        for(unsigned int l = 0; l < 5; l++) {
          set_noisy_inoutput(N, noiseProbability, noiseStandardDeviation);
          do_supervised_estimation(s_printf("robust_criterion_with_noise_criterion=%s", criteria[l]), "LinearNonLinearTransform", criteria[l][0], N, "reproducible", 1e-4, 100, false);
          result.add(criteria[l], fit);
        }
        result.save(filename);
      }
    }
    if((what == "") || (what == "outliers")) {
      std::string filename = s_printf("tex/results/robust_criterion_with_outliers.tex");
      if(file_tobedone(filename)) {
        ResultTable result("Criterion", "", "", "Robustness to outliers", false);
        const double noiseProbability = 0.1, noiseStandardDeviation = 1;
        for(unsigned int l = 0; l < 5; l++) {
          set_noisy_inoutput(N, noiseProbability, noiseStandardDeviation);
          do_supervised_estimation(s_printf("robust_criterion_with_outliers_criterion=%s", criteria[l]), "LinearNonLinearTransform", criteria[l][0], N, "reproducible", 1e-4, 100, false);
          result.add(criteria[l], fit);
        }
        result.save(filename);
      }
    }
  }
  /** Runs a sequence generation experiment. */
  unsigned int run_sequence_generation(String name, String type, char criterion, unsigned int N, unsigned int T)
  {
    set_sequence_inoutput(name, type, N, T);
    do_supervised_estimation(s_printf("sequence_generation_criterion=%c_T=%d", criterion, T), type, criterion, N, "reuse", 1e-6, 200);
    // Returns a string view of the transform a posteriori Hamming distance and related binary errors.
    {
      std::string s;
      network::KernelSupervisedEstimator error(*transform, *output, 'h', 0);
      unsigned int count0 = 0, count1 = 0;
      for(unsigned int t = 0; t < T; t++) {
        double v = (output->get(0, t) - 0.5) * (transform->get(0, t) - 0.5);
        if(v < 0) {
          if(output->get(0, t) < 0.5)
            count0++;
          else
            count1++;
        }
        s += s_printf("%s%.0f,%.2f#%c%s", t == 0 ? "> |" : "|", output->get(0, t), transform->get(0, t),
                      v < 0 ? '-' : '+', t < T - 1 ? "" : s_printf("| error = (>0 = %d | <1 = %d)= %d / %d = %.0f%% === %0.f%%\n",
                                                                   count0, count1, (count0 + count1), T, 100.0 * (count0 + count1) / T,
                                                                   100 * error.network::KernelEstimator::rho()).c_str());
      }
      std::string filename = s_printf("tex/results/sequence_generation/sequence_errors_%s_criterion=%c_N=%d_T=%d.txt", type.c_str(), criterion, transform->getN(), T);
      // - printf("> %s\n", s.c_str());
      s_save(filename, s);
      return count0 + count1;
    }
  }
  /** Runs a sequence generation experiment. */
  void run_sequence_generation(String name, String type, char criterion)
  {
    std::string filename = s_printf("tex/results/sequence_generation_%s_%s_criterion=%c.tex", name.c_str(), type.c_str(), criterion);
    if(file_tobedone(filename)) {
      ResultTable result("Number of units", "Sequence length", "Node type", type, false);
      for(unsigned int N = 2; N <= 8; N++) {
        ExponentialDecayFit fit1;
        unsigned int T = 0, Tmin = N == 2 ? 2 : N / 2, Tmax = 1 + 2 * N * N;
        initReuse();
        for(T = Tmin; T <= Tmax; T++) {
          unsigned int error = run_sequence_generation(name, type, criterion, N, T);
          if(error > 0)
            break;
          else
            fit1 = fit;
        }
        if(T > Tmin)
          result.add2(N, T == Tmax ? 999 : T - 1, fit1);
        else
          result.add(N);
      }
      result.save(filename);
    }
  }
  /** Runs the {sequence-generation} experiment. */
  void run_sequence_generation()
  {
    const char *names[] = { "zerone", "ramp", "sierpinski" };
    const char *types[] = { "LinearNonLinearTransform", "IntegrateAndFireTransform" };
    const char *criteria[] = { "h", "2" };
    for(unsigned int n = 0; n < 1; n++)
      for(unsigned int k = 0; k < 1; k++)
        for(unsigned int l = 0; l < 1; l++)
          run_sequence_generation(names[n], types[k], criteria[l][0]);
  }
  /** Runs all experiments. */
  void run(Struct what = "{}")
  {
    printf(">  Experimenting %s ... \n", ((String) what).c_str());
    if(what.get("what") == "todo") {
      verbose_but_nosave = true;
      run_sequence_generation("zerone", "LinearNonLinearTransform", 'h', 4, 4);
    } else if(what.get("what") == "all") {
      run_reverse_engineering();
      run_reverse_engineering_variability();
      run_robust_criterion();
      run_sequence_generation();
    } else if(what.get("what") == "reverse_engineering")
      run_reverse_engineering(what.get("type"));
    else if(what.get("what") == "sequence_generation")
      run_sequence_generation(what.get("name", "zerone"), what.get("type", "LinearNonLinearTransform"), (char) what.get("criterion", "h"));
    else {
      Struct tasks;
      tasks.reset((String) "[" +
                  "{what: reverse_engineering, type: LinearNonLinearTransform}," +
                  "{what: reverse_engineering, type: IntegrateAndFireTransform}," +
                  "{what: reverse_engineering, type: LinearTransform}," +
                  "{what: reverse_engineering, type: SparseNonLinearTransform}," +
                  "{what: reverse_engineering, type: SoftMaxTransform}," +
                  "{what: sequence_generation, name: zerone, criterion: h}," +
                  "{what: sequence_generation, name: zerone, criterion: 2}," +
                  "{what: sequence_generation, name: ramp, criterion: h}," +
                  "{what: sequence_generation, name: ramp, criterion: 2}," +
                  "]");
      int i = what;
      if((i != Struct::INT_NAN) && (0 <= i) && (i < tasks.getLength()))
        run(tasks.get(i));
      else
        assume(false, " illegal-argument", "in Experimentation::run nothing to do");
    }
    printf(">  ... experiment done.\n");
  }
};
