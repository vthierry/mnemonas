#include "s_printf.h"
#include "assume.h"

#include <stdio.h>
#include <stdarg.h>

std::string s_printf(String header, ...)
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
