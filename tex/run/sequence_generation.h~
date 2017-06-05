// #include "inc/s_printf.h"
// #include "inc/s_save.h"

// Evaluates the sequence generation capablity
void sequence_generation()
{
  class SequenceGenerationTest {
public:
    static const network::ExponentialDecayFit& run(String name, unsigned int N, unsigned int T)
    {
      network::Input input(0, 0);
      network::BufferedInput output(name, 1, T);
      network::IntegrateAndFireTransform network(N, input);
      network::KernelSupervisedEstimator estimator(network, output);
      estimator.run(1e-6, 1e-4, 100, s_printf("tex/run/result/sequence_generation-" + name + "-%d-%d", N, T),
                    s_printf(" {\n   'estimation' : 'sequence-generation',\n   'sequence-name' : '" + name + "',\n   'network-size' : %d,\n   'sequence-length' : %d\n }", N, T));
      static network::ExponentialDecayFit fit;
      fit = estimator.getFit();
      return fit;
    }
  }
  test;
  test.run("ramp", 2, 10);
}
