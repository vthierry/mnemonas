#include "s_printf.h"
#include "assume.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

std::string s_printf(String header, ...)
{
  const unsigned int chars_buffer_size = 10000;
  char chars_buffer[chars_buffer_size];
  {
    va_list a;
    va_start(a, header);
    int l = vsnprintf(chars_buffer, chars_buffer_size, header.c_str(), a);
    assume(0 <= l, "illegal-argument", "in s_printf: erroneous format %s", header.c_str());
    assume(l < (int) chars_buffer_size, "illegal-state", "in s_printf: formating overflow");
    va_end(a);
  }
  return (std::string) chars_buffer;
}
