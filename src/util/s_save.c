#include "s_save.h"
#include "assume.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>

void s_save_mkdir(String file)
{
  system(("mkdir -p `dirname \"" + file + "\"`").c_str());
}
void s_save(String file, String string)
{
  s_save_mkdir(file);
  FILE *fp = fopen(file.c_str(), "w");
  assume(fp != NULL, "IO-Exception", "in s_save unable to open %s", file.c_str());
  fprintf(fp, "%s", string.c_str());
  assume(ferror(fp) == 0, "IO-Exception", "in s_save error %s when saving in %s", strerror(ferror(fp)), file.c_str());
  fclose(fp);
}
