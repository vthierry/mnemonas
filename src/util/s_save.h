#include "util/s_string.h"

/** Saves a string in a file.
 * @param file The file name.
 * @param string The string to save.
 */
void s_save(String file, String string);

/** Creates the directory in order to save in a file.
 * @param file The file name.
 */
void s_save_mkdir(String file);
