#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include "s_string.h"

void assume(bool condition, String thrown, const char *message, ...)
{
  static const unsigned int nchars = 10000;
  static char chars[nchars];
  if(!condition) {
    va_list a;
    va_start(a, message);
    vsnprintf(chars, nchars, message, a);
    va_end(a);
    fprintf(stderr, "Runtime fatal error %s : %s\n", thrown.c_str(), chars);
#ifdef WIN32
    // Quit program
    exit(3);
#else
    // send signal (3) to stop the program (keep the stack)
    raise(SIGQUIT);
#endif
  }
}
