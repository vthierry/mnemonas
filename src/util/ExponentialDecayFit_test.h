///@cond INTERNAL

void ExponentialDecayFit_test()
{
  ExponentialDecayFit fit;
  for(double t = 0; t < 100; t++)
    fit.add(exp(-t / 314.16) + 1.2345);
  assume(fabs(fit.getDecay() - 314.16) < 1e-8 && fabs(fit.getBias() - 1.2345) < 1e-10, "illegal-state", "in ExponentialDecayFit_test wrong estimation : %s\n", fit.asString().c_str());
  // - fit.show("/tmp/fit", true);
}
///@endcond
