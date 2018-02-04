///@cond INTERNAL

void CurveFit_test()
{
  CurveFit fit;
  for(double t = 0; t < 100; t++)
    fit.add(1.2345);
  assume(std::isnan(fit.getDecay()) && std::isnan(fit.getGain()) && fabs(fit.getBias() - 1.2345) < 1e-10, "illegal-state", "in CurveFit_test 1/3 wrong estimation : %s\n", fit.asString().c_str());
  fit.clear();
  for(double t = 0; t < 100; t++)
    fit.add(1.2345 + 0.012345 * (t - 99));
  assume(std::isnan(fit.getDecay()) && fabs(fit.getGain() - 0.012345) < 1e-10 && fabs(fit.getBias() - 1.2345) < 1e-10, "illegal-state", "in CurveFit_test 2/3 wrong estimation : %s\n", fit.asString().c_str());
  fit.clear();
  for(double t = 0; t < 100; t++)
    fit.add(2 * exp(-t / 314.16) + 1.2345);
  assume(fabs(fit.getDecay() - 314.16) < 1e-4 && fabs(fit.getBias() - 1.2345) < 1e-6, "illegal-state", "in CurveFit_test 3.1/3 wrong estimation : %s\n", fit.asString().c_str());
  fit.clear();
  for(double t = 0; t < 100; t++)
    fit.add(2 * exp(t / 314.16) + 1.2345);
  assume(fabs(fit.getDecay() + 314.16) < 1e-4 && fabs(fit.getBias() - 1.2345) < 1e-6, "illegal-state", "in CurveFit_test 3.2/3 wrong estimation : %s\n", fit.asString().c_str());
  fit.clear();
  for(double t = 0; t < 100; t++)
    fit.add(-2 * exp(t / 314.16) + 1.2345);
  assume(fabs(fit.getDecay() + 314.16) < 1e-4 && fabs(fit.getBias() - 1.2345) < 1e-6, "illegal-state", "in CurveFit_test 3.3/3 wrong estimation : %s\n", fit.asString().c_str());
  // -fit.show("/tmp/fit", true);
  exit(0);
}
///@endcond
