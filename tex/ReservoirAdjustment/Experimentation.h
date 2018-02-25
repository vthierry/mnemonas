///@cond INTERNAL

#include <vector>
#include <map>

/** Generates the experimental results of the next part of the work on backward-tuning. */
class Experimentation2 {
public:
  Experimentation2() {}
  ~Experimentation2() {}
  Histogram ratio1, ratio2;
  /** Runs these experiments. */
  void run(Struct what = "{}")
  {
    printf(">  Experimenting ReservoirAdjustment (%s) ... \n", ((String) what).c_str());
    std::string s = "";
    ratio1.clear(), ratio2.clear();
    for(unsigned int N = 16; N <= 32; N *= 2)
      for(unsigned int i = 50; i <= (N == 16 ? 100 : 50); i *= 2) {
        for(unsigned int k = 10; k < 20; k++)
          s += (s == "" ? "    " : ",\n    ") + run_once(N, i, k);
        s_save("experimenting-reservoir-adjustment.txt", "{\n  data : [\n" + s + "\n],\n  ratio1 : " + ratio1.asString() + "\n  ratio2 : " + ratio2.asString() + "\n}\n");
      }
    printf(">  ... experiment done.\n");
  }
  std::string run_once(const unsigned int N = 16, const unsigned int maxIteration = 50, unsigned int seed = 0)
  {
    // Reading  mackey glass data and cropping the learning set input1/output1 et test set input2/output2
    network::BufferedInput data("tex/ReservoirAdjustment/data/chaotic-sequence-anthony", "csv");
    network::BufferedInput input1(data, 1, 0, data.T / 2), output1(data, 1, 1, data.T / 2);
    network::BufferedInput input2(data, 1, 0, data.T / 2, data.T / 2), output2(data, 1, 1, data.T / 2, data.T / 2);
    // Defining a NLN network with sparse connectivity
    network::SparseNonLinearTransform transform(N, input2);
    // with a learning cirterion and estimator
    network::SupervisedCriterion criterion1(transform, output1, '2', 1, 'o');
    network::KernelDistributedEstimator estimator(transform, criterion1);
    // and test criterion
    network::SupervisedCriterion criterion2(transform, output2, '2', 1, 'n');
    // Random selection of weights
    transform.setWeightsRandom(0, 0.5 / N, false, "normal", seed);
    // Initial error
    transform.setInput(input1);
    double c0 = criterion1.TransformCriterion::rho();
    transform.setInput(input2);
    double err0 = criterion2.TransformCriterion::rho();
    // Readout optimization and related test error
    transform.setInput(input1);
    double c1 = estimator.updateReadout(1);
    transform.setInput(input2);
    double err1 = criterion2.TransformCriterion::rho();
    // Recurrent weight adjustment and related test error
    transform.setInput(input1);
    double c2 = estimator.run(1e-6, maxIteration, 0, 10, true);
    transform.setInput(input2);
    double err2 = criterion2.TransformCriterion::rho();
    // Returning the result
    // { N = number-of-units, K = number-of-iteration, c = [initial-critrion, readout-adjustment-criterion, recurrent-weights-adjustment-errorcriterion], err = [initial-error, readout-adjustment-error, recurrent-weights-adjustment-error], ratio = [readout-adjustment-error / initial-error, recurrent-weights-adjustment-error / readout-adjustment-error]}
    double rerr1 = err1 / err0, rerr2 = err2 / err1;
    if(rerr2 < 1)
      ratio1.add(rerr1), ratio2.add(rerr2);
    std::string s = s_printf("{ 'N' = %d, 'K' = %d, 'c' = [%g, %g, %g], 'err' = [%g, %g, %g], 'ratio' = [%g, %g]}", N, maxIteration, c0, c1, c2, err0, err1, err2, rerr1, rerr2);
    printf("> %s\n\n", s.c_str());
    return s;
  }
};

///@endcond
