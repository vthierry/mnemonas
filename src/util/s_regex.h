#include "util/s_string.h"

/** Regular expression replace function.
 * @param string The input string.
 * @param regex The regular-expression to match.
 * @param output The output pattern.
 * @return The string replacement if any, else the original string.
 */
std::string s_regex_replace(String string, String regex, String output);

/** Regular expression match function.
 * @param string The input string.
 * @param regex The regular-expression to match.
 * @return True if the string match the regex, false otherwise.
 */
bool s_regex_match(String string, String regex);
