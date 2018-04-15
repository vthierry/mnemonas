#include "util/s_string.h"

/** Executes a system command and returns the stdout.
 * @param command The command to execute. The command path must be absolute (e.g. <tt>/bin/pwd</tt> not <tt>pwd</tt>).
 * @param noline If true replaces new lines and spaces by a unique space.
 * @return The command result, or an error message.
 */
std::string s_exec(String command, bool noline = false);
