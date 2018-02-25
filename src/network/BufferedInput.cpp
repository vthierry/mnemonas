#include "mnemonas.hpp"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "util/gnuplot.h"

///@cond INTERNAL
network::BufferedInput::BufferedInput(const BufferedInput& input) : Input(input.N, input.T), vvalues(NULL), buffered(input.buffered), vectored(input.vectored)
{
  if(buffered) {
    values = new double[N * (int) T];
    for(unsigned int t = 0, nt = 0; t < T; t++)
      for(unsigned int n = 0; n < N; n++, nt++)
        const_cast < double * > (values)[nt] = input.get(n, t);
  } else
    values = input.values;
  if(vectored)
    vvalues = input.vvalues;
}
///@endcond
network::BufferedInput::BufferedInput(const Input& input, unsigned int N0, unsigned int n0, unsigned int T0, unsigned int t0) : Input(0 < N0 ? N0 : input.N, 0 < T0 ? T0 : input.T), vvalues(NULL), buffered(true), vectored(false)
{
  values = new double[N * (int) T];
  for(unsigned int t = 0, nt = 0; t < T; t++)
    for(unsigned int n = 0; n < N; n++, nt++)
      const_cast < double * > (values)[nt] = input.get(n + n0, t + t0);
}
network::BufferedInput::BufferedInput(const double *values, unsigned int N, unsigned int T) : Input(N, T), values(values), vvalues(NULL), buffered(false), vectored(false) {}
network::BufferedInput::BufferedInput(unsigned int N) : Input(N, 0), values(NULL), vvalues(new std::vector < double >[N]), buffered(false), vectored(true) {}
network::BufferedInput::BufferedInput(String file, String format) : Input(1, 1), vvalues(NULL), buffered(true), vectored(false)
{
  unsigned int N = 0;
  double T = 0;
  std::string ext = format == "csv" ? ".csv" : ".dat";
  FILE *fp = fopen((file + ext).c_str(), "r");
  assume(fp != NULL, "IO-Exception", "in network::BufferedInput unable to open %s%s", file.c_str(), ext.c_str());
  if(format == "binary-unit-time") {
    assume(fscanf(fp, "%d %lf\n", &N, &T) == 2, "IO-Exception", "in network::BufferedInput/binary-unit-time spurious header for file when loading %s.dat", file.c_str());
    values = new double[N * (int) T];
    fread(const_cast < double * > (values), 8, N * (int) T, fp);
  } else if(format == "csv") {
    std::vector < double > data;
    unsigned int n = 0;
    T = 0, N = 0;
    for(double v = NAN; fscanf(fp, "%lg", &v) == 1;) {
      char c = '\n';
      fscanf(fp, "%c", &c);
      n++;
      if(c == '\n') {
        if(N == 0)
          N = n;
        else
          assume(N == n, "IO-Exception", "in network::BufferedInput/csv n=%d<=N=%d at t=%d incoherent number of sample", n, N, T);
        n = 0, T++;
      }
      data.push_back(v);
    }
    assume(n == 0, "IO-Exception", "in network::BufferedInput/csv n=%d<=N=%d at t=%d incoherent number of sample on the last line", n, N, T);
    assume(N * T == data.size(), "IO-Exception", "in network::BufferedInput/csv n = %d, uncoherent data sizes N=%d x T= %.0f = %.0f != %d", n, N, T, N * T, data.size());
    values = new double[N * (int) T];
    std::copy(data.begin(), data.end(), const_cast < double * > (values));
  } else
    assume(false, "illegal-argument", "in network::BufferedInput %s is an unknown file format\n", format.c_str());
  assume(ferror(fp) == 0, "IO-Exception", "in network::BufferedInput error %s when loading in %s.dat", strerror(ferror(fp)), file.c_str());
  fclose(fp);
  setN(N), setT(T);
}
network::BufferedInput::BufferedInput(String name, unsigned int N, unsigned int T, ...) : Input(N, T), values(new double[N * T]), vvalues(NULL), buffered(true), vectored(false)
{
  va_list a;
  va_start(a, T);
  double *v = const_cast < double * > (values);
  if(name == "normal") {
    unsigned int seed = va_arg(a, int);
    va_end(a);
    Density::setSeed(seed);
    for(unsigned int nt = 0; nt < N * T; nt++)
      v[nt] = Density::gaussian(0, 1);
  } else if(name == "zerone") {
    unsigned int p = va_arg(a, int);
    va_end(a);
    for(unsigned int t = 0, s = 0, c = 0, nt = 0; t < T; t++) {
      if(t % p == 0) {
        s = c = 0;
        Density::setSeed(0);
      }
      for(unsigned int n = 0; n < N; n++, nt++) {
        v[nt] = s == p - 1 && c == 0 ? 1 : s == p - 1 && c == s ? 0 :
                Density::uniform() > 0.5 ? 1 : 0;
        s++;
        if(v[nt] == 1)
          c++;
      }
    }
  } else if(name == "sierpinski") {
    va_end(a);
    unsigned int Cnp[T];
    for(unsigned int t = 0, nt = 0, k = 0, l = 0; t < T; t++, k = k == l ? 0 : k + 1, l = k == 0 ? l + 1 : l) {
      Cnp[t] = t > l && l > k && k > 0 ? Cnp[t - l] + Cnp[t - l - 1] : 1;
      // - printf("%4d %c", Cnp[t], k == l ? '\n' : ' ');
      for(unsigned int n = 0; n < N; n++, nt++)
        v[nt] = (Cnp[t] >> n) % 2 == 0 ? 1 : 0;
    }
  } else if(name == "ramp") {
    va_end(a);
    for(unsigned int t = 0, nt = 0; t < T; t++)
      for(unsigned int n = 0; n < N; n++, nt++)
        v[nt] = t % (T / (n + 1));
  } else if(name == "mean") {
    double m = va_arg(a, double);
    va_end(a);
    Density::setSeed(0);
    for(unsigned int nt = 0; nt < N * T; nt++)
      v[nt] = m + Density::gaussian(0, 0.1 * m);
  } else if(name == "icorr") {
    double c = va_arg(a, double);
    assume(-1 <= c && c <= 1, "illegal-argument", "in network::BufferInput::BufferInput(\"acorr\",...) we must have acorr=%g in [-1,1]", c);
    assume(1 < N, "illegal-argument", "in network::BufferInput::BufferInput(\"acorr\",...) the number of units N=%d must be at least 2", N);
    va_end(a);
    Density::setSeed(0);
    for(unsigned int nt = 0; nt < N * T; nt++)
      v[nt] = Density::gaussian(0, 1);
    double s = sqrt(1 - c * c);
    for(unsigned int t = 0, nt = 0; t < T; t++)
      for(unsigned int n = 0; n < N; n += 2, nt += 2)
        v[nt] = s * v[nt] + c * v[nt + 1];
  } else if(name == "acorr") {
    double cs = 2 * va_arg(a, double);
    assume(-1 <= cs && cs <= 1, "illegal-argument", "in network::BufferInput::BufferInput(\"acorr\",...) we must have acorr=%g in [-1/2,1/2]", cs);
    va_end(a);
    Density::setSeed(0);
    for(unsigned int nt = 0; nt < N * T; nt++)
      v[nt] = Density::gaussian(0, 1);
    double c = 0.5 * (sqrt(1 + cs) + sqrt(1 - cs)), s = sqrt(1 - c * c);
    for(unsigned int t = 1, nt = N; t < T; t++)
      for(unsigned int n = 0; n < N; n++, nt++)
        v[nt] = c * v[nt] + s * v[nt - N];
  } else
    assume(false, "illegal-argument", "in network::BufferedInput %s is an unknown sequence name\n", name.c_str());
}
network::BufferedInput::BufferedInput(const Input& input, String name, ...) : Input(input.N, input.T), values(new double[input.N * (int) input.T]), vvalues(NULL), buffered(true), vectored(false)
{
  va_list a;
  va_start(a, name);
  double *v = const_cast < double * > (values);
  if(name == "affine") {
    double offset = va_arg(a, double);
    double gain = va_arg(a, double);
    va_end(a);
    for(unsigned int t = 0, nt = 0; t < T; t++)
      for(unsigned int n = 0; n < N; n++, nt++)
        v[nt] = offset + gain * input.get(n, t);
  } else if(name == "noise") {
    double noiseProbability = va_arg(a, double);
    double noiseStandardDeviation = va_arg(a, double);
    va_end(a);
    for(unsigned int t = 0, nt = 0; t < T; t++)
      for(unsigned int n = 0; n < N; n++, nt++)
        v[nt] = input.get(n, t) + (noiseProbability <= Density::uniform(0.0, 1.0) ? 0 : Density::gaussian(0, noiseStandardDeviation));
  } else
    assume(false, "illegal-argument", "in network::BufferedInput %s is an unknown transformation name\n", name.c_str());
}
///@cond INTERNAL
network::BufferedInput::~BufferedInput()
{
  if(buffered)
    delete[] values;
  if(vectored)
    delete[] vvalues;
}
///@endcond
double network::BufferedInput::get(unsigned int n, double t) const
{
  if(t < 0)
    return 0;
  if(vectored) {
    assume(n < N && t < vvalues[n].size(), "illegal-argument", "in network::BufferedInput::get, index out of range, we must have n=%d in {0, %d{ and t=%g in [0, %g[", n, N, t, vvalues[n].size());
    return vvalues[n][t];
  } else {
    assume(n < N && t < T, "illegal-argument", "in network::BufferedInput::get, index out of range, we must have n=%d in {0, %d{ and t=%g in [0, %g[", n, N, t, T);
    return values[n + N * (int) t];
  }
}
void network::BufferedInput::add(const double *value)
{
  assume(vectored, "illegal-argument", "in network::BufferedInput::add, attempt to add a value on a fixed length buffer");
  for(unsigned int n = 0; n < N; n++)
    vvalues[n].push_back(value[n]);
  setT(T + 1);
}
void network::BufferedInput::save(String file, String format, bool show) const
{
  unsigned int T_ = (unsigned int) T;
  if(file == "stdout") {
    for(unsigned int n = 0; n < N; n++)
      printf(" %4d |%c", n, n < N - 1 ? ' ' : '\n');
    for(unsigned int n = 0; n < N; n++)
      printf("-------%c", n < N - 1 ? '-' : '\n');
    for(unsigned int t = 0; t < T_; t++)
      for(unsigned int n = 0; n < N; n++)
        printf("%6.1g %c", get(n, t), n < N - 1 ? ' ' : '\n');
    for(unsigned int n = 0; n < N; n++)
      printf("-------%c", n < N - 1 ? '-' : '\n');
  } else {
    s_save_mkdir(file);
    std::string ext = format == "csv" ? ".csv" : ".dat";
    FILE *fp = fopen((file + ext).c_str(), "w");
    assume(fp != NULL, "IO-Exception", "in network::ObservedInput::save unable to open %s%s", file.c_str(), ext.c_str());
    if(format == "binary-unit-time") {
      fprintf(fp, "%d %.0f\n", N, T);
      if(vectored) {
        double v[1];
        for(unsigned int t = 0; t < T_; t++)
          for(unsigned int n = 0; n < N; n++) {
            v[0] = get(n, t);
            fwrite(v, 8, 1, fp);
          }
      } else
        fwrite(values, 8, N * T_, fp);
    } else if((format == "ascii-unit-time") || (format == "csv"))
      for(unsigned int t = 0; t < T_; t++)
        for(unsigned int n = 0; n < N; n++)
          fprintf(fp, "%g%c", get(n, t), n < N - 1 ? (format == "csv" ? ',' : ' ') : '\n');
    else if((format == "gnuplot") || (format == "gnushow"))
      for(unsigned int t = 0; t < T_; t++)
        for(unsigned int n = 0; n < N; n++)
          fprintf(fp, "%d %d %g\n", n, t, get(n, t));
    else if(format == "gnuimg")
      for(unsigned int n = 0; n < N; n++)
        for(unsigned int t = 0; t < T_; t++)
          fprintf(fp, "%f%c", get(n, t), t < T_ - 1 ? ' ' : '\n');
    else
      assume(false, "illegal-argument", "in network::ObservedInput::save %s is an unknown format\n", format.c_str());
    assume(ferror(fp) == 0, "IO-Exception", "in network::ObservedInput::save error %s when saving in %s.dat", strerror(ferror(fp)), file.c_str());
    fclose(fp);
  }
  if(format == "gnuplot") {
    std::string plot =
      s_printf("set multiplot layout %d,1 $title\nset xrange [0:*]\nset yrange [*:*]\nset ytics nomirror\n", N);
    for(unsigned int n = 0; n < N; n++)
      plot += s_printf("plot \"" + file + ".dat\" every %d::%d using 2:3 with lines linecolor \"black\" notitle\n", N, n);
    gnuplot(file, plot, show);
  }
  if(format == "gnuimg")
    gnuplot(file, "set pal gray\nset pal gamma 2\nplot \"" + file + ".dat\" matrix with image notitle\n", show);
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
