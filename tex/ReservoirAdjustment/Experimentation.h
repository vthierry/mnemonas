///@cond INTERNAL

#include <vector>
#include <map>

/** Generates the experimental results of the next part of the work on backward-tuning. */
class Experimentation2 {
public:
  Experimentation2() {}
  ~Experimentation2() {}
  /** Runs these experiments. */
  void run(Struct what = "{}")
  {
    printf(">  Experimenting ReservoirAdjustment (%s) ... \n", ((String) what).c_str());
    // Tests distributed estimation of sparse network
    if(false) {
      static const unsigned int M = 1, N = 16, N0 = 1, T = 1000;
      network::BufferedInput input("normal", M, T, true);
      network::SparseNonLinearTransform transform1(N, input);
      transform1.setWeightsRandom(0, 0.5 / N, false, "normal", 0);
      network::BufferedInput output(transform1, N0);
      network::SparseNonLinearTransform transform2(transform1);
      transform2.setWeights(transform1);
      transform2.setWeightsRandom(0, 0.5 / N, false, "normal", 1);
      network::SupervisedCriterion criterion(transform2, output, '2', 1, 'n');
      network::KernelDistributedEstimator estimator(transform2, criterion);
      estimator.run(1e-6, 100, 0, 10, true);
    }
    network::BufferedInput data("tex/ReservoirAdjustment/data/chaotic-sequence-anthony", "csv");
    printf(">  ... experiment done.\n");
  }
};

///@endcond
