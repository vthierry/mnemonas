///@cond INTERNAL

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
          assume(load.get(n, t) == save.get(n, t), "illegal-state", "in network_test/BufferedInput binary save-load error");
      system(("/bin/rm -f " + file + ".dat").c_str());
    }
    // - save.save(file, "gnushow");
    // Tests the csv load/save
    {
      std::string values = "1,2,3\n1,4,9\n1,8,27\n1,16,81\n";
      s_save("/tmp/test-buffered-input.csv", values);
      network::BufferedInput data("/tmp/test-buffered-input", "csv");
      data.save("/tmp/test-buffered-output", "csv");
      std::string result = s_load("/tmp/test-buffered-output.csv");
      assume(values == result, "illegal-state", "in network_test/BufferedInput csv save-load error");
    }
  }
  // Tests the recurrent transform mechanism
  {
    static const unsigned int N = 2, T = 100;
    network::BufferedInput input("normal", 1, T);
    network::LinearTransform transform(N, input);
    transform.setWeight(0, 4, 1), transform.setWeight(1, 2, 1);
    network::BufferedInput output(transform);
    for(unsigned int t = 1; t < T; t++)
      assume(output.get(0, t - 1) == output.get(1, t), "illegal-state", "in network_test/RecurrentTransform recurrent error");
    network::BufferedInput output2(transform);
    for(unsigned int t = 0; t < T; t++)
      assume(output.get(0, t) == output2.get(0, t), "illegal-state", "in network_test/RecurrentTransform reproducibility error");
  }
  // Tests the recurrent weight estimation mechanism
  {
    static const unsigned int M = 1, N = 2, N0 = 2, T = 10;
    network::BufferedInput input("normal", M, T, true);
    network::LinearTransform transform1(N, input);
    transform1.setWeight(0, 2, 0.1), transform1.setWeight(0, 3, 0.3), transform1.setWeight(0, 4, 0.0);
    transform1.setWeight(1, 2, -.2), transform1.setWeight(1, 3, 0.4), transform1.setWeight(1, 4, 0.5);
    network::BufferedInput output(transform1, N0);
    network::LinearTransform transform2(transform1);
    transform2.setWeightsRandom(0, 0.05 / N, true, "normal", 0);
    network::SupervisedCriterion criterion(transform2, output);
    network::KernelExperimentalEstimator estimator(transform2, criterion);
    double err = estimator.run(1e-12, 1e-4, 100, "");
    assume(err < 1e-12, "illegal-state", "in network_test/KernelSupervisedEstimation over-threshold error = %g\n", err);
  }
  // Tests the recurrent weight estimation mechanism with distributed estimator
  {
    static const unsigned int M = 1, N = 2, N0 = 2, T = 10;
    network::BufferedInput input("normal", M, T, true);
    network::LinearTransform transform1(N, input);
    transform1.setWeight(0, 2, 0.1), transform1.setWeight(0, 3, 0.3), transform1.setWeight(0, 4, 0.0);
    transform1.setWeight(1, 2, -.2), transform1.setWeight(1, 3, 0.4), transform1.setWeight(1, 4, 0.5);
    network::BufferedInput output(transform1, N0);
    network::LinearTransform transform2(transform1);
    transform2.setWeightsRandom(0, 0.05 / N, true, "normal", 0);
    network::SupervisedCriterion criterion(transform2, output);
    network::KernelDistributedEstimator estimator(transform2, criterion);
    double err = estimator.run();
    assume(err < 1e-6, "illegal-state", "in network_test/KernelSupervisedEstimation over-threshold error = %g\n", err);
  }
  // Tests the different model basic mechanism
  {
    class Test {
      static network::KernelTransform *newKernelTransform(String type, unsigned int N, const network::Input& input)
      {
        network::KernelTransform *network = network::KernelTransform::newKernelTransform(type, N, input);
        if(type == "LinearNonLinearTransform")
          ((network::LinearNonLinearTransform *) network)->setLeak(NAN);
        else if(type == "SparseNonLinearTransform")
          ((network::SparseNonLinearTransform *) network)->setConnections(N / 2, 0);
        else if(type == "IntegrateAndFireTransform")
          ((network::IntegrateAndFireTransform *) network)->setSharpness(NAN);
        return network;
      }
      static void testDerivatives(String type, network::KernelTransform& transform)
      {
        unsigned int N = transform.getN(), R = transform.getR(), T = transform.getT();
        transform.reset(true);
	double *connected = new double[N*N];
	for(unsigned int nn = 0; nn < N * N; nn++)
	  connected[nn] = 0;
        for(unsigned int t = 0; t < T; t++)
          for(int n = N - 1; 0 <= n; n--) {
            transform.get(n, t);
            if(t > 0) {
              for(int t_ = t; 0 <= t_ && (int) t <= t_ + (int) R; t_--)
                for(unsigned int n_ = ((int) t) == t_ ? n + 1 : 0; n_ < N; n_++) {
                  double d0 = transform.getValueDerivative(n, t, n_, t_);
		  connected[n_ + n * N] += fabs(d0);
                  double d1 = transform.getValueDerivativeApproximation(n, t, n_, t_);
                  assume(fabs(d0 - d1) <= 1e-2 * (1 < fabs(d0) + fabs(d1) ? fabs(d0) + fabs(d1) : 1),
                         "numerical-error", "in network_test/testDerivatives (%s) transform.getValueDerivative(%d, %d; %d, %d) : (d_analytic = %g) != (d_numeric = %g) N = %d T = %d get(n,t) = %g get(n_, t_) = %g\n", type.c_str(), n, t, n_, t_, d0, d1, N, T, transform.getValue(n, t), transform.getValue(n_, t_));
                }
            }
          }
	/** Tests the connectivity. */
	{
	  unsigned int ok_connected = 0, notok_connected = 0, notok_unconnected = 0;
	  for(unsigned int n = 0; n < N; n++)
	    for(unsigned int n_ = 0; n_ < N; n_++) {
	      ok_connected += transform.isConnected(n, n_) ? 1 : 0;
	      notok_connected += transform.isConnected(n, n_) && 0 == connected[n_ + n * N];
	      notok_unconnected += (!transform.isConnected(n, n_)) && 0 < connected[n_ + n * N];
	      //-assume(!(transform.isConnected(n, n_) && 0 == connected[n_ + n * N]), " numerical-error", "in network_test/testDerivatives (%s) transform.isConnected(n=%d, n_=%d) but no derivative", type.c_str(), n, n_);
	    }
	  assume(notok_unconnected == 0,  "numerical-error", "in network_test/testDerivatives (%s) #%d spurious connection not given by transform.isConnected()", type.c_str(), notok_unconnected);
	  assume(notok_connected <= ok_connected / N,  "numerical-error", "in network_test/testDerivatives (%s) #%d<#%d empty connection while given bytransform.isConnected()", type.c_str(), notok_connected, ok_connected);
	}
	delete[] connected;
      }
      static void testReverseEngineering(String type, unsigned int N = 2)
      {
        static const unsigned int M = 1, N0 = N;
        unsigned int T = (N * (N + 1)) * 4;
        network::BufferedInput input("normal", M, T, true);
        network::KernelTransform *transform1 = newKernelTransform(type, N, input);
        transform1->setWeightsRandom(0, 0.5 / N, false, "normal", 0);
        testDerivatives(type, *transform1);
        network::BufferedInput output(*transform1, N0);
        network::KernelTransform *transform2 = newKernelTransform(type, N, input);
        transform2->setWeightsRandom(0, 0.5 / N, false, "normal", 1);
        network::SupervisedCriterion criterion(*transform2, output, '2', 1, 'n');
        network::KernelExperimentalEstimator estimator(*transform2, criterion);
        double err = estimator.run(1e-3, 1e-4, 100, "");
        assume(err < 1e-3, "illegal-state", "in network_test/testReverseEngineering for the model '%s' over-threshold error = %g\n", type.c_str(), err);
        delete transform1;
        delete transform2;
      }
      static void testReadout(unsigned int N = 2)
      {
        static const unsigned int M = 1, N0 = N;
        unsigned int T = (N * (N + 1)) * 4;
        network::BufferedInput input("normal", M, T, true);
        network::KernelTransform *transform1 = newKernelTransform("SparseNonLinearTransform", N, input);
        transform1->setWeightsRandom(0, 0.5 / N, false, "normal", 0);
        network::BufferedInput output(*transform1, N0);
        network::KernelTransform *transform2 = newKernelTransform("SparseNonLinearTransform", N, input);
        transform2->setWeightsRandom(0, 0.5 / N, false, "normal", 1);
        network::SupervisedCriterion criterion(*transform2, output, '2', 1, 'n');
        network::KernelEstimator estimator(*transform2, criterion);
        double err = estimator.updateReadout(N0);
	assume(err < 1e-3, "illegal-state", "in network_test/testReadout for the model over-threshold error = %g\n", err);
        delete transform1;
        delete transform2;
	exit(0);
      }
public:
      static void run()
      {
        std::vector < std::string > types = { "LinearTransform", "LinearNonLinearTransform", "SparseNonLinearTransform", "SoftMaxTransform", "IntegrateAndFireTransform" };
        for(std::vector < std::string > ::const_iterator i = types.begin(); i != types.end(); ++i)
          testReverseEngineering(*i);
	testReadout();
      }
    }
    test;
    test.run();
  }
  // Tests the different supervised criterion mechanism
  {
    class Test {
      static void testCriteriaReverseEngineering(char type, unsigned int N = 2)
      {
        static const unsigned int M = 1, N0 = N;
        unsigned int T = (N * (N + 1)) * 2;
        network::BufferedInput input("normal", M, T, true);
        network::LinearNonLinearTransform transform1(N, input, 0, 1);
        transform1.setWeightsRandom(0, 0.5 / N, false, "normal", 0);
        network::BufferedInput output(transform1, N0);
        network::LinearNonLinearTransform transform2(N, input, 0, 1);
        transform2.setWeightsRandom(0, 0.1 / N, false, "normal", 1);
        double nu =
          type == '1' ? 1e-1 :
          type == '0' ? 1e-1 :
          type == 'a' ? 1e-1 :
          type == 'b' ? 1 :
          type == 'h' ? 1e-6 :
          0;
        network::SupervisedCriterion criterion(transform2, output, type, nu);
        network::KernelExperimentalEstimator estimator(transform2, criterion);
        double err0 =
          type == '2' ? 1e-12 :
          type == '1' ? 1e-12 :
          type == '0' ? 1e-12 :
          type == 'a' ? 1e-2 :
          type == 'b' ? 1e-1 :
          type == 'h' ? 1e-12 :
          0;
        double err = estimator.run(1e-12, 1e-4, 10000, "");
        assume(err < err0, "illegal-state", "in network_test/testCriteriaReverseEngineering for the criterion '%c' over-threshold error = %g [< %g]\n", type, err, err0);
      }
public:
      static void testCriteriaReverseEngineering()
      {
        char types[] = { '2', '1', '0', 'a', 'b', 'h' };
        for(unsigned int n = 0; n < 6; n++)
          testCriteriaReverseEngineering(types[n]);
      }
    }
    test;
    test.testCriteriaReverseEngineering();
  }
  // Tests the statistical observable mechanism
  {
    class Test {
public:
      static void testObservables()
      {
        static const unsigned int T = 100000;
        static const char *names[] = { "mean", "icorr", "acorr" };
        for(unsigned int n = 0; n < 3; n++) {
          unsigned int N = names[n][0] == 'i' ? 2 : 1;
          network::BufferedInput output(names[n], N, T, 0.12345);
          std::vector < network::ObservableCriterion::Observable * > observables =
            network::ObservableCriterion::getObservables(names[n], N, /* tau = */ 1);
          network::Input input(0, 0);
          network::KernelTransform transform(0, 0, input);
          network::ObservableCriterion criterion(transform, output, observables);
          double values[5], err = 0;
          if(names[n][0] == 'm')
            values[0] = 0.12345;
          else if(names[n][0] == 'i')
            values[0] = values[1] = 0, values[2] = values[4] = 1, values[3] = 0.12345;
          else if(names[n][0] == 'a')
            values[0] = values[3] = 0, values[1] = 1, values[2] = 0.12345;
          for(unsigned int k = 0; k < observables.size(); k++)
            err += fabs(criterion.getObservableExpectedValue(k) - values[k]);
          assume(err < 1e-2, "illegal-state", "in network_test/testObservables for the observables '%s' over-threshold error = %g\n", names[n], err);
          network::ObservableCriterion::deleteObservables(observables);
        }
      }
      static void testObservablesEstimation()
      {
        static const unsigned int T = 500, N0 = 1, N = 1;
        network::BufferedInput input("normal", 1, T, true);
        network::LinearNonLinearTransform transform(N, input, -2, 2);
        transform.setOffset(NAN).setLeak(NAN).setWeightsRandom(0, 0.5 / N, false, "normal", 1);
        std::vector < network::ObservableCriterion::Observable * > observables =
          network::ObservableCriterion::getObservables("acorr", N0, 1);
        double values[3] = { 0.2345, 0.3456, 0.1234 };
        network::ObservableCriterion criterion(transform, observables, values);
        network::KernelExperimentalEstimator estimator(transform, criterion);
        double err = estimator.run(1e-3, 1e-4, 1000, "");
        assume(err < 1e-3, "illegal-state", "in network_test/testObservablesEstimation over-threshold error = %g\n", err);
        network::ObservableCriterion::deleteObservables(observables);
      }
    }
    test;
    test.testObservables();
    test.testObservablesEstimation();
  }
}
///@endcond
