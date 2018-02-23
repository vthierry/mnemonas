/*! \mainpage
 *
 * This source bundle provides all files of the:
 * - <i>Recurrent neural network weight estimation though backward tuning</i> <a href="https://hal.inria.fr/hal-01610735v1"><b>publication</b></a>.
 *
 * It is a part of the <a href="https://team.inria.fr/mnemosyne">mnemosyne</a> research code.
 *
 * The documentation:
 * - <a href="hierarchy.html"><b>Class hierarchy</b></a>
 * - \subpage util
 *    - Struct() Implements a generic minimal iterative structure.
 * - <a href="files.html"><b>Files list</b></a> (and <a href="globals.html"><b>members</b></a>)
 *
 * Files are available at <a href="https://github.com/vthierry/mnemonas">https://github.com/vthierry/mnemonas</a> or as a set of <a href="./sources.zip"><b>source files</b></a> unded the open source <a href="http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html">CeCILL-C</a> licence. 
 *
 * You also can generate a Python wrapper for this middleware using <a href="http://www.swig.org">swig</a>. Simply download the <a href="./sources.zip"><b>source files</b></a> and run <tt>make pywrap</tt>.
 *
 */

/*! \page util Utility routines
 * 
 * - assume() Checks a condition at run-time and throws a fatal error exception and/or dumps a warning if not verified. 
 * - <a href="s__string_8h.html">String</a> String manipulation simplified routines 
 *   - s_printf() Returns a string formated a-la printf.
 *   - s_load() Loads a string from a file.
 *   - s_save() Saves a string in a file.
 *   - s_regex_match() Regular expression match function.
 *   - s_regex_replace() Regular expression replace function.
 * - now() Gets the cpu or real time from program start. 
 * - gnuplot() Generates a gnuplot dipslay script.
 */
