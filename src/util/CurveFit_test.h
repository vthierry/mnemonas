///@cond INTERNAL

void CurveFit_test()
{
  CurveFit fit;

  /*
   *  for(double t = 0; t < 2; t++)
   *  fit.add(1.2345);
   *  assume(fit.getMode() == 'c' && fabs(fit.getBias() - 1.2345) < 1e-12, "illegal-state", "in CurveFit_test 1.1/3 wrong estimation : %s\n", fit.asString().c_str());
   *  // fit.show("/tmp/fit1", true);
   *  fit.clear();
   *  for(double t = 0; t < 10000; t++)
   *  fit.add(1.2345 + Density::gaussian(0, 0.01));
   *  assume(fit.getMode() == 'c' && fabs(fit.getBias() - 1.2345) < 1e-3, "illegal-state", "in CurveFit_test 1.2/3 wrong estimation : %s\n", fit.asString().c_str());
   *  // fit.show("/tmp/fit1", true);
   *  fit.clear();
   *
   *  for(double t = 0; t < 3; t++)
   *  fit.add(1.2345 + 0.12345 * (t - 2));
   *  // fit.show("/tmp/fit2", true);
   *  assume(fit.getMode() == 'a' && fabs(fit.getGain() - 0.12345) < 1e-12 && fabs(fit.getBias() - 1.2345) < 1e-12, "illegal-state", "in CurveFit_test 2.1/3 wrong estimation : %s\n", fit.asString().c_str());
   *  fit.clear();
   *  for(double t = 0; t < 10000; t++)
   *  fit.add(1.2345 + 0.012345 * (t - 9999) + Density::gaussian(0, 0.001));
   *  assume(fit.getMode() == 'a' && fabs(fit.getGain() - 0.012345) < 1e-2 && fabs(fit.getBias() - 1.2345) < 1e-2, "illegal-state", "in CurveFit_test 2.2/3 wrong estimation : %s\n", fit.asString().c_str());
   *  // fit.show("/tmp/fit2", true);
   *  fit.clear();
   *
   */
  for(double t = 0; t < 4; t++)
    fit.add(2 * exp(-(t - 3) / 3.1416) + 1.2345);
  // fit.show("/tmp/fit3.1", true);
  assume(fit.getMode() == 'e' && fabs(fit.getDecay() - 3.1416) < 1e-4 && fabs(fit.getBias() - 1.2345) < 1e-6, "illegal-state", "in CurveFit_test 3.1/3 wrong estimation : %s\n", fit.asString().c_str());
  fit.clear();
  for(double t = 0; t < 4; t++)
    fit.add(2 * exp((t - 3) / 3.1416) + 1.2345);
  // fit.show("/tmp/fit3.1", true);
  assume(fit.getMode() == 'e' && fabs(fit.getDecay() + 3.1416) < 1e-4 && fabs(fit.getBias() - 1.2345) < 1e-6, "illegal-state", "in CurveFit_test 3.2/3 wrong estimation : %s\n", fit.asString().c_str());
  fit.clear();
  for(double t = 0; t < 100; t++)
    fit.add(2 * exp(-(t - 99) / 31.416) + 1.2345);
  // fit.show("/tmp/fit3.2", true);
  assume(fit.getMode() == 'e' && fabs(fit.getDecay() - 31.416) < 1e-4 && fabs(fit.getBias() - 1.2345) < 1e-4, "illegal-state", "in CurveFit_test 3.3/3 wrong estimation : %s\n", fit.asString().c_str());
  fit.clear();
  for(double t = 0; t < 100; t++)
    fit.add(-2 * exp(-(t - 99) / 31.416) + 1.2345);
  // fit.show("/tmp/fit3.3", true);
  assume(fit.getMode() == 'e' && fabs(fit.getDecay() - 31.416) < 1e-4 && fabs(fit.getBias() - 1.2345) < 1e-4, "illegal-state", "in CurveFit_test 3.4/3 wrong estimation : %s\n", fit.asString().c_str());
}
///@endcond
