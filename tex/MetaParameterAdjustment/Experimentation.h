///@cond INTERNAL

#include <vector>
#include <map>

/** Generates the experimental results of the next part of the work on backward-tuning. */
class Experimentation3 {
public:
  Experimentation3() {}
  ~Experimentation3() {}
  /** Runs these experiments. */
  void run(Struct what = "{}")
  {
    printf(">  Experimenting MetaParameterAdjustment (%s) ... \n", ((String) what).c_str());
    // Tests on Lyapounov exponent
    if(false) {
      static const unsigned int M = 1, N = 16, T = 100;
      network::BufferedInput input("normal", M, T, true);
      network::SparseNonLinearTransform transform(N, input);
      std::vector < Histogram > values, lyapounovs;
      double w0 = 0, w1 = 10;
      for(double w = w0; w <= w1; w += 0.1 * (w1 - w0)) {
        transform.setWeightsRandom(0, w / N, false, "normal", 0);
        lyapounovs.push_back(transform.getLyapunovExponent());
        network::BufferedInput output(transform);
        values.push_back(output.getHistogram());
      }
      Histogram::plot("stdout", lyapounovs, w0, w1, true);
      Histogram::plot("stdout", values, w0, w1, true);
    }
    // Tests ParameterOptimizer
    if(false) {
      ParameterOptimizer o("{'mode'  : ['true', 'false'], 'scale' : [0.1, 0.2, 0.5, 1, 2, 5, 10]}");
      printf(">>\n%s\n", o.asString().c_str());
    }
    printf(">  ... experiment done.\n");
  }
};

///@endcond
