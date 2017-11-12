#include "s_load.h"
#include "assume.h"

#include <stdio.h>
#include <string.h>

std::string s_load(String file)
{
  std::string string;
  FILE *fp = fopen(file.c_str(), "r");
  assume(fp != NULL, "IO-Exception", "in s_load unable to open %s", file.c_str());
  for(char c; (c = (char) fgetc(fp)) != EOF;)
    string += c;
  fclose(fp);
  return string;
}
