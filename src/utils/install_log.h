#pragma once

/**
 * Initializes the installation log file by clearing any existing content.
 * Should be called at the start of installation before any commands are run.
 */
void init_install_log(void);

/**
 * Writes a step header to the installation log file.
 *
 * @param step_name The name of the step to write.
 */
void write_install_log_header(const char *step_name);

/**
 * Writes a message to the installation log file.
 *
 * @param format Printf-style format string.
 * @param ... Format arguments.
 */
void write_install_log(const char *format, ...);

/**
 * Reads the last N lines from the install log file.
 * Caller must free the result using free_install_log_lines().
 *
 * @param max_lines Maximum number of lines to return.
 * @param out_count Output: actual number of lines returned.
 * @return Array of line strings, or NULL on error.
 */
char **read_install_log_lines(int max_lines, int *out_count);

/**
 * Frees a lines array returned by read_install_log_lines().
 *
 * @param lines The array to free.
 * @param count The number of elements in the array.
 */
void free_install_log_lines(char **lines, int count);
