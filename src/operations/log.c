/**
 * This code is responsible for handling all installation log file operations
 * including initialization, writing step headers, and reading log lines.
 */

#include "../all.h"

void init_install_log(void)
{
    FILE *f = fopen(INSTALL_LOG_PATH, "w");
    if (f)
    {
        fclose(f);
    }
}

void write_install_log_header(const char *step_name)
{
    FILE *f = fopen(INSTALL_LOG_PATH, "a");
    if (f)
    {
        fprintf(f, "\n");
        fprintf(f, "--------------------------------------------------------------\n");
        fprintf(f, "  %s\n", step_name);
        fprintf(f, "--------------------------------------------------------------\n");
        fprintf(f, "\n");
        fclose(f);
    }
}

char **read_install_log_lines(int max_lines, int *out_count)
{
    // Initialize output count to zero.
    *out_count = 0;

    // Open log file for reading.
    FILE *f = fopen(INSTALL_LOG_PATH, "r");
    if (!f)
    {
        return NULL;
    }

    // Allocate array to hold line pointers.
    char **lines = calloc(max_lines, sizeof(char *));
    if (!lines)
    {
        fclose(f);
        return NULL;
    }

    // Initialize line buffer and counter.
    char line_buf[512];
    int line_count = 0;

    // Read lines into circular buffer, keeping only the last max_lines.
    while (fgets(line_buf, sizeof(line_buf), f))
    {
        // Remove trailing newline.
        size_t len = strlen(line_buf);
        if (len > 0 && line_buf[len - 1] == '\n')
        {
            line_buf[len - 1] = '\0';
        }

        // Shift buffer and free oldest line if full.
        if (line_count >= max_lines)
        {
            free(lines[0]);
            memmove(lines, lines + 1, (max_lines - 1) * sizeof(char *));
            line_count = max_lines - 1;
        }

        // Store duplicate of current line.
        lines[line_count] = strdup(line_buf);
        line_count++;
    }

    // Close file and return results.
    fclose(f);
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
