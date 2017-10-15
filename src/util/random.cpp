#include "random.hpp"

// Used for a inpredictible seed
#include "now.h"

// Used for runtime error detection
#include "assume.h"

double random::Density::p(double x) const
{
  assume(false, "illegal-state", "in random::density::p this method must be overridden.");
  return NAN;
}
std::mt19937_64 random::generator;
std::uniform_real_distribution < double > random::distribution(0.0, 1.0);

// Generates the seed of the pseudo random generator.
void random::setSeed(int seed)
{
  generator.seed(seed == -1 ? (int) (1e6 * now()) : seed);
}
// Generates an integer uniform pseudo-random value.
int random::uniform(int min, int max)
{
  int r = (int) uniform((double) min, (double) max);
  return r < max ? r : uniform(min, max);
}
// Generates a double uniform pseudo-random value.
double random::uniform(double min, double max)
{
  return min + (max - min) * distribution(generator);
}
// Gets a Gaussian random value.
double random::gaussian(double mean, double stdev)
{
  return stdev * sqrt(-2.0 * loguniform()) * cos(2.0 * M_PI * uniform()) + mean;
}
// Gets the Gaussian mean correspondign to a given bias.
double random::getGaussianBias(double bias, double stdev)
{
  // The mean corresponding to a given bias has been-precalculated in maple and stored in an array.
  return stdev * gaussian_bias[bias < 0 ? 0 : bias >= 1 ? 100 : (int) (100 * bias)];
}
// Precomputed table of Gaussian bias
// bias := ['fsolve(int(exp(-(x-m)^2/2)/sqrt(2*Pi), x = 0 .. infinity) = p / 100, m)'$p=0..100];
const double random::gaussian_bias[] = { -13, -2.326347874, -2.053748911, -1.880793608, -1.750686071, -1.644853627, -1.554773595, -1.475791028, -1.405071560, -1.340755034, -1.281551566, -1.226528120, -1.174986792, -1.126391129, -1.080319341, -1.036433389, -0.9944578832, -0.9541652531, -0.9153650878, -0.8778962951, -0.8416212336, -0.8064212470, -0.7721932142, -0.7388468492, -0.7063025628, -0.6744897502, -0.6433454054, -0.6128129910, -0.5828415073, -0.5533847196, -0.5244005127, -0.4958503473, -0.4676987991, -0.4399131657, -0.4124631294, -0.3853204664, -0.3584587933, -0.3318533464, -0.3054807881, -0.2793190344, -0.2533471031, -0.2275449766, -0.2018934791, -0.1763741648, -0.1509692155, -0.1256613469, -0.1004337205, -0.07526986210, -0.05015358346, -0.02506890826, 0., 0.02506890826, 0.05015358346, 0.07526986210, 0.1004337205, 0.1256613469, 0.1509692155, 0.1763741648, 0.2018934791, 0.2275449766, 0.2533471031, 0.2793190344, 0.3054807881, 0.3318533464, 0.3584587933, 0.3853204664, 0.4124631294, 0.4399131657, 0.4676987991, 0.4958503473, 0.5244005127, 0.5533847196, 0.5828415073, 0.6128129910, 0.6433454054, 0.6744897502, 0.7063025628, 0.7388468492, 0.7721932142, 0.8064212470, 0.8416212336, 0.8778962951, 0.9153650878, 0.9541652531, 0.9944578832, 1.036433389, 1.080319341, 1.126391129, 1.174986792, 1.226528120, 1.281551566, 1.340755034, 1.405071560, 1.475791028, 1.554773595, 1.644853627, 1.750686071, 1.880793608, 2.053748911, 2.326347874, 13 };
// Gets a Gamma/Poisson distribution random value.
double random::gamma(unsigned int degree, double rate)
{
  double p = 0;
  for(unsigned int i = 0; i < degree; i++)
    p += -loguniform() * rate;
  return p;
}
// Returns log(uniform()) avoiding -inf values
double random::loguniform()
{
  double r = uniform();
  return r == 0 ? loguniform() : log(r);
}
// Gets an array of random true values, adjusting the random draw to get an exact number of true value.
bool *random::booleans(unsigned int size, unsigned int count)
{
  assume(count <= size, "illegal-argument", "in random::booleans: uncoherent count = %d > size = %d", count, size);
  bool *c = new bool[size];
  for(int s = size, s1 = count; s > 0; s--)
    if((c[s - 1] = (uniform() < ((s1 <= 0) ? 0 : (s <= s1) ? 1 : (double) s1 / (double) s))))
      s1--;
  return c;
}
