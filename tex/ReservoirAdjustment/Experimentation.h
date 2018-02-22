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
    run_once(false);
    run_once(true);
    printf(">  ... experiment done.\n");
  }
  void run_once(bool with = true)
  {
    network::BufferedInput data("tex/ReservoirAdjustment/data/chaotic-sequence-anthony", "csv");
    network::BufferedInput input1(data, 1, data.getT()/2), output1(data, 1, data.getT()/2);
    static const unsigned int N = 16;
    network::SparseNonLinearTransform transform1(N, input1);
    transform1.setWeightsRandom(0, 0.5 / N, false, "normal", 0);
    network::SupervisedCriterion criterion1(transform1, output1, '2', 1, 'n');
    network::KernelDistributedEstimator estimator(transform1, criterion1);
    estimator.updateReadout(1);
    if (with)
      estimator.run(1e-6, 100, 0, 10, true);
    network::BufferedInput input2(data, 1, data.getT()/2, data.getT()/2), output2(data, 1, data.getT()/2, data.getT()/2);
    network::SparseNonLinearTransform transform2(N, input2);
    transform2.setWeights(transform1);
    network::SupervisedCriterion criterion2(transform2, output2, '2', 1, 'n');
    printf("err with %d>%g\n", with, criterion2.TransformCriterion::rho());
  }
};

///@endcond
