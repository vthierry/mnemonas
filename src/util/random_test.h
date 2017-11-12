///@cond INTERNAL

void random_test()
{
  // Tests the seed reproductibility
  {
    static const unsigned int T = 1000;
    double samples[2][T];
    for(unsigned k = 0; k < 2; k++) {
      random::setSeed(0);
      for(unsigned t = 0; t < T; t++)
        samples[k][t] = random::gaussian();
    }
    for(unsigned t = 0; t < T; t++)
      assume(samples[0][t] == samples[1][t], "illegal-state", "in random_test seed error");
  }
  // Tests distribution sampling precision
  static const unsigned int L = 1000000, l = (int) sqrt(L);
  {
    // Uniform distribution in [-1, 1]
    Histogram histogram(l, -1, 1);
    for(unsigned int i = 0; i < L; i++)
      histogram.add(-1 + 2 * random::uniform());
    assume(fabs(histogram.get("mean")) < 1e-3 &&
           fabs(histogram.get("stdev") - 2 * pow(12, -0.5)) < 1e-3 &&
           fabs(histogram.get("skew")) < 1e-3 &&
           fabs(histogram.get("kurt") + 6.0 / 5.0) < 3e-3 &&
           fabs(histogram.get("entropy") - histogram.get("uniform-entropy")) < 1e-3 &&
           fabs(histogram.get("uniform-divergence")) < 1e-3 &&
           histogram.get("best-model") == 0,
           "illegal-state",
           "in random_test/uniform spurious numbers: %s\n", histogram.asString().c_str());
    // - printf("Uniform : %s\n", histogram.asString().c_str());
    // - histogram.plot("/tmp/uniform", "automatic", true);
  }

  {
    Histogram histogram(l, -10, 10);
    for(unsigned int i = 0; i < L; i++)
      histogram.add(random::gaussian());
    assume(fabs(histogram.get("mean")) < 2e-3 &&
           fabs(histogram.get("stdev") - 1) < 2e-3 &&
           fabs(histogram.get("skew")) < 3e-3 &&
           fabs(histogram.get("kurt")) < 1e-2 &&
           fabs(histogram.get("entropy") - histogram.get("gaussian-entropy")) < 2e-3 &&
           fabs(histogram.get("gaussian-divergence")) < 1e-3 &&
           histogram.get("best-model") == 1,
           "illegal-state",
           "in random_test/gaussian/1 spurious numbers: %s\n", histogram.asString().c_str());
    // - printf("Gaussian : %s\n", histogram.asString().c_str());
    // - histogram.plot("/tmp/gaussian", "automatic", true);
    Histogram histogram2(100, -10, 10);
    for(unsigned int i = 0; i < L; i++)
      histogram2.add(histogram.draw());
    assume(fabs(histogram2.get("mean")) < 2e-2 &&
           fabs(histogram2.get("stdev") - 1) < 2e-2 &&
           fabs(histogram2.get("entropy") - histogram2.get("gaussian-entropy")) < 2e-2 &&
           fabs(histogram2.get("gaussian-divergence")) < 1e-3 &&
           histogram.get("best-model") == 1,
           "illegal-state",
           "in random_test/gaussian/2 spurious numbers: %s\n", histogram2.asString().c_str());
    assume(fabs(histogram2.getDivergence(histogram)) < 2e-3 &&
           fabs(histogram2.getDivergence(histogram, true)) < 2e-3,
           "illegal-state",
           "in random_test/gaussian/2 spurious divergences: div(h2||h1) = %g div(h1||h2)= %g",
           fabs(histogram2.getDivergence(histogram)),
           fabs(histogram2.getDivergence(histogram, true)));
  }
  {
    Histogram histogram(l, 0, 10);
    for(unsigned int i = 0; i < L; i++)
      histogram.add(random::gamma(2));
    assume(fabs(histogram.get("gamma-degree") - 2) < 1e-2 &&
           fabs(histogram.get("gamma-rate") - 1) < 1e-3 &&
           fabs(histogram.get("gamma-divergence")) < 1e-3 &&
           histogram.get("best-model") == 2,
           "illegal-state",
           "in random_test/gamma spurious numbers: %s\n", histogram.asString().c_str());
    // - printf("Gamma : %s\n", histogram.asString().c_str());
    // - histogram.plot("/tmp/gamma", "automatic", true);
  }
}
///@endcond
