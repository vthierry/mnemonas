#include "s_string.h"

/** Checks a condition at run-time and throws a fatal error exception and/or dumps a warning if not verified.
 * @param condition The condition that must be true.
 * @param thrown The name of the thrown exception if the assume fails. Typical exception are:
 * - illegal-argument : when a method or function receives an argument with a spurious value.
 * - illegal-state    : when detecting an unexpected condition (a bug) at some point in the code.
 * - numerical-error  : when detecting an unexpected condition (a bug) at some point in the code.
 * - IO-exception     : when an input/output operation fails.
 * The exception is a fatal error unless its names starts with a space (e.g., " numerical-error"), in that case a simple warning is printed on stderr.
 * @param message The error message, a string format à-la printf with more parameters if used.
 * @throws The exception to be thrown if the assume fails.
 */
void assume(bool condition, String thrown, const char *message, ...);
