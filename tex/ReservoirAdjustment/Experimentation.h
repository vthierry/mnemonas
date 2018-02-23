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
    network::BufferedInput input1(data, 1, 0, data.T/2), output1(data, 1, 1, data.T/2);
    static const unsigned int N = 16;
    network::SparseNonLinearTransform transform(N, input1);
    transform.setWeightsRandom(0, 0.5 / N, false, "normal", 0);
    network::SupervisedCriterion criterion1(transform, output1, '2', 1, 'o');
    network::KernelDistributedEstimator estimator(transform, criterion1);
    printf("> N0 = %d\n", criterion1.getN0());
    estimator.updateReadout(1);
    if (with)
      estimator.run(1e-6, 100, 0, 10, true);
    network::BufferedInput input2(data, 1, 0, data.T/2, data.T/2), output2(data, 1, 1, data.T/2, data.T/2);
    //@todo ransform.setInput(input2);
    network::SupervisedCriterion criterion2(transform, output2, '2', 1, 'n');
    printf("err with %d>%g\n", with, criterion2.TransformCriterion::rho());
  }
};

///@endcond
