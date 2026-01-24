#pragma once
#include "../all.h"

/**
 * Executes a shell command, or logs it if dry run mode is enabled.
 *
 * In dry run mode, commands are written to CONFIG_DRY_RUN_LOG_PATH instead of
 * being executed, and the function returns 0 (success).
 *
 * @param command The shell command to execute.
 *
 * @return - `0` - Success (or dry run mode).
 * @return - `non-zero` - The return value of system() on failure.
 */
int run_command(const char *command);

/**
 * Closes the dry run log file if open.
 *
 * Should be called at the end of installation to flush and close
 * the log file properly.
 */
void close_dry_run_log(void);

/** Callback invoked periodically during command execution. */
typedef void (*CommandTickCallback)(void);

/**
 * Sets a callback to be invoked periodically during command execution.
 * The callback can handle input checking, animation updates, and other
 * periodic tasks.
 *
 * @param callback Function to call on each tick, or NULL to disable.
 */
void set_command_tick_callback(CommandTickCallback callback);

/**
 * Escapes a string for safe use in shell commands.
 *
 * Wraps the input in single quotes and escapes any embedded single quotes
 * using the '\'' technique (end quote, literal quote, start quote).
 *
 * @param input The string to escape.
 * @param output Buffer to write the escaped string.
 * @param output_size Size of the output buffer.
 *
 * @return - `0` - Success.
 * @return - `-1` - Output buffer too small.
 */
int shell_escape(const char *input, char *output, size_t output_size);
