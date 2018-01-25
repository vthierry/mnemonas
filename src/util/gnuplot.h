#include "s_string.h"

/** Generates a gnuplot display script.
 * - Generates a script to display <a href="http://gnuplot.sourceforge.net">gnuplot</a> commands.
 * @param file The related data file name (without extension). By contract,
 *   - the file basename is used as the plot title;
 *   - the file name is used to generate a <tt>.gnuplot.sh</tt> display script.
 *   - the file name is used to generate a <tt>.png</tt> image of the plot.
 *   - the data is stored in a file of extension <tt>.dat</tt>.
 * @param plot The gnuplot display commands.
 *   - The $title variable contains the plot <tt>title "plot-title"</tt> parameter.
 * See: <a href="http://gnuplot.sourceforge.net/docs_4.2/node154.html">gnuplot set commands</a>.
 * @param show If true performs on the fly display.
 */
void gnuplot(String file, String plot, bool show = false);
