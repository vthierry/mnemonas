#include "gnuplot.h"
#include "assume.h"
#include "s_save.h"

void gnuplot(String file, String plot, bool show)
{
  // Replaces all $ by \$ in plot
  std::string splot = plot;
  {
    static String from = "$", to = "\\$";
    for(size_t start_pos = 0; (start_pos = splot.find(from, start_pos)) != std::string::npos; start_pos += to.length())
      if((start_pos + 5 < splot.length()) && (splot.substr(start_pos + 1, 5) != "title"))
        splot.replace(start_pos, from.length(), to);
  }
  std::string script =
    "# " + file + "gnuplot display script (automatically generated, do NOT edit)\n" +
    "title=\"title \\\"`basename $0 | sed 's/.gnuplot.sh$//'`\\\"\"\n" +
    "while [ \\! -z \"$1\" ] ; do case \"$1\" in\n" +
    " --png)  line1='set term png'; line2='set output \"" + file + ".png\"';;\n" +
    " *) echo '$0 [--png]'; exit;;\n" +
    "esac; shift; done\n\n" +
    "cat << EOD | gnuplot -persist\n" +
    "$line1\n$line2\n" +
    splot + "\n" +
    "EOD\n";
  s_save(file + ".gnuplot.sh", script);
  system(("sh " + file + ".gnuplot.sh --png 2>/dev/null 1>/dev/null").c_str());
  if(show)
    system(("sh " + file + ".gnuplot.sh 2>/dev/null 1>/dev/null").c_str());
}
