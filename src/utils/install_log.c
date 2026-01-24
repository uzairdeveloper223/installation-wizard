/**
 * This code is responsible for handling all installation log file operations
 * including initialization, writing step headers, and reading log lines.
 */

#include "../all.h"

void init_install_log(void)
{
    FILE *log_file = fopen(CONFIG_INSTALL_LOG_PATH, "w");
    if (log_file)
    {
        fclose(log_file);
    }
}

void write_install_log_header(const char *step_name)
{
    FILE *log_file = fopen(CONFIG_INSTALL_LOG_PATH, "a");
    if (log_file)
    {
        fprintf(log_file, "\n");
        fprintf(log_file, "--------------------------------------------------------------\n");
        fprintf(log_file, "  %s\n", step_name);
        fprintf(log_file, "--------------------------------------------------------------\n");
        fprintf(log_file, "\n");
        fclose(log_file);
    }
}

void write_install_log(const char *format, ...)
{
    FILE *log_file = fopen(CONFIG_INSTALL_LOG_PATH, "a");
    if (log_file)
    {
        va_list arguments;
        va_start(arguments, format);
        vfprintf(log_file, format, arguments);
        va_end(arguments);
        fprintf(log_file, "\n");
        fclose(log_file);
    }
}

char **read_install_log_lines(int max_lines, int *out_count)
{
    // Initialize output count to zero.
    *out_count = 0;

    // Open log file for reading.
    FILE *log_file = fopen(CONFIG_INSTALL_LOG_PATH, "r");
    if (!log_file)
    {
        return NULL;
    }

    // Allocate array to hold line pointers.
    char **lines = calloc(max_lines, sizeof(char *));
    if (!lines)
    {
        fclose(log_file);
        return NULL;
    }

    // Initialize line buffer and counter.
    char line_buffer[512];
    int line_count = 0;

    // Read lines into circular buffer, keeping only the last max_lines.
    while (fgets(line_buffer, sizeof(line_buffer), log_file))
    {
        // Remove trailing newline.
        size_t length = strlen(line_buffer);
        if (length > 0 && line_buffer[length - 1] == '\n')
        {
            line_buffer[length - 1] = '\0';
        }

        // Shift buffer and free oldest line if full.
        if (line_count >= max_lines)
        {
            free(lines[0]);
            memmove(lines, lines + 1, (max_lines - 1) * sizeof(char *));
            line_count = max_lines - 1;
        }

        // Store duplicate of current line.
        lines[line_count] = strdup(line_buffer);
        line_count++;
    }

    // Close file and return results.
    fclose(log_file);
    *out_count = line_count;
    return lines;
}

void free_install_log_lines(char **lines, int count)
{
    if (!lines) return;

    // Free each line one-by-one.
    for (int i = 0; i < count; i++)
    {
        free(lines[i]);
    }
    free(lines);
}
