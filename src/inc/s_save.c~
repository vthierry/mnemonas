#include "assume.h"

#include <stdio.h>
#include <string.h>

void s_save(String file, String string)
{
  FILE *fp = fopen(file.c_str(), "w");
  assume(fp != NULL, "IO-Exception", "in s_save unable to open %s", file.c_str());
  fprintf(fp, "%s", string.c_str());
  assume(ferror(fp) == 0, "IO-Exception", "in s_save error %s when saving in %s", strerror(ferror(fp)), file.c_str());
  fclose(fp);
}
