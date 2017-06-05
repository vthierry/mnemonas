#include <stdio.h>
#include <stdarg.h>
#include "assume.h"

/** Returns a string formated a-la printf.
 * @param header Print header. A string or a "format" a-la printf with the subsequent arguments. Formatted strings have 100K bytes maximal size.
 * - Tip: If a spurious error occurs, it is a good idea to test the a-la printf construct via a printf in order the compiler checks the data types.
 * @return The formatted string.
 */
std::string s_printf(std::string header, ...)
{
  static const int nchars = 100000;
  char chars[nchars];
  {
    va_list a;
    va_start(a, header);
    int l = vsnprintf(chars, nchars, header.c_str(), a);
    assume(0 <= l, "illegal-argument", "in s_printf: erroneous format %s", header.c_str());
    assume(l < nchars, "illegal-state", "in s_printf: formating overflow");
    va_end(a);
  }
  return (std::string) chars;
}
