#include "main.hpp"

#include <stdio.h>
#include <string.h>

#include "inc/s_printf.h"
#include "inc/gnuplot.h"

network::BufferedInput::BufferedInput(const BufferedInput& input) : Input(input.getN(), input.getT()), vvalues(NULL), buffered(input.buffered), vectored(input.vectored)
{
  if(buffered) {
    values = new double[input.getN() * (int) input.getT()];
    for(unsigned int nt = 0; nt < N * T; nt++)
      const_cast < double * > (values)[nt] = input.values[nt];
  } else
    values = input.values;
  if(vectored) {}
}
network::BufferedInput::BufferedInput(const double *values, unsigned int N, unsigned int T) : Input(N, T), values(values), vvalues(NULL), buffered(false), vectored(false) {}
network::BufferedInput::BufferedInput(unsigned int N) : Input(N, 0), values(NULL), vvalues(new std::vector < double >[N]), buffered(false), vectored(true) {}
network::BufferedInput::BufferedInput(const Input& input, unsigned int N0) : Input(0 < N0 ? N0 : input.getN(), input.getT()), values(new double[(0 < N0 ? N0 : input.getN()) * (int) input.getT()]), vvalues(NULL), buffered(true), vectored(false)
{
  for(unsigned int t = 0, nt = 0; t < T; t++)
    for(unsigned int n = 0; n < N; n++, nt++)
      const_cast < double * > (values)[nt] = input.get(n, t);
}
network::BufferedInput::BufferedInput(unsigned int N, unsigned int T, double mean, double sigma, String mode, int seed) : Input(N, T), values(new double[N * T]), vvalues(NULL), buffered(true), vectored(false)
{
  random::setSeed(seed);
  for(unsigned int nt = 0; nt < N * T; nt++)
    const_cast < double * > (values)[nt] = mode == "normal" ?
                                           random::gaussian(mean, sigma) :
                                           random::uniform(mean - sqrt(3) * sigma, mean + sqrt(3) * sigma);
}
network::BufferedInput::BufferedInput(String file, String format) : Input(1, 1), vvalues(NULL), buffered(true), vectored(false)
{
  FILE *fp = fopen((file + ".dat").c_str(), "r");
  assume(fp != NULL, "IO-Exception", "in network::BufferedInput unable to open the file %s.dat", file.c_str());
  if(format == "binary-unit-time") {
    fscanf(fp, "%d %lf\n", &N, &T);
    values = new double[N * (int) T];
    fread(const_cast < double * > (values), 8, N * (int) T, fp);
  } else
    assume(false, "illegal-argument", "in network::BufferedInput %s is an unknown file format\n", format.c_str());
  assume(ferror(fp) == 0, "IO-Exception", "in network::BufferedInput error %s when saving in %s.dat", strerror(ferror(fp)), file.c_str());
  fclose(fp);
}
network::BufferedInput::BufferedInput(String name, unsigned int N, unsigned int T) : Input(N, T), values(new double[N * T]), vvalues(NULL), buffered(true), vectored(false)
{
  double *v = const_cast < double * > (values);
  if(name == "sierpinski") {
    unsigned int Cnp[T];
    for(unsigned int t = 0, nt = 0, k = 0, l = 0; t < T; t++, k = k == l ? 0 : k + 1, l = k == 0 ? l + 1 : l) {
      Cnp[t] = t > l && l > k && k > 0 ? Cnp[t - l] + Cnp[t - l - 1] : 1;
      // - printf("%4d %c", Cnp[t], k == l ? '\n' : ' ');
      for(unsigned int n = 0; n < N; n++, nt++)
        v[nt] = (Cnp[t] >> n) % 2 == 0 ? -1 : 1;
    }
  } else if(name == "ramp")
    for(unsigned int t = 0, nt = 0; t < T; t++)
      for(unsigned int n = 0; n < N; n++, nt++)
        v[nt] = t % (T / (n + 1));
  else
    assume(false, "illegal-argument", "in network::BufferedInput %s is an unknown sequence name\n", name.c_str());
}
network::BufferedInput::~BufferedInput()
{
  if(buffered)
    delete[] values;
  if(vectored)
    delete[] vvalues;
}
void network::BufferedInput::save(String file, String format) const
{
  {
    FILE *fp = fopen((file + ".dat").c_str(), "w");
    assume(fp != NULL, "IO-Exception", "in network::ObservedInput::save unable to open %s.dat", file.c_str());
    if(format == "binary-unit-time") {
      fprintf(fp, "%d %.0f\n", N, T);
      if(vectored) {
        double v[1];
        for(unsigned int t = 0; t < (unsigned int) T; t++)
          for(unsigned int n = 0; n < N; n++) {
            v[0] = get(n, t);
            fwrite(v, 8, 1, fp);
          }
      } else
        fwrite(values, 8, N * (int) T, fp);
    } else if(format == "ascii-unit-time")
      for(unsigned int t = 0; t < (unsigned int) T; t++)
        for(unsigned int n = 0; n < N; n++)
          fprintf(fp, "%g%c", get(n, t), n < N - 1 ? ' ' : '\n');
    else if((format == "gnuplot") || (format == "gnushow"))
      for(unsigned int t = 0; t < (unsigned int) T; t++)
        for(unsigned int n = 0; n < N; n++)
          fprintf(fp, "%d %d %g\n", n, t, get(n, t));
    else
      assume(false, "illegal-argument", "in network::ObservedInput::save %s is an unknown format\n", format.c_str());
    assume(ferror(fp) == 0, "IO-Exception", "in network::ObservedInput::save error %s when saving in %s.dat", strerror(ferror(fp)), file.c_str());
    fclose(fp);
  }
  if((format == "gnuplot") || (format == "gnushow")) {
    std::string plot =
      s_printf("set multiplot layout %d,1 $title\nset xrange [0:*]\nset yrange [*:*]\nset ytics nomirror\n", N);
    for(unsigned int n = 0; n < N; n++)
      plot += s_printf("plot \"" + file + ".dat\" every %d::%d using 2:3 with lines linecolor \"black\" notitle\n", N, n);
    gnuplot(file, plot, format == "gnushow");
  }
}
const Histogram network::BufferedInput::getHistogram(String what_) const
{
  /*
   *  double max = DBL_MAX, min = -max, count = 0;
   *  for(unsigned int t = 0, nt = 0; t < (unsigned int) R; t++)
   *  for(unsigned int n = 0; n < N; n++, nt++)
   *   min = value < min ? values[nt] : min, max = value > max ? values[nt] : max, count++;
   *  Histogram histogram((int) sqrt(count), min, max);
   */
  std::string what = " " + what_ + " ";
  Histogram histogram;
  for(unsigned int n = 0; n < N; n++)
    if((what_ == "") || (what.find(s_printf(" %d ", n)) != std::string::npos))
      for(unsigned int t = 0; t < (unsigned int) T; t++)
        histogram.add(get(n, t));
  return histogram;
}
