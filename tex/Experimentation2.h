///@cond INTERNAL

/** Generates the experimental results of the next part of the work on backward-tuning. */
class Experimentation2 {
public:
  Experimentation2() {}
  ~Experimentation2() {}
  /** Runs these experiments. */
  void run(Struct what = "{}")
  {
    printf(">  Experimenting2(%s) ... \n", ((String) what).c_str());
    // Tests distributed estimation of sparse network
    if(true) {
      static const unsigned int M = 1, N = 16, N0 = 1, T = 1000;
      network::BufferedInput input("normal", M, T, true);
      network::SparseNonLinearTransform transform1(N, input);
      transform1.setWeightsRandom(0, 0.5 / N, false, "normal", 0);
      network::BufferedInput output(transform1, N0);
      network::SparseNonLinearTransform transform2(transform1);
      transform2.setWeights(transform1);
      transform2.setWeightsRandom(0, 0.5 / N, false, "normal", 1);
      network::TransformSupervisedCriterion criterion(transform2, output, '2', 1, false);
      network::KernelDistributedEstimator estimator(transform2, criterion);
      estimator.run(1e-6, 100, 0, 10, true);
    }
    // Tests ParameterOptimizer
    if(false) {
      ParameterOptimizer o("{'mode'  : ['true', 'false'], 'scale' : [0.1, 0.2, 0.5, 1, 2, 5, 10]}");
      printf(">>\n%s\n", o.asString().c_str());
    }
    network::BufferedInput data("tex/tests/chaotic-sequence-anthony", "csv");
    printf(">  ... experiment done.\n");
  }
};

///@endcond
