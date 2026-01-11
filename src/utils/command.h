#pragma once
#include "../all.h"

/** The path to the dry run log file. */
#define DRY_RUN_LOG_PATH "dry-run.log"

/**
 * Executes a shell command, or logs it if dry run mode is enabled.
 *
 * In dry run mode, commands are written to DRY_RUN_LOG_PATH instead of
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

/** Callback invoked during command execution for UI updates. */
typedef void (*CommandPollCallback)(void);

/**
 * Sets a callback to be invoked during command execution.
 * The callback can handle input polling and UI updates.
 *
 * @param callback Function to call during polling, or NULL to disable.
 */
void set_command_poll_callback(CommandPollCallback callback);

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
