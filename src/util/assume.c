#include "assume.h"

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

void assume(bool condition, String thrown, String message, ...)
{
  if(!condition) {
    const unsigned int chars_buffer_size = 10000;
    char chars_buffer[chars_buffer_size];
    {
      va_list a;
      va_start(a, message);
      vsnprintf(chars_buffer, chars_buffer_size, message.c_str(), a);
      va_end(a);
    }
    bool simple_warning = thrown[0] == ' ';
    fprintf(stderr, "Runtime %s '%s' : %s\n", simple_warning ? "warning" : "fatal error", thrown.c_str(), chars_buffer);
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
