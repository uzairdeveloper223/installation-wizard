/**
 * This code is responsible for displaying a confirmation summary
 * of all user selections before proceeding with installation.
 */

#include "../all.h"

int run_confirmation_step(WINDOW *modal)
{
    Store *store = get_store();

    // Clear and draw step header.
    clear_modal(modal);
    wattron(modal, A_BOLD | COLOR_PAIR(CUSTOM_COLOR_PAIR_MAIN));
    mvwprintw(modal, 2, 3, "Step 4: Confirm Installation");
    wattroff(modal, A_BOLD);

    // Display summary of selected options.
    mvwprintw(modal, 4, 3, "Ready to install LimeOS with the following settings:");
    mvwprintw(modal, 6, 3, "  Locale: %s", store->locale);
    mvwprintw(modal, 7, 3, "  Disk:   %s", store->disk);

    // Display partition summary.
    unsigned long long disk_size = get_disk_size(store->disk);
    unsigned long long used = sum_partition_sizes(store->partitions, store->partition_count);
    unsigned long long free_space = (disk_size > used) ? disk_size - used : 0;
    char free_str[32];
    format_disk_size(free_space, free_str, sizeof(free_str));
    if (store->partition_count > 0)
    {
        mvwprintw(
            modal,                                      // Modal window.
            8,                                          // Line number.
            3,                                          // Column number.
            "  Partitions: %d partitions, %s left",     // Format string.
            store->partition_count,
            free_str
        );
    }
    else
    {
        mvwprintw(modal, 8, 3, "  Partitions: (none)");
    }

    // Display warning about disk formatting.
    char warning_text[128];
    snprintf(
        warning_text, sizeof(warning_text),
        "All data on %s will be erased!\n"
        "This action cannot be undone.", store->disk
    );
    render_warning(modal, 10, 3, warning_text);

    // Display navigation footer.
    const char *footer[] = {"[Enter] Install", "[Esc] Back", NULL};
    render_footer(modal, footer);
    wrefresh(modal);

    // Wait for user confirmation or back.
    int key;
    while ((key = wgetch(modal)) != '\n' && key != 27)
    {
        // Ignore other input.
    }

    return key == '\n';
}
