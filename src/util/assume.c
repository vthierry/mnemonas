#include "assume.h"

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

void assume(bool condition, String thrown, const char *message, ...)
{
  bool simple_warning = thrown[0] == ' ';
  static const unsigned int nchars = 10000;
  static char chars[nchars];
  if(!condition) {
    va_list a;
    va_start(a, message);
    vsnprintf(chars, nchars, message, a);
    va_end(a);
    fprintf(stderr, "Runtime %s '%s' : %s\n", simple_warning ? "warning" : "fatal error", thrown.c_str(), chars);
    if(simple_warning)
      return;
#ifdef WIN32
    // Quit program
    exit(3);
#else
    // send signal (3) to stop the program (keep the stack)
    raise(SIGQUIT);
#endif
  }
}
