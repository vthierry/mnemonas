#include "inc/s_printf.h"
#include "inc/s_save.h"

// Evaluates the recurrent weigth estimation mechanism precision
void reverse_engineering()
{
  random::setSeed(0);
  class ReverseEngineeringTest {
public:
    static network::KernelTransform *newKernelTransform(String type, unsigned int N, const network::Input& input, bool root)
    {
      network::KernelTransform *network;
      if(type == "LinearTransform")
        network = new network::LinearTransform(N, input);
      else if(type == "LinearNonLinearTransform") {
        network = new network::LinearNonLinearTransform(N, input);
        ((network::LinearNonLinearTransform *) network)->setOffset(0);
      } else if(type == "IntegrateAndFireTransform")
        network = new network::IntegrateAndFireTransform(N, input);
      else if(type == "SoftMaxTransform")
        network = new network::SoftMaxTransform(N, input);
      else
        assume(false, "illegal-argument", "in network_test/newKernelTransform bad network type %s", type.c_str());
      network->setWeightsRandom(0, 0.5 / sqrt(N), false, "normal", root ? 0 : 1);
      return network;
    }
    static const network::ExponentialDecayFit& run(String type, unsigned int N)
    {
      static const unsigned int M = 1, N0 = 1;
      unsigned int T = (N * (N + 1)) * 2;
      network::BufferedInput input(M, T, 0, 1, "normal", 0);
      network::KernelTransform *root = newKernelTransform(type, N, input, true);
      network::BufferedInput output(*root, N0);
      network::KernelTransform *network = newKernelTransform(type, N, input, false);
      network::KernelSupervisedEstimator estimator(*network, output);
      estimator.run(1e-6, 1e-4, 100,
                    s_printf("tex/run/result/reverse_engineering-" + type + "-%d", N),
                    s_printf(" {\n   'estimation' : 'reverse-engineering',\n   'network-type' : '" + type + "',\n   'network-size' : %d,\n   'input' : " + input.getHistogram().asString("min max mean stdev", true) + ",\n   'output' : " + output.getHistogram().asString("min max mean stdev", true) + "\n }", N));
      static network::ExponentialDecayFit fit;
      fit = estimator.getFit();
      delete network;
      delete root;
      return fit;
    }
    static void run(String type, unsigned int N0, unsigned int N1)
    {
      std::string l0, l1, l2, l3, l4, l5;
      for(unsigned int N = N0; N <= N1; N *= 2) {
        const network::ExponentialDecayFit& fit = run(type, N);
        l0 += "c",
        l1 += s_printf(" & %d", N),
        l2 += s_printf(" & %.0f", fit.getCount()),
        l3 += s_printf(" & %.0f", fit.getDecay()),
        l4 += s_printf(" & %.1e", fit.getMinimalValue()),
        l5 += s_printf(" & %.1e", fit.getBias());
      }
      std::string s = "\\begin{tabular}{l|" + l0 + "}\n" +
                      "Number of units" + l1 + " \\\\\n" +
                      "Number of Iterations" + l2 + " \\\\\n" +
                      "Exponential display time" + l3 + " \\\\\n" +
                      "Minimal criterion value" + l4 + " \\\\\n" +
                      "Final bias interpolation" + l5 + " \\\\\n" +
                      "\\end{tabular}\n";
      printf(s.c_str());
      s_save(s_printf("tex/run/result/reverse_engineering-" + type + "-%d-%d.tex", N0, N1), s);
    }
  }
  test;
  test.run("LinearTransform", 2, 16);
  // test.run("LinearNonLinearTransform", 2, 8);
  // test.run("IntegrateAndFireTransform", 2, 8);
  // test.run("SoftMaxTransform", 2, 8);
}
