#pragma once
#include "../all.h"

/**
 * Checks if a shared library is available. 
 * 
 * @param name The `.so` file name of the shared library.
 * 
 * @return - `True (1)` The shared library is available.
 * @return - `False (0)` The shared library is not available.
 * 
 * @note - Use `ldconfig -p | grep "libname"` to find shared library names.
 * @note - Include ABI version in the shared library name (e.g. `libX11.so.6`).
 */
int is_library_available(const char *name);

/**
 * Checks if a command is available in PATH.
 *
 * @param name The command name (e.g. "parted").
 *
 * @return - `True (1)` The command is available.
 * @return - `False (0)` The command is not available.
 */
int is_command_available(const char *name);
