void network_test()
{
  // Tests the buffered/observed saved/load mechanism
  {
    static const unsigned int N = 2, T = 100;
    static const String file = "/tmp/test";
    // Generates a data file
    network::BufferedInput save(N);
    for(unsigned int t = 0; t < T; t++) {
      double v[2] = { (double) t, (double) ((2 * t) % T) };
      save.add(v);
    }
    save.save(file);
    // Reads the data file and check values
    {
      network::BufferedInput load(file);
      for(unsigned int t = 0; t < T; t++)
        for(unsigned int n = 0; n < N; n++)
          assume(load.get(n, t) == save.get(n, t), "illegal-state", "in network_test/BufferedInput save-load error");
      system(("/bin/rm -f " + file + ".dat").c_str());
    }
    // - save.save(file, "gnushow");
  }
  // Tests the recurrent transform mechanism
  {
    static const unsigned int N = 2, T = 100;
    network::BufferedInput input(1, T);
    network::LinearTransform transform(N, input);
    transform.setWeight(0, 3, 1), transform.setWeight(1, 1, 1);
    network::BufferedInput output(transform);
    for(unsigned int t = 1; t < T; t++)
      assume(output.get(0, t - 1) == output.get(1, t), "illegal-state", "in network_test/RecurrentTransform recurrent error");
    network::BufferedInput output2(transform);
    for(unsigned int t = 0; t < T; t++)
      assume(output.get(0, t) == output2.get(0, t), "illegal-state", "in network_test/RecurrentTransform reproducibility error");
  }
  // Tests an exponential profile
  {
    network::ExponentialDecayFit fit;
    for(double t = 0; t < 100; t++)
      fit.add(exp(-t / 314.16) + 1.2345);
    assume(fabs(fit.getDecay() - 314.16) < 1e-8 && fabs(fit.getBias() - 1.2345) < 1e-10, "illegal-state", "in network_test/ExponentialDecayFit wrong estimation : %s\n", fit.asString().c_str());
    // - fit.show("/tmp/fit", true);
  }
  // Tests the recurrent weight estimation mechanism
  {
    static const unsigned int M = 1, N = 2, N0 = 1, T = 10;
    network::BufferedInput input(M, T, 0, 1, "normal", 0);
    network::LinearTransform transform1(N, input);
    transform1.setWeight(0, 1, 0.1), transform1.setWeight(0, 2, 0.3), transform1.setWeight(0, 3, 0.0);
    transform1.setWeight(1, 1, -.2), transform1.setWeight(1, 2, 0.4), transform1.setWeight(1, 3, 0.5);
    network::BufferedInput output(transform1, N0);
    network::LinearTransform transform2(transform1);
    transform2.setWeightsRandom(0, 0.5 / N, false, "normal", 0);
    network::KernelSupervisedEstimator estimator(transform2, output);
    double err = estimator.run(1e-12, 1e-6, 0, "");
    assume(err < 1e-12, "illegal-state", "in network_test/KernelSupervisedEstimator over-threshold error = %g\n", err);
  }
}
