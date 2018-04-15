#include "s_exec.h"
#include "assume.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Used for isspace()
#include <ctype.h>
// Used for wait()
#include <sys/wait.h>

std::string s_exec(String command, bool noline)
{
  int pp[2];
  // Creates a pipe to read the system stdout
  assume(pipe(pp) != -1, "io-error", "in sys::run: cannot create pipe");
  // Forking the system call using the pipe
  switch(fork()) {
  // Error in the forking process
  case -1:
    assume(false, "io-error", "in sys::run: cannot fork");
  // This code is executed in the child
  case 0:
    // Connects stdout to the pipe
    assume(dup2(pp[1], 1) == 1, "io-error", "in sys::run: failed to connect pipes");
    // Closes unused pipes
    close(pp[0]);
    close(pp[1]);
    // Executes and quit
    exit(system(command.c_str()));
  default:
    // The code remainder is executed by the parent
    break;
  }
  // Closes unused pipe descriptor
  close(pp[1]);
  // Reads the stdout
  std::string result;
  for(char c; read(pp[0], &c, 1) == 1;)
    if(noline && isspace(c)) {
      if((result.length() > 0) && (result[result.length() - 1] != ' '))
        result += ' ';
    } else
      result += c;
  // Closes and synchronizes
  close(pp[0]);
  wait(NULL);
  // Returns in proper format
  return result;
}
