///@cond INTERNAL

/** Generates the experimental results of the next part of the work on backward-tuning. */
class Experimentation2 {
public:
  Experimentation2() {}
  ~Experimentation2() {}
  /** Runs all experiments. */
  void run(Struct what = "{}")
  {
    printf(">  Experimenting2(%s) ... \n", ((String) what).c_str());
    network::BufferedInput data("tex/tests/chaotic-sequence-anthony", "csv");
    printf(">  ... experiment done.\n");
  }
};

///@endcond
